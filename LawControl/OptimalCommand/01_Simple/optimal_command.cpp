#include <iostream>
#include <vector>
#include <cmath>

// Resolution with Euler, not super precise, but it illustrates the idea.
int main()
{
    const double T = 5.0; // Time horizon
    const int N = 1000; // Number of time steps
    const double dt = T / N; // Time step

    std::vector<double> x(N + 1, 0.0); // State vector (x)
    std::vector<double> lambda(N + 1, 0.0); // Costate vector (lambda)
    std::vector<double> u(N + 1, 0.0); // Control vector (u)

    // Initial condition x(0) = 1
    x[0] = 1.0;

    // Final condition lambda(T) = 0
    lambda[N] = 0.0;

    // Backward Euler for lambda (go back in time)
    for (int i = N - 1; i >= 0; --i)
    {
        lambda[i] = lambda[i + 1] + dt * 2 * x[i + 1];  // \dot{\lambda} = -2x
    }

    // Forward Euler for x (go forward in time)
    for (int i = 0; i < N; ++i)
    {
        u[i] = -lambda[i] / 2;              // optimality : u = -lambda/2
        x[i + 1] = x[i] + dt * u[i];        // \dot{x} = u
    }

    // Display some values
    std::cout << "t,x,u,lambda" << std::endl;
    for (int i = 0; i <= N; i += N / 10)
    {
        std::cout << i * dt << "," << x[i] << "," << u[i] << "," << lambda[i]
                  << std::endl;
    }

    return 0;
}
