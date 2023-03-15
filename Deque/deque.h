#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

template <typename T>
class Deque {
private:
    T** dq;
    size_t dq_size;
    size_t cap;
    const static size_t minicap = 16;
    struct miniArray;
    miniArray top;
    miniArray bottom;
    void reserve(size_t up_cap, size_t down_cap);
    size_t get_apos(size_t index) const;
    size_t get_minipos(size_t index) const;

public:
    Deque(): top(0, minicap - 1), bottom(0, minicap - 1) {
        dq_size = 0;
        cap = 4;
        dq = new T*[4];
        size_t exit = 0;
        try {
            for (size_t i = 0; i < 4; ++i) {
                dq[i] = reinterpret_cast<T*>(new uint8_t[minicap * sizeof(T)]);
                ++exit;
            }
        } catch (...) {
            for (size_t i = 0; i < exit; ++i) {
                delete[] reinterpret_cast<uint8_t*>(dq[i]);
            }
            delete[] dq;
            throw;
        }
    }

    Deque(const Deque& copy): dq_size(copy.dq_size), cap(copy.cap), top(copy.top), bottom(copy.bottom) {
        dq = new T*[cap];
        size_t exit = 0, exi = 0, exj = 0;
        try {
            for (size_t i = 0; i < cap; ++i) {
                dq[i] = reinterpret_cast<T*>(new uint8_t[minicap * sizeof(T)]);
                ++exit;
            }
            for (size_t j = top.edge; j < minicap; ++j) {
                new(dq[top.apos] + j) T(copy.dq[top.apos][j]);
                exj = j + 1;
            }
            for (size_t i = top.apos + 1; i < bottom.apos; ++i) {
                for (size_t j = 0; j < minicap; ++j) {
                    new(dq[i] + j) T(copy.dq[i][j]);
                    exj = j + 1;
                }
                exi = i + 1;
            }
            if (bottom.apos != top.apos) {
                for (size_t j = 0; j <= bottom.edge; ++j) {
                    new(dq[bottom.apos] + j) T(copy.dq[bottom.apos][j]);
                    exj = j + 1;
                }
            }
        } catch (...) {
            for (size_t j = top.edge; j < (exi == top.apos ? exj : minicap); ++j) {
                (dq[top.apos] + j)->~T();
            }
            for (size_t i = top.apos + 1; i < exi; ++i) {
                for (size_t j = 0; j < minicap; ++j) {
                    (dq[i] + j)->~T();
                }
            }
            if (exi != top.apos) {
                for (size_t j = 0; j < exj; ++j) {
                    (dq[exi] + j)->~T();
                }
            }
            for (size_t i = 0; i < exit; ++i) {
                delete[] reinterpret_cast<uint8_t*>(dq[i]);
            }
            delete[] dq;
            throw;
        }
    }

    ~Deque() {
        for (size_t j = top.edge; j < minicap; ++j) {
            (dq[top.apos] + j)->~T();
        }
        for (size_t i = top.apos + 1; i < bottom.apos; ++i) {
            for (size_t j = 0; j < minicap; ++j) {
                (dq[i] + j)->~T();
            }
        }
        if (bottom.apos != top.apos) {
            for (size_t j = 0; j <= bottom.edge; ++j) {
                (dq[bottom.apos] + j)->~T();
            }
        }
        for (size_t i = 0; i < cap; ++i) {
            delete[] reinterpret_cast<uint8_t*>(dq[i]);
        }
        delete[] dq;
    }

    explicit Deque(int n, const T& val = T());

    Deque& operator=(const Deque& c) {
        Deque<T> copy = c;
        miniArray::swap(top, copy.top);
        miniArray::swap(bottom, copy.bottom);
        std::swap(dq_size, copy.dq_size);
        std::swap(cap, copy.cap);
        std::swap(*dq, *copy.dq);
        return *this;
    }

    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    const T& at(size_t index) const;
    size_t size() const {
        return dq_size;
    }

    void push_back(T val);
    void push_front(T val);
    void pop_back();
    void pop_front();

    template<bool is_const>
    class Iter;

    using const_iterator = Iter<true>;
    using iterator = Iter<false>;
    using const_reverse_iterator = std::reverse_iterator<Iter<true>>;
    using reverse_iterator = std::reverse_iterator<Iter<false>>;

    iterator begin() {
        return iterator(dq + top.apos, top.edge);
    }
    iterator end() {
        if (bottom.edge == minicap - 1) {
            return iterator(dq + bottom.apos + 1, 0);
        }
        return iterator(dq + bottom.apos, bottom.edge + 1);
    }

