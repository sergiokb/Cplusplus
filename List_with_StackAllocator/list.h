#include <iostream>
template<size_t N>
class StackStorage {
private:
    uint8_t memory[N];
    uint8_t* pointer = memory;

public:
    StackStorage() = default;
    ~StackStorage() = default;
    StackStorage(StackStorage const&) = delete;
    void operator=(StackStorage const&) = delete;
    uint8_t* get_pointer() {
        return pointer;
    }
    void set_pointer(uint8_t* p) {
        pointer = p;
    }
    uint8_t* get_memory(int sz, int align) {
        auto h = reinterpret_cast<uintptr_t>(pointer);
        unsigned long shift = h % align;
        pointer += (align - shift);
        uint8_t* ptr = pointer;
        pointer += sz;
        return ptr;
    }
};


template<typename T, size_t N>
class StackAllocator {
private:
    StackStorage<N>* st;

public:
    using value_type = T;
    template <typename U, size_t M> friend class StackAllocator;
    StackAllocator() = default;
    ~StackAllocator() = default;
    StackAllocator(StackStorage<N>& stt): st(&stt) {}
    StackAllocator& operator=(const StackAllocator& copy) {
        st = copy.st;
        return *this;
    }
    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other): st(other.st){}
    T* allocate(const size_t n) {
        return reinterpret_cast<T*>(st->get_memory(n * sizeof(T), alignof(T)));
    }
    void deallocate(T* p, const size_t n) {
        auto u = reinterpret_cast<uint8_t*>(p);
        if(u + n == st->get_pointer()) {
            st->set_pointer(u);
        }
    }

    template <typename ...Args>
    void construct(T* p, const Args& ...args) {
        new(p) T(args...);
    }

    void destroy(T* p) {
        p->~T();
    }

    template <typename U>
    struct rebind{
        using other = StackAllocator<U, N>;
    };

    template <typename T1, size_t N1, typename T2, size_t N2>
    friend bool operator==(const StackAllocator<T1, N1>& a1, const StackAllocator<T2, N2>& a2);
};

template <typename T1, size_t N1, typename T2, size_t N2>
bool operator==(const StackAllocator<T1, N1>& a1, const StackAllocator<T2, N2>& a2) {
    return (a1.st == a2.st && N1 == N2);
}

template <typename T1, size_t N1, typename T2, size_t N2>
bool operator!=(const StackAllocator<T1, N1>& a1, const StackAllocator<T2, N2>& a2) {
    return !(a1 == a2);
}


template <typename T, typename Allocator = std::allocator<T>>
class List {
    private:
        size_t sz = 0;
        struct BaseNode {
            BaseNode* prev = nullptr;
            BaseNode* next = nullptr;
        };
        struct Node: BaseNode {
            T value;
            Node() = default;
            Node(const T& val): value(val) {};
        };
        BaseNode fakeNode;

    public:
        using AllocTraits = std::allocator_traits<Allocator>;
        using NodeAlloc = typename AllocTraits::template rebind_alloc<Node>;
        using NodeTraits = std::allocator_traits<NodeAlloc>;

    private:
        NodeAlloc alloc;

    public:
        List() = default;
        List(size_t n) {
            try{
                for(size_t i = 0; i < n; i++) {
                    emplace_back();
                }
            } catch(...) {
                while(sz > 0) {
                    pop_back();
                }
                throw;
            }
        };
        List(size_t n, const T& val) {
            try{
                for(size_t i = 0; i < n; i++) {
                    push_back(val);
                }
            } catch(...) {
                while(sz > 0) {
                    pop_back();
                }
                throw;
            }

        };
        List(Allocator al) {
            alloc = NodeAlloc(al);
        };
        List(const size_t n, Allocator al)  {
            alloc = NodeAlloc(al);
            try {
                for(size_t i = 0; i < n; i++) {
                    emplace_back();
                }
            }   catch(...) {
                while(sz > 0) {
                    pop_back();
                }
                throw;
            }
        };
        List(size_t n, const T& val, Allocator al){
            alloc = NodeAlloc(al);
            try {
                for(size_t i = 0; i < n; i++) {
                    push_back(val);
                }
            }   catch(...) {
                while(sz > 0) {
                    pop_back();
                }
                throw;
            }
        }

        List(const List& lst): alloc(NodeTraits::select_on_container_copy_construction(lst.alloc)){
            try {
                for(auto p = lst.begin(); p != lst.end(); ++p) {
                    push_back(*p);
                }
            }   catch(...) {
                while(sz > 0) {
                    pop_back();
                }
                throw;
            }

        };

        ~List() {
            while(sz > 0) {
                pop_back();
            }
        };

        List& operator=(const List& lst) {
            auto copy_alloc = alloc;
            if (NodeTraits::propagate_on_container_copy_assignment::value) alloc = lst.alloc;
            size_t t = 0;
            try {
                for (auto p = lst.begin(); p != lst.end(); ++p) {
                    push_back(*p);
                    ++t;
                }
            }   catch(...) {
                while(t > 0) {
                    pop_back();
                    --t;
                }
                alloc = copy_alloc;
                throw;
            }
            alloc = copy_alloc;
            while (sz > lst.sz) {
                pop_front();
            }
            if (NodeTraits::propagate_on_container_copy_assignment::value) alloc = lst.alloc;
            return *this;
        };

