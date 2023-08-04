#include "PolyFit.hpp"
#include <iostream>

static void print_weights(std::vector<double> const& weights)
{
    std::cout << "Weights:" << std::endl;
    for (unsigned i = 0u; i < weights.size(); i++)
    {
        std::cout << " " << weights[i];
    }
    std::cout << std::endl;
}

static double f(double x)
{
    return 3.2 - 12.5 * x + 0.223 * x * x * x;
}

int main()
{
    PolyFit p(3);

    for (size_t i = 0u; i < 10u; i++)
    {
        p.add(i, f(i));
    }

    std::vector<double> w;
    p.weights(w);
    print_weights(w);

    std::cout << "f(3.5) = " << f(3.5) << std::endl;
    std::cout << "Estimate = " << p(3.5) << std::endl;

    return 0;
}
