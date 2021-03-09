#include <symengine/basic.h>
#include <symengine/add.h>
#include <symengine/symbol.h>
#include <symengine/dict.h>
#include <symengine/integer.h>
#include <symengine/mul.h>
#include <symengine/pow.h>

#include <iostream>

using S = SymEngine::RCP<const SymEngine::Basic>;

// https://github.com/symengine/symengine
// g++ -W -Wall --std=c++11 SymEngine.cpp -o prog -lsymengine -lgmp
int main()
{
    S x = SymEngine::symbol("x");
    S y = SymEngine::symbol("y");

    S e = mul(add(x, y), add(x, y));
    std::cout << "Formula: " << *e << std::endl;

    S r = expand(e);
    std::cout << "Expanding: " << *r << std::endl;

    S i60 = SymEngine::integer(60);
    r = r->subs({{x, i60}, {y, i60}});
    std::cout << "Result: " << *r << std::endl;

    return 0;
}