        Allocator get_allocator() {
            return alloc;
        }

        size_t size() const{
            return sz;
        }

        void emplace_back() {
            Node* node = NodeTraits::allocate(alloc, 1);
            try {
                NodeTraits::construct(alloc, node);
                if (sz == 0) {
                    node->prev = &fakeNode;
                    fakeNode.next = node;
                }
                else {
                    node->prev = fakeNode.prev;
                    fakeNode.prev->next = node;
                }
                node->next = &fakeNode;
                fakeNode.prev = node;
                ++sz;
            }   catch(...) {
                NodeTraits::deallocate(alloc, static_cast<Node*>(node), 1);
                throw;
            }
        }

        void push_before(BaseNode* after, const T& val) {
            Node* node = NodeTraits::allocate(alloc, 1);
            try {
                NodeTraits::construct(alloc, node, val);
                if (sz == 0) {
                    node->prev = after;
                    after->next = node;
                }
                else {
                    node->prev = after->prev;
                    after->prev->next = node;
                }
                node->next = after;
                after->prev = node;
                ++sz;
            } catch(...) {
                NodeTraits::deallocate(alloc, static_cast<Node*>(node), 1);
                throw;
            }
        }

        void pop(BaseNode* ptr) {
            BaseNode* d = ptr;
            if (sz == 1) {
                fakeNode.prev = nullptr;
                fakeNode.next = nullptr;
            }
            else {
                ptr->prev->next = ptr->next;
                ptr->next->prev = ptr->prev;
            }
            NodeTraits::destroy(alloc, static_cast<Node*>(d));
            NodeTraits::deallocate(alloc, static_cast<Node*>(d), 1);
            --sz;
        }


        void push_back(const T& val) {
            push_before(&fakeNode, val);
        }
        void push_front(const T& val) {
            if (sz == 0) push_before(&fakeNode, val);
            else push_before(fakeNode.next, val);
        }

        void pop_back() {
            pop(fakeNode.prev);
        }
        void pop_front() {
            pop(fakeNode.next);
        }

        template <bool is_const>
        class Iterator{
        private:
            BaseNode* ptr = nullptr;
        public:
            using difference_type = long long;
            using value_type = T;
            using iterator_category = std::bidirectional_iterator_tag;
            using reference = std::conditional_t<is_const, const T&, T&>;
            using pointer = std::conditional_t<is_const, const T*, T*>;


            Iterator() = default;
            explicit Iterator(const BaseNode* p): ptr(const_cast<BaseNode*>(p)){}
            Iterator(const Iterator<false>& other): ptr(other.ptr){};
            Iterator& operator=(const Iterator& other) {
                Iterator copy = other;
                ptr = copy.ptr;
                return *this;
            }
            Iterator& operator++() {
                ptr = ptr->next;
                return *this;
            }
            Iterator& operator--() {
                ptr = ptr->prev;
                return *this;
            }
            Iterator operator++(int) {
                Iterator it = *this;
                ++(*this);
                return it;
            }
            Iterator operator--(int) {
                Iterator it = *this;
                --(*this);
                return it;
            }
            BaseNode* get_ptr() const{
                return ptr;
            }
            bool operator==(const Iterator& other) const{
                return (ptr == other.ptr);
            }
            bool operator!=(const Iterator& other) const{
                return !(ptr == other.ptr);
            }
            reference operator*() const {
                return reinterpret_cast<Node*>(ptr)->value;
            }
            pointer operator->() const{
                return &(reinterpret_cast<Node*>(ptr)->value);
            }
            friend class List<T, Allocator>;
        };

        using const_iterator = Iterator<true>;
        using iterator = Iterator<false>;
        using const_reverse_iterator = std::reverse_iterator<Iterator<true>>;
        using reverse_iterator = std::reverse_iterator<Iterator<false>>;

        iterator begin() {
            return iterator(fakeNode.next);
        };
        const_iterator begin() const {
            return const_iterator(fakeNode.next);
        };
        const_iterator cbegin() const {
            return const_iterator(fakeNode.next);
        };

        iterator end() {
            return iterator(&fakeNode);
        };
        const_iterator end() const {
            return const_iterator(&fakeNode);
        };
        const_iterator cend() const {
            return const_iterator(&fakeNode);
        };

        reverse_iterator rbegin() {
            return std::reverse_iterator(end());
        };
        const_reverse_iterator rbegin() const {
            return std::reverse_iterator(cend());
        };
        const_reverse_iterator crbegin() const {
            return std::reverse_iterator(cend());
        };

        reverse_iterator rend() {
            return std::reverse_iterator(begin());
        };
        const_reverse_iterator rend() const {
            return std::reverse_iterator(cbegin());
        };
        const_reverse_iterator crend() const {
            return std::reverse_iterator(cbegin());
        };

        iterator insert(const_iterator it, const T& val) {
            push_before(it.get_ptr(), val);
            return iterator(it.get_ptr());
        };

        iterator erase(const_iterator it) {
            iterator copy(it.get_ptr()->next);
            pop(it.get_ptr());
            return copy;
        };
    };


