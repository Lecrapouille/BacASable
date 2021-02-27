// Personal implementation of Youtube video "Better Code: Runtime Polymorphism"
// by Sean Parent. See original video, document on
// https://sean-parent.stlab.cc/papers-and-presentations/#better-code-runtime-polymorphism

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>

// -----------------------------------------------------------------------------
template<typename T>
static void draw(const T& o, std::ostream& out, size_t position)
{
    out << std::string(position, ' ') << o << std::endl;
}

// -----------------------------------------------------------------------------
class Object
{
public:

    // make_unique: heap allocating.
    // No more passing x as const reference since constructor frequently uses
    // sink arguments.
    template<typename T>
    Object(T x)
        : m_self(std::make_shared<Model<T>>(std::move(x)))
    {
        std::cout << "Object ctor" << std::endl;
    }

    friend void draw(const Object& o, std::ostream& out, size_t position)
    {
        o.m_self->draw(out, position);
    }

private:

    struct Concept
    {
        virtual ~Concept() = default;
        virtual void draw(std::ostream& out, size_t position) const = 0;
    };

    template<typename T>
    struct Model final: Concept
    {
        Model(T x)
            : m_data(std::move(x))
        {}

        virtual void draw(std::ostream& out, size_t position) const override
        {
            ::draw(m_data, out, position);
        }

        T m_data;
    };

    // Do your own memory mamangement: do not create garbage for your client to
    // clean-up: std::unique_ptr<Concept> m_self;
    // But:
    // A shared pointer to an immuable (const) object has value sementics: this
    // is why passing arguments by const&= reference works.
    // Mutable polymorphic objects are the exception.
    // Copy-on-write can be obtained using shared_ptr::unique()
    std::shared_ptr<const Concept> m_self;
};

// *****************************************************************************
// Example
// *****************************************************************************

// The runtime-concept idiom allows polymorphism when needed without inheritance.
// Client is not burdened with inheritance, factories, class registrartion and
// memory management. Penalty of runtime polymorphism is only payed when needed.
class MyClass // Inherits from nothing
{
    // ...
};

template<>
void draw(const MyClass& /*c*/, std::ostream& out, size_t position)
{
    out << std::string(position, ' ') << "MyClass" << std::endl;
}

// -----------------------------------------------------------------------------

using Container = std::vector<Object>;

template<>
void draw(const Container& c, std::ostream& out, size_t position)
{
    out << std::string(position, ' ') << "<document>" << std::endl;
    for (const auto& o: c)
    {
        draw(o, out, position + 2u);
    }
    out << std::string(position, ' ') << "</document>" << std::endl;
}

// -----------------------------------------------------------------------------

using History = std::vector<Container>;

void commit(History& x)
{
    assert(x.size());
    x.push_back(x.back());
}

void undo(History& x)
{
    assert(x.size());
    x.pop_back();
}

Container& current(History& x)
{
    assert(x.size());
    return x.back();
}

// -----------------------------------------------------------------------------
// g++ -W -Wall --std=c++17 RuntimePolymorphism.cpp -o prog
// c++14 because of unique_ptr
// c++17 because of std::reverse
int main()
{
    {
        std::cout << "\nContainer constructor:" << std::endl;
        Container container;
        container.reserve(10u);

        std::cout << "\nEmplace back Objects:" << std::endl;
        container.emplace_back(0);
        container.emplace_back(std::string("Hello World"));
        container.emplace_back(2);
        container.emplace_back(3);
        container.emplace_back(MyClass{});

        std::cout << "\nPut my container into my container:" << std::endl;
        // Put my container into my container. This will not crash by recurisivity
        container.emplace_back(container);
        std::cout << "\nReverse container:" << std::endl;
        std::reverse(container.begin(), container.end());
        draw(container, std::cout, 0);
    }
    {
        History h(1);

        std::cout << "\nModif 1:" << std::endl;
        current(h).emplace_back(0);
        current(h).emplace_back(std::string("Hello World"));
        draw(current(h), std::cout, 0);
        commit(h);

        std::cout << "\nModif 2:" << std::endl;
        current(h)[0] = 42.5;
        current(h)[1] = std::string("coucou");
        current(h).emplace_back(MyClass{});
        current(h).emplace_back(current(h));
        draw(current(h), std::cout, 0);

        std::cout << "\nUndo:" << std::endl;
        undo(h);
        draw(current(h), std::cout, 0);
    }

    return 0;
}
