#include <iostream>
#include <vector>
#include <string>

class BigInteger;

BigInteger operator*(const BigInteger &a, const BigInteger &b);

BigInteger operator/(const BigInteger &a, const BigInteger &b);

bool operator!=(const BigInteger &a, const BigInteger &b);

bool operator<=(const BigInteger &a, const BigInteger &b);

class BigInteger {
private:
    static const int base = 1000000000;
    static const int bucketsz = 9;  // 9 - это количество цифр в элементе вектора числа
    mutable bool sign = 1;
    std::vector<long long> num;
public:
    BigInteger(long long n) {
        long long nc = n;
        if (nc < 0) {
            sign = 0;
            nc *= -1;
        }
        if (nc == 0) {
            num.push_back(0);
        }
        while (nc != 0) {
            num.push_back(nc % base);
            nc /= base;
        }
    }

    BigInteger() : BigInteger(0ll) {}

    BigInteger(const BigInteger &a) = default;

    BigInteger(const std::string &s) {
        if (s[0] == '-') {
            sign = 0;
        }
        size_t i;
        for (i = 0; i < (s.size() - 1 + sign) / bucketsz; i++) {
            num.push_back(std::stoi(s.substr(s.size() - bucketsz * (i + 1), bucketsz)));
        }
        if ((s.size() - 1 + sign) % bucketsz > 0) {
            num.push_back(std::stoi(s.substr(1 - sign, (s.size() - 1 + sign) % bucketsz)));
        }
    }

    BigInteger &operator=(const BigInteger &a) {
        BigInteger copy = a;
        swap(copy);
        return *this;
    }

    void swap(BigInteger &a) {
        std::swap(sign, a.sign);
        std::swap(num, a.num);
    }

    BigInteger operator-() {
        BigInteger copy = *this;
        copy.changeSign();
        return copy;
    }

    BigInteger &operator+=(const BigInteger &b) {
        if (sign == b.sign) {
            return posum(b);
        } else {
            if ((sign == 0 && b.sign == 1 && absIsLess(b, *this))
                || (sign == 1 && b.sign == 0 && absIsLess(b, *this)) || num == b.num) {
                return podifbs(b);
            } else {
                changeSign();
                return podifsb(b);
            }
        }
    }

    BigInteger &posum(const BigInteger &b) {
        if (num.size() <= b.num.size()) {
            num.resize(b.num.size() + 1);
        }
        for (size_t i = 0; i < b.num.size(); ++i) {
            num[i] += b.num[i];
            if (num[i] >= base) {
                num[i] -= base;
                ++num[i + 1];
            }
        }
        removeZeros();
        return *this;
    }

    BigInteger &podifbs(const BigInteger &b) {
        size_t i;
        for (i = 0; i < b.num.size(); ++i) {
            num[i] -= b.num[i];
            if (num[i] < 0) {
                num[i] += base;
                --num[i + 1];
            }
        }
        while (i < num.size() - 1 && num[i] < 0) {
            num[i] += base;
            --num[i + 1];
            ++i;
        }
        removeZeros();
        if (num[num.size() - 1] == 0) {
            sign = 1;
        }
        return *this;
    }

    BigInteger &podifsb(const BigInteger &b) {
        if (num.size() <= b.num.size()) {
            num.resize(b.num.size() + 1);
        }
        num[num.size() - 1] = 1;
        size_t i;
        for (i = 0; i < b.num.size(); ++i) {
            num[i] -= b.num[i];
            if (num[i] < 0) {
                num[i] += base;
                --num[i + 1];
            }
        }
        while (i < num.size() - 1 && num[i] < 0) {
            num[i] += base;
            --num[i + 1];
            ++i;
        }
        removeZeros();
        num[0] = base - num[0];
        for (i = 1; i < num.size(); ++i) {
            num[i] = base - 1 - num[i];
        }
        return *this;
    }

    BigInteger &operator++() {
        *this += 1;
        return *this;
    }

    BigInteger operator++(int) {
        BigInteger copy = *this;
        ++(*this);
        return copy;
    }

    BigInteger &operator-=(const BigInteger &b) {
        changeSign();
        *this += b;
        changeSign();
        return *this;
    }

    BigInteger &operator--() {
        *this -= 1;
        return *this;
    }

    BigInteger operator--(int) {
        BigInteger copy = *this;
        --*this;
        return copy;
    }

    BigInteger &operator*=(const BigInteger &b) {
        BigInteger mult;
        if (*this == 0 || b == 0) {
            *this = 0;
            return *this;
        }
        mult.num.assign(num.size() + b.num.size() + 1, 0);
        for (size_t i = 0; i < b.num.size(); i++) {
            for (size_t j = 0; j < num.size(); j++) {
                mult.num[i + j] += (b.num[i] * num[j]);
                if (mult.num[i + j] >= base) {
                    mult.num[i + j + 1] += (mult.num[i + j] / base);
                    mult.num[i + j] = mult.num[i + j] % base;
                }
            }
        }
        mult.removeZeros();
        mult.sign = 1 - (sign + b.sign) % 2;
        *this = mult;
        return *this;
    }

