#include <iostream>

struct U
{
    union {
        int32_t i;
        float   f;
    };
    enum { INT, FLOAT } tag;

    U()
        : i(0), tag(U::INT)
    {}

    U(int i_)
        : i(i_), tag(U::INT)
    {}

    U(float f_)
        : f(f_), tag(U::FLOAT)
    {}

    U& operator=(int const other)
    {
        this->i = other;
        this->tag = U::INT;
        return *this;
    }

    U& operator=(float const other)
    {
        this->f = other;
        this->tag = U::FLOAT;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& os, const U& u)
{
    switch (u.tag)
    {
    case U::INT: std::cout << u.i; break;
    case U::FLOAT: default: std::cout << u.f; break;
    }
    return os;
}

template <typename T>
class Stack
{
public:

    Stack()
        : sp(sp0)
    {}

    template <typename N>
    inline void push(N const n)
    {
        *(sp++) = n;
    }

    // Inspired by
    // https://github.com/opentracing/opentracing-cpp/blob/master/include/opentracing/span.h#L174
    inline void push(std::initializer_list<T> const& list)
    {
        for (auto& it: list)
            *(sp++) = it;
    }

    inline void drop() { --sp; }

    //template <typename N>
    //inline N pop() { return *reinterpret_cast<N*>(&((--sp)->f)); }

    template <typename N>
    inline N pop()
    {
        U u = *(--sp);
        switch (u.tag)
        {
        case U::INT: return N(u.i);
        case U::FLOAT:
        default:
            return N(u.f);
        }
    }

    inline T& pick(int const n) { return *(sp - n - 1); }
    std::ostream& display(std::ostream& os)
    {
        os << "Stack:";
        T* s = sp0;
        while (s != sp)
            os << ' ' << (*s++);

        os << std::endl;
        return os;
    }

private:

    T data[16];
    T* const sp0 = data;
    T* sp;
};

static Stack<U> s;

int main()
{
    std::cout << "Push: " << std::endl;
    s.push(40);
    s.push(5.6f);
    s.push({ 5.6f, 40, 41, 5.7f});
    s.display(std::cout);

    std::cout << "Float operation: " << std::endl;
    s.push(s.pop<float>() + s.pop<float>());
    s.display(std::cout);

    std::cout << "Integer operation: " << std::endl;
    s.push(s.pop<int32_t>() + 50);
    s.display(std::cout);

    return 0;
}
