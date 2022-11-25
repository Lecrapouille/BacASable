#include <iostream>
#include <vector>
#include <string>
#include <memory>

class Visitable; class A; class B; class C;

// ============================================================================
//! \brief Base class of visitor. visit() methods have been renamed as operator()
//! visit method are dummy used as fallbacks.
// ============================================================================
class Visitor
{
public:

    virtual ~Visitor() = default;

    virtual void operator()(Visitable&)
    {
        std::cout << "do generic fallback action ..." << std::endl;
    }
    virtual void operator()(A& a) { fallback(a); }
    virtual void operator()(B& b) { fallback(b); }
    virtual void operator()(C& c) { fallback(c); }

    template<typename T>
    void fallback(T& object)
    {
        std::cout << "Visitor::Fallback " << object.name() << ": ";
        this->operator()(static_cast<Visitable&>(object));
    }
};

// ============================================================================
//! \brief Base class for accepting a visitor class.
// ============================================================================
class Visitable
{
public:

    virtual ~Visitable() = default;
    virtual void accept(Visitor&) {};
};

// ============================================================================
//! \brief First concrete vistable class used as based class.
// ============================================================================
class A: public Visitable
{
public:

    A()
    {
        m_name = "A";
    }

    virtual void accept(Visitor& visitor) override
    {
        visitor(*this);
    }

    void fooA()
    {
        std::cout << m_name << "::fooA()" << std::endl;
    }

    inline std::string const& name() const
    {
        return m_name;
    }

protected:

    std::string m_name;
};

// ============================================================================
//! \brief Second concrete vistable class.
// ============================================================================
class B: public A
{
public:

    B()
    {
        m_name = "B";
    }

    virtual void accept(Visitor& visitor) override
    {
        visitor(*this);
    }

    void fooB(int const bar)
    {
        std::cout << m_name << "::fooB(" << bar << ")" << std::endl;
    }
};

// ============================================================================
//! \brief Thrid concrete vistable class deriving from the second class.
// ============================================================================
class C: public B
{
public:

    C()
    {
        m_name = "C";
    }

    virtual void accept(Visitor& visitor) override
    {
        visitor(*this);
    }

    void fooC(std::string const bar)
    {
        std::cout << m_name << "::fooC(" << bar << ")" << std::endl;
    }
};

// ============================================================================
//! \brief Visitor managng fallback.
// ============================================================================
class FallBackVisitor: public Visitor
{
public:

    virtual void operator()(A& a) override
    {
        fallback(a);
    }

    virtual void operator()(B& b) override
    {
        fallback(b);
    }

    virtual void operator()(C& c) override
    {
        fallback(c);
    }
};

// ============================================================================
//! \brief First concrete vistor visiting class A.
// ============================================================================
class VisitorA: public FallBackVisitor
{
public:

    virtual void operator()(A& a) override
    {
        a.fooA();
    }
};

// ============================================================================
//! \brief First concrete vistor visiting class B.
// ============================================================================
class VisitorB: public FallBackVisitor
{
public:

    virtual void operator()(B& b) override
    {
        b.fooB(42);
    }
};

// ============================================================================
//! \brief First concrete vistor visiting class A and class C (and therefore B).
// ============================================================================
class VisitorABC: public VisitorB
{
public:

    virtual void operator()(A& a) override
    {
        a.fooA();
    }

    virtual void operator()(C& c) override
    {
        c.fooC("Coucou");
    }
};

// ============================================================================
// g++ -W -Wall --std=c++14 01_FallbackVisitor.cpp -o visitor
// C++14 because of std::unique_ptr
// ============================================================================
int main()
{
    std::vector<std::unique_ptr<Visitable>> visitables;
    visitables.push_back(std::make_unique<A>());
    visitables.push_back(std::make_unique<B>());
    visitables.push_back(std::make_unique<B>());
    visitables.push_back(std::make_unique<C>());
    visitables.push_back(std::make_unique<A>());

    std::cout << "=== Visit A elements ===" << std::endl;
    VisitorA vA;
    for (auto& it: visitables)
    {
        it->accept(vA);
    }

    std::cout << "=== Visit B elements ===" << std::endl;
    VisitorB vB;
    for (auto& it: visitables)
    {
        it->accept(vB);
    }

    std::cout << "=== Visit ABC elements ===" << std::endl;
    VisitorABC vABC;
    for (auto& it: visitables)
    {
        it->accept(vABC);
    }

    std::cout << "=== Fallback ===" << std::endl;
    A a; a.accept(vB);

    return 0;
}