    BigInteger &operator/=(const BigInteger &b) {
        BigInteger bc;
        BigInteger quot;
        BigInteger dvs;
        bool y = false;
        if (absIsLess(*this, b)) {
            *this = 0;
            return *this;
        }
        quot.sign = 1 - (sign + b.sign) % 2;
        if (b.sign == 0) {
            y = true;
            b.sign = 1;
        }
        quot.num.assign(num.size(), 0);
        int r, l, m;
        for (size_t i = num.size(); i > 0; i--) {
            dvs.shift();
            dvs.num[0] = num[i - 1];
            l = 0;
            r = base;
            while (r - l > 1) {
                bc = b;
                m = (r + l) / 2;
                bc *= m;
                if (bc <= dvs) {
                    l = m;
                } else r = m;
            }
            bc = b;
            bc *= r;
            if (bc <= dvs) {
                quot.num[i - 1] = r;
            } else quot.num[i - 1] = l;
            bc = b;
            bc *= quot.num[i - 1];
            dvs -= bc;
        }
        if (y) {
            b.sign = 0;
        }
        quot.removeZeros();
        *this = quot;
        return *this;
    }

    BigInteger& operator%=(const BigInteger &b) {
        return *this -= b * (*this / b);
    }

    std::string toString() const {
        std::string s;
        char c;
        bool t = false;
        s.reserve(bucketsz * num.size() + 1);
        if (sign == 0) {
            s += '-';
        }
        if (num[num.size() - 1] == 0) {
            s += '0';
        } else {
            for (long long tis = base / 10; tis > 0; tis /= 10) {
                c = (num[num.size() - 1] / tis) % 10 + '0';
                if (c != '0' || t) {
                    t = true;
                    s += c;
                }
            }
            for (size_t i = num.size() - 1; i > 0; i--) {
                for (long long tis = base / 10; tis > 0; tis /= 10) {
                    c = (num[i - 1] / tis) % 10 + '0';
                    s += c;
                }
            }
        }
        return s;
    }

    void shift() {
        if (!num.empty() && num[num.size() - 1] == 0) {
            return;
        }
        num.push_back(0);
        for (size_t i = num.size() - 1; i > 0; i--) {
            num[i] = num[i - 1];
        }
        num[0] = 0;
    }

    bool getSign() const {
        return sign;
    }

    void changeSign() const {
        if (num[num.size() - 1] != 0) sign = 1 - sign;
    }

    void removeZeros() {
        while (num.size() > 1 && num.back() == 0) num.pop_back();
    }

    explicit operator bool() const {
        return (num[num.size() - 1] != 0);
    }

    friend bool operator==(const BigInteger &a, const BigInteger &b);

    friend bool operator<(const BigInteger &a, const BigInteger &b);

    friend bool absIsLess(const BigInteger &a, const BigInteger &b);
};

BigInteger operator+(const BigInteger &a, const BigInteger &b) {
    BigInteger sum = a;
    sum += b;
    return sum;
}

BigInteger operator-(const BigInteger &a, const BigInteger &b) {
    BigInteger dif = a;
    dif -= b;
    return dif;
}

BigInteger operator*(const BigInteger &a, const BigInteger &b) {
    BigInteger mult = a;
    mult *= b;
    return mult;
}

BigInteger operator/(const BigInteger &a, const BigInteger &b) {
    BigInteger quot = a;
    quot /= b;
    return quot;
}

BigInteger operator%(const BigInteger &a, const BigInteger &b) {
    BigInteger rem = a;
    rem %= b;
    return rem;
}

bool operator==(const BigInteger &a, const BigInteger &b) {
    return a.sign == b.sign && a.num == b.num;
}

bool operator!=(const BigInteger &a, const BigInteger &b) {
    return !(a == b);
}

bool absIsLess(const BigInteger &a, const BigInteger &b) {
    if (a.num.size() < b.num.size()) {
        return true;
    }
    if (a.num.size() > b.num.size()) {
        return false;
    }
    for (size_t i = a.num.size(); i > 0; i--) {
        if (a.num[i - 1] < b.num[i - 1]) return true;
        if (a.num[i - 1] > b.num[i - 1]) return false;
    }
    return false;
}

bool operator<(const BigInteger &a, const BigInteger &b) {
    if (a.sign == 0 && b.sign == 0) {
        return 1 - absIsLess(a, b);
    }
    if (a.sign == 0 && b.sign == 1) {
        return true;
    }
    if (a.sign == 1 && b.sign == 0) {
        return false;
    } else return absIsLess(a, b);
}

