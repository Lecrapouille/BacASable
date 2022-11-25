#include <iostream>
#include <vector>
#include <string>
#include <memory>

class Visitable; class A; class B; class C;

// ============================================================================
//! \brief Base class of visitor. Dummy visit() method used as fallbacks.
// ============================================================================
class Visitor
{
public:

    virtual ~Visitor() = default;
    virtual void visit(Visitable&) {};
    virtual void visit(A&) {};
    virtual void visit(B&) {};
    virtual void visit(C&) {};
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
//! \brief First concrete vistable class an used as based class for B and C.
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
        visitor.visit(*this);
    }

    void fooA()
    {
        std::cout << m_name << "::fooA()" << std::endl;
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
        visitor.visit(*this);
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
        visitor.visit(*this);
    }

    void fooC(std::string const bar)
    {
        std::cout << m_name << "::fooC(" << bar << ")" << std::endl;
    }
};

// ============================================================================
//! \brief First concrete vistor visiting class A.
// ============================================================================
class VisitorA: public Visitor
{
public:

    virtual void visit(A& a) override
    {
        a.fooA();
    }
};

// ============================================================================
//! \brief First concrete vistor visiting class B.
// ============================================================================
class VisitorB: public Visitor
{
public:

    virtual void visit(B& b) override
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

    virtual void visit(A& a) override
    {
        a.fooA();
    }

    virtual void visit(C& c) override
    {
        c.fooC("Coucou");
    }
};

// ============================================================================
// g++ -W -Wall --std=c++14 00_BasicVisitor.cpp -o visitor
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

    return 0;
}
