#include <ginac/ginac.h>
#include <iostream>

// https://www.ginac.de/
// g++ -W -Wall GINaC.cpp -o prog `pkg-config ginac --libs --cflags`
int main()
{
    GiNaC::symbol x("x");
    GiNaC::symbol y("y");

    GiNaC::ex e = x * x + 3 * y;
    std::cout << "Formula: " << e << std::endl;

    GiNaC::ex der = GiNaC::diff(e, x);
    std::cout << "Derivative dx: " << der
              << " with x=7 => result = " << der.subs(x == 7)
              << std::endl;

    std::cout << "Formula " << e << " with x=7, y=5: "
              << e.subs(GiNaC::lst{x == 7, y == 5})
              << std::endl;

    return 0;
}
