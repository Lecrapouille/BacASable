// Personal implementation of Youtube video "Better Code: Runtime Polymorphism"
// by Sean Parent. See original video, document on
// https://sean-parent.stlab.cc/papers-and-presentations/#better-code-runtime-polymorphism

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cassert>
#include <future>

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
        : m_self(std::make_unique<Model<T>>(std::move(x)))
    {
        std::cout << "Object ctor" << std::endl;
    }

    // Guidelines:
    // - The semantics of copy are to create a new object which is equal to and
    //   logically disjoint from the original.
    // - Copy constructor must copy the object. The compiler is free to elide
    //   copies so if the copy constructor does something else the code is
    //   incorrect.
    // - When a type manages 'remote parts' it is necessary to supply a copy
    //   constructor. If you can, use an existing class (such as vector) to -
    //   manage remote parts.
    Object(const Object& x)
        // virtualized copy. Equivalent to :
        // m_self(std::make_unique<Model<T>>(*x.m_self))
        : m_self(x.m_self->copy())
    {
        std::cout << "Object copy" << std::endl;
    }

    // Assign operator guidelines:
    // - Assignement is consistent with copy. Generally
    //   T X; x = y; is equivalent to T X = y;
    // - Assignements satisfying the "strong exception guarantee" is a nice
    //   property: either complete successfully or throw an exception, leaving
    //   the object unchanged.
    // - Assignement (like all other operations) must statisfy the basic execption
    //   guarantee.
    // - Do not optimize for rare cases which impact common cases: do not test
    //   for self-assignement to avoid the copy.
    //
    // Bad way:
    // Object& operator=(const Object& x)
    // {
    //     Object tmp(x);
    //     m_self = std::move(tmp.m_self);
    //     return *this;
    // }
    // Guideline:
    // - Pass sink arguments by value and swap or move into place.
    // - A sink argument is any argument consumed or returned by the function:
    //   the argument to assignement is a sink argument. However, because of a
    //   language defect, you must write a move assignement operator.
    // Object& operator=(const Object& x)
    // {
    //     Object tmp(x);
    //     *this = std::move(tmp);
    //     return *this;
    // }
    Object& operator=(const Object& x)
    {
        return *this = Object(x);
    }

    // Move assignement operator.
    Object& operator=(Object&&) noexcept = default;

    // Provide a move constructor and move assignment to avoid copies and get
    // fast permutations.
    //
    // When calling std::reverse(container.begin(), container.end()); data are
    // copied to a temporary and then copy across pairwise swap(). We have to
    // provide our own swap. We use Move constructor since the swap(), since
    // C++14, is written in terms of moves.
    //   Object(Object&& x) noexcept
    //       : m_self(std::move(x.m_self))
    //   {}
    // Shorter and equivalent code:
    Object(Object&& x) noexcept = default;

    friend void draw(const Object& o, std::ostream& out, size_t position)
    {
        o.m_self->draw(out, position);
    }

private:

    struct Concept
    {
        virtual ~Concept() = default;
        virtual std::unique_ptr<Concept> copy() const = 0;
        virtual void draw(std::ostream& out, size_t position) const = 0;
    };

    template<typename T>
    struct Model final: Concept
    {
        Model(T x)
            : m_data(std::move(x))
        {}

        virtual std::unique_ptr<Concept> copy() const override
        {
            return std::make_unique<Model<T>>(*this);
        }

        virtual void draw(std::ostream& out, size_t position) const override
        {
            ::draw(m_data, out, position);
        }

        T m_data;
    };

    // Do your own memory mamangement: do not create garbage for your client to
    // clean-up:
    std::unique_ptr<Concept> m_self;
    // But:
    // A shared pointer to an immuable (const) object has value sementics: this
    // is why passing arguments by const&= reference works.
    // Mutable polymorphic objects are the exception.
    // Copy-on-write can be obtained using shared_ptr::unique()
    //std::shared_ptr<const Concept> m_self;
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
// g++ -W -Wall --std=c++17 RuntimePolymorphism.cpp -o prog -lpthread
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

        auto saving = std::async([container = current(h)]()
        {
            std::cout << "Saving (3s) ..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            std::cout << "---- SAVED ----" << std::endl;
            draw(container, std::cout, 0);
        });

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