    const_iterator begin() const {
        return const_iterator(dq + top.apos, top.edge);
    }
    const_iterator end() const {
        if (bottom.edge == minicap - 1) {
            return const_iterator(dq + bottom.apos + 1, 0);
        }
        return const_iterator(dq + bottom.apos, bottom.edge + 1);
    }

    const_iterator cbegin() const {
        return const_iterator(dq + top.apos, top.edge);
    }
    const_iterator cend() const {
        if (bottom.edge == minicap - 1) {
            return const_iterator(dq + bottom.apos + 1, 0);
        }
        return const_iterator(dq + bottom.apos, bottom.edge + 1);
    }

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

    void insert(Deque::iterator it, const T& val) {
        push_front(val);
        for (iterator i = begin(); i < it - 1; ++i) {
            T t = *i;
            *i = *(i + 1);
            *(i + 1) = t;
        }
    }

    void erase(Deque::iterator it) {
        for (iterator i = it; i > begin(); --i) {
            T t = *i;
            *i = *(i - 1);
            *(i - 1) = t;
        }
        pop_front();
    }
};

template <typename T>
template <bool is_const>
class Deque<T>::Iter {
private:
    T** apoint;
    size_t pos;
public:
    using reference = std::conditional_t<is_const, const T&, T&>;
    using pointer = std::conditional_t<is_const, const T*, T*>;
    Iter(T** apoint, size_t pos): apoint(apoint), pos(pos) {};
    Iter& operator++() {
        if (pos < minicap - 1) {
            ++pos;
        } else {
            ++apoint;
            pos = 0;
        }
        return *this;
    };
    Iter& operator--() {
        if (pos > 0) {
            --pos;
        } else {
            --apoint;
            pos = minicap - 1;
        }
        return *this;
    }
    Iter operator++(int) {
        Iter it = *this;
        ++(*this);
        return it;
    };
    Iter operator--(int) {
        Iter it = *this;
        --(*this);
        return it;
    }
    Iter<is_const> operator+(const int& n) const {
        auto p = static_cast<int>(pos);
        if(p + n >= 0) {
            return Iter<is_const>(apoint + (p + n) / minicap, (p + n) % minicap);
        } else {
            return Iter<is_const>(apoint - 1 - (-n - p - 1) / minicap,
                                  (minicap - (-n - p) % minicap) % minicap);
        }
    }
    Iter<is_const> operator-(const int& n) const {
        return *this + (-n);
    }

    int operator-(const Iter<is_const>& it) const {
        return (apoint - it.apoint) * minicap + pos - it.pos;
    }

    bool operator==(const Iter<is_const>& it) const {
        return apoint == it.apoint && pos == it.pos;
    }
    bool operator!=(const Iter<is_const>& it) const {
        return apoint != it.apoint || pos != it.pos;
    }
    bool operator<(const Iter<is_const>& it) const {
        return (apoint < it.apoint) || (apoint == it.apoint && pos < it.pos);
    }
    bool operator>=(const Iter<is_const>& it) const {
        return !(*this < it);
    }
    bool operator>(const Iter<is_const>& it) const {
        return (apoint > it.apoint) || (apoint == it.apoint && pos > it.pos);
    }
    bool operator<=(const Iter<is_const>& it) const {
        return !(*this > it);
    }
    reference operator*() const {
        return *(*apoint + pos);
    }
    pointer operator->() {
        return *apoint + pos;
    }
};

template<typename T>
struct Deque<T>::miniArray {
    size_t apos = 0;
    size_t edge = 0;
    miniArray(size_t apos, size_t edge): apos(apos), edge(edge) {};
    static void swap(miniArray& arr1, miniArray& arr2) {
        std::swap(arr1.apos, arr2.apos);
        std::swap(arr1.edge, arr2.edge);
    }
};

template<typename T>
size_t Deque<T>::get_apos(size_t index) const {
    return top.apos + (top.edge + index) / minicap;
}
template<typename T>
size_t Deque<T>::get_minipos(size_t index) const {
    return (top.edge + index) % minicap;
}

template<typename T>
T& Deque<T>::operator[](size_t index) {
    return dq[get_apos(index)][get_minipos(index)];
}
template<typename T>
const T &Deque<T>::operator[](size_t index) const {
    return dq[get_apos(index)][get_minipos(index)];
}

template<typename T>
T &Deque<T>::at(size_t index) {
    if (index >= dq_size) {
        throw std::out_of_range("at");
    }
    return dq[get_apos(index)][get_minipos(index)];
}
template<typename T>
const T &Deque<T>::at(size_t index) const {
    if (index >= dq_size) {
        throw std::out_of_range("at");
    }
    return dq[get_apos(index)][get_minipos(index)];
}

