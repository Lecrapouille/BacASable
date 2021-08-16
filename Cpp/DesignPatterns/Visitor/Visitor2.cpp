#include <iostream>
#include <vector>

class IVisitee;
class A;
class B;

class IVisitor
{
public:

    virtual ~IVisitor() = default;

    // Methods visit()
    virtual void operator()(IVisitee&) { std::cout << "IVisitee do nothing" << std::endl; }
    virtual void operator()(A&) { std::cout << "A:: do nothing" << std::endl; }
    virtual void operator()(B&) { std::cout << "B:: do nothing" << std::endl; }

    template<typename T>
    void fallback(T& object)
    {
        std::cout << "Fallback + ";
        this->operator()(static_cast<IVisitee&>(object));
    }
};

class IVisitee
{
public:

    virtual ~IVisitee() = default;
    virtual void accept(IVisitor&) {}
};

class A: public IVisitee
{
public:

    void accept(IVisitor& visitor) override
    {
        visitor(*this);
    }

    int a = 42;
};

class B: public IVisitee
{
public:

    void accept(IVisitor& visitor) override
    {
        visitor(*this);
    }

    int b = 45;
};

class FallBackVisitor: public IVisitor
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
};

class Visitor1: public FallBackVisitor
{
public:

    virtual void operator()(A& a) override
    {
        std::cout << a.a << std::endl;
    }
};

class Visitor2: public IVisitor
{
public:

    virtual void operator()(A& a) override
    {
        std::cout << a.a << std::endl;
    }
};

// g++ -W -Wall --std=c++11 Visitor2.cpp -o prog
int main()
{
    Visitor1 visitor1;
    Visitor2 visitor2;
    std::vector<IVisitee*> visitees;

                               // Visitor1                         Visitor2
    visitees.push_back(new A); // a.a                              a.a
    visitees.push_back(new B); // fallback + IVisitee do nothing   B:: do nothing
    visitees.push_back(new B); // fallback + IVisitee do nothing   B:: do nothing
    visitees.push_back(new A); // a.a                              a.a

    std::cout << "Visitor1:" << std::endl;
    for (auto& it: visitees)
    {
        it->accept(visitor1);
    }

    std::cout << std::endl << "Visitor2:" << std::endl;
    for (auto& it: visitees)
    {
        it->accept(visitor2);
    }

    // ... call delete here
    return 0;
}