bool operator>(const BigInteger &a, const BigInteger &b) {
    return b < a;
}

bool operator<=(const BigInteger &a, const BigInteger &b) {
    return a < b || a == b;
}

bool operator>=(const BigInteger &a, const BigInteger &b) {
    return a > b || a == b;
}

std::istream &operator>>(std::istream &in, BigInteger &num) {
    std::string s;
    in >> s;
    num = BigInteger(s);
    return in;
}

std::ostream &operator<<(std::ostream &out, const BigInteger &num) {
    out << num.toString();
    return out;
}

class Rational;

BigInteger gcd(BigInteger &p, BigInteger &q);

bool operator!=(const Rational &a, const Rational &b);

class Rational {
private:
    static const int bucketsz = 9;
    BigInteger p;
    BigInteger q = 1;
public:
    Rational(const BigInteger &a) {
        p = a;
    }

    Rational(const BigInteger &a, const BigInteger &b) {
        p = a;
        q = b;
        if(q.getSign() == 0){
            q.changeSign();
            p.changeSign();
        }
    }

    Rational(long long n) {
        p = n;
    }

    Rational() : Rational(0ll) {}

    explicit operator double() {
        return std::stod(asDecimal(30));
    }

    Rational operator-() {
        Rational copy = *this;
        copy.p.changeSign();
        return copy;
    }

    Rational &operator+=(const Rational &b) {
        p *= b.q;
        p += (b.p * q);
        q *= b.q;
        BigInteger nod = gcd(p, q);
        p /= nod;
        q /= nod;
        return *this;
    }

    Rational &operator-=(const Rational &b) {
        b.p.changeSign();
        *this += b;
        b.p.changeSign();
        return *this;
    }

    Rational &operator*=(const Rational &b) {
        if (*this == 0) {
            return *this;
        }
        p *= b.p;
        q *= b.q;
        BigInteger nod = gcd(p, q);
        p /= nod;
        q /= nod;
        return *this;
    }

    Rational &operator/=(const Rational &b) {
        if (*this == 0) {
            return *this;
        }
        p *= b.q;
        q *= b.p;
        if(q.getSign() == 0){
            q.changeSign();
            p.changeSign();
        }
        BigInteger nod;
        nod = gcd(p, q);
        p /= nod;
        q /= nod;
        return *this;
    }

    std::string toString() const {
        std::string s;
        s += p.toString();
        if (q > 1) {
            s += '/';
            s += q.toString();
        }
        return s;
    }

    std::string asDecimal(size_t presicion) const {
        std::string s;
        std::string s1 = (p / q).toString();
        s.reserve(bucketsz * s1.size() + presicion + 2);
        if(s1 == "0" && p < 0) {
            s += '-';
        }
        s += s1;
        BigInteger w = p % q;
        for (size_t i = 0; i < presicion; i++) {
            w *= 10;
        }
        w /= q;
        if (presicion != 0) {
            s += '.';
        }
        std::string s2 = w.toString();
        if(w < 0) {
            s2.erase(0, 1);
        }
        for (size_t i = 0; i < presicion - s2.size(); i++) {
            s += '0';
        }
        s += s2;
        return s;
    }

    friend bool operator==(const Rational &a, const Rational &b);

    friend bool operator<(const Rational &a, const Rational &b);
};

BigInteger gcd(BigInteger &p, BigInteger &q) {
    BigInteger a = p;
    BigInteger b = q;
    if (a.getSign() == 0) {
        a.changeSign();
    }
    if (b > a) {
        a.swap(b);
    }
    while (b != 0) {
        a %= b;
        a.swap(b);
    }
    return a;
}

Rational operator+(const Rational &a, const Rational &b) {
    Rational sum = a;
    sum += b;
    return sum;
}

Rational operator-(const Rational &a, const Rational &b) {
    Rational dif = a;
    dif -= b;
    return dif;
}

Rational operator*(const Rational &a, const Rational &b) {
    Rational mult = a;
    mult *= b;
    return mult;
}

Rational operator/(const Rational &a, const Rational &b) {
    Rational quot = a;
    quot /= b;
    return quot;
}

bool operator==(const Rational &a, const Rational &b) {
    return a.p == b.p && a.q == b.q;
}

bool operator!=(const Rational &a, const Rational &b) {
    return !(a == b);
}

bool operator<(const Rational &a, const Rational &b) {
    return a.p * b.q < a.q * b.p;
}

bool operator>(const Rational &a, const Rational &b) {
    return b < a;
}

bool operator<=(const Rational &a, const Rational &b) {
    return a < b || a == b;
}

bool operator>=(const Rational &a, const Rational &b) {
    return a > b || a == b;
}
