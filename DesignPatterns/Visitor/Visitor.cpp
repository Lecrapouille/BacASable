#include <iostream>
#include <vector>

class A;
class B;

class IVisitor
{
public:

    virtual ~IVisitor() = default;
    virtual void visit(A const& a) = 0;
    virtual void visit(B const& b) = 0;
};

class IVisitee
{
public:

    virtual ~IVisitee() = default;
    virtual void accept(IVisitor& visitor) = 0;
};

class A: public IVisitee
{
public:

    void accept(IVisitor& visitor) override
    {
        visitor.visit(*this);
    }

    int a = 42;
};

class B: public IVisitee
{
public:

    void accept(IVisitor& visitor) override
    {
        visitor.visit(*this);
    }

    int b = 45;
};

class Visitor: public IVisitor
{
public:

    virtual void visit(A const& a) override
    {
        std::cout << a.a << std::endl;
    }

    virtual void visit(B const& b) override
    {
        std::cout << b.b << std::endl;
    }
};

// g++ -W -Wall --std=c++11 Visitor.cpp -o prog
int main()
{
    Visitor visitor;
    std::vector<IVisitee*> visitees;

    visitees.push_back(new A);
    visitees.push_back(new B);
    visitees.push_back(new B);
    visitees.push_back(new A);

    for (auto& it: visitees)
    {
        it->accept(visitor);
    }

    // ... call delete here
    return 0;
}