template<typename T>
void Deque<T>::push_back(T val) {
    if (dq_size > 0) {
        if (bottom.apos == cap - 1 && bottom.edge == minicap - 1) {
            reserve(0, cap);
        }
        if (bottom.edge == minicap - 1) {
            ++bottom.apos;
            bottom.edge = 0;
        }
        else {
            ++bottom.edge;
        }
    }
    ++dq_size;
    try {
        new(dq[bottom.apos] + bottom.edge) T(val);
    } catch (...) {
        if (bottom.edge == 0) {
            --bottom.apos;
            bottom.edge = minicap - 1;
        }
        else {
            --bottom.edge;
        }
        --dq_size;
    }
}

template<typename T>
void Deque<T>::push_front(T val) {
    if (dq_size > 0) {
        if (top.apos == 0 && top.edge == 0) {
            reserve(cap, 0);
        }
        if (top.edge == 0) {
            --top.apos;
            top.edge = minicap - 1;
        } else {
            --top.edge;
        }
    }
    ++dq_size;
    try {
        new(dq[top.apos] + top.edge) T(val);
    } catch (...) {
        if (top.edge == minicap - 1) {
            --top.apos;
            top.edge = 0;
        } else {
            --top.edge;
        }
        --dq_size;
    }
}

template<typename T>
void Deque<T>::pop_back() {
    if (dq_size == 0) return;
    (dq[bottom.apos] + bottom.edge)->~T();
    if (bottom.edge == 0) {
        --bottom.apos;
        bottom.edge = minicap - 1;
    } else {
        --bottom.edge;
    }
    --dq_size;
}

template<typename T>
void Deque<T>::pop_front() {
    if (dq_size == 0) return;
    (dq[top.apos] + top.edge)->~T();
    if (top.edge == minicap - 1) {
        ++top.apos;
        top.edge = 0;
    } else {
        ++top.edge;
    }
    --dq_size;
}

template<typename T>
void Deque<T>::reserve(size_t up_cap, size_t down_cap) {
    T** ndq = new T*[up_cap + cap + down_cap];
    size_t exit = 0;
    try {
        for (size_t i = 0; i < up_cap + cap + down_cap; ++i) {
            if (i < up_cap || i >= up_cap + cap)
                ndq[i] = reinterpret_cast<T*>(new uint8_t[minicap * sizeof(T)]);
            else ndq[i] = dq[i - up_cap];
            ++exit;
        }
    } catch (...) {
        for (size_t i = 0; i < exit; ++i) {
            delete[] reinterpret_cast<uint8_t*>(ndq[i]);
        }
        delete[] ndq;
        throw;
    }
    delete dq;
    dq = ndq;
    cap = up_cap + cap + down_cap;
    top.apos += up_cap;
    bottom.apos += up_cap;
}

template<typename T>
Deque<T>::Deque(int n, const T& val): dq_size(n), top(0, 0), bottom(0, minicap - 1) {
    auto m = static_cast<size_t>(n);
    if (m > minicap) {
        cap = (m + minicap - 1) / minicap;
        top = miniArray(0, 0);
        bottom = miniArray(cap - 1, (m - 1) % minicap);
    } else {
        cap = 4;
        top = miniArray(0, minicap - m);
        bottom = miniArray(0, minicap - 1);
    }
    dq = new T*[cap];
    size_t exit = 0, exi = 0, exj = 0;
    try {
        for (size_t i = 0; i < cap; ++i) {
            dq[i] = reinterpret_cast<T*>(new uint8_t[minicap * sizeof(T)]);
            ++exit;
        }
        for (size_t j = top.edge; j < minicap; ++j) {
            new(dq[top.apos] + j) T(val);
            exj = j + 1;
        }
        for (size_t i = top.apos + 1; i < bottom.apos; ++i) {
            for (size_t j = 0; j < minicap; ++j) {
                new(dq[i] + j) T(val);
                exj = j + 1;
            }
            exi = i + 1;
        }
        if (bottom.apos != top.apos) {
            for (size_t j = 0; j <= bottom.edge; ++j) {
                new(dq[bottom.apos] + j) T(val);
                exj = j + 1;
            }
        }
    } catch (...) {
        for (size_t j = top.edge; j < (exi == top.apos ? exj : minicap); ++j) {
            (dq[top.apos] + j)->~T();
        }
        for (size_t i = top.apos + 1; i < exi; ++i) {
            for (size_t j = 0; j < minicap; ++j) {
                (dq[i] + j)->~T();
            }
        }
        if (exi != top.apos) {
            for (size_t j = 0; j < exj; ++j) {
                (dq[exi] + j)->~T();
            }
        }
        for (size_t i = 0; i < exit; ++i) {
            delete[] reinterpret_cast<uint8_t*>(dq[i]);
        }
        delete[] dq;
        throw;
    }
}