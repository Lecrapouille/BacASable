#include <vector>
#include <numeric>
#include <iostream>

using Real = float;

// https://youtu.be/VlNr4aqL2bM
// https://stackoverflow.com/questions/10330002/sum-of-small-Real-numbers-c
struct KahanAccumulation
{
    Real sum = 0.0f;
    Real correction = 0.0f;
};

KahanAccumulation KahanSum(KahanAccumulation accumulation, Real value)
{
    KahanAccumulation result;
    Real y = value - accumulation.correction;
    Real t = accumulation.sum + y;
    result.correction = (t - accumulation.sum) - y;
    result.sum = t;
    return result;
}

Real randomize(Real fMin, Real fMax)
{
    Real f = (Real)rand() / RAND_MAX;
    return fMin + f*(fMax - fMin); //returns random value
}

int main()
{
  std::vector<Real> numbers(1000u, 0.01f);
  //for (size_t i = 0u; i < numbers.size(); ++i) {
  //  numbers[i] = randomize(0.0f, 1.0f);
  //}

  // Naive mean: the result is less and less good when the size of the array increases
  {
    Real sum = std::accumulate(numbers.begin(), numbers.end(), 0.0f);
    std::cout << "naive: " << sum / numbers.size() << std::endl;
  }

  // Mean with result normalization: poor results because it acts like a moving average
  {
    Real result = 0.0f;
    for (size_t i = 0u; i < numbers.size(); ++i) {
      result = (result + numbers[i] * (numbers.size() - 1u)) / numbers.size();
    }
    std::cout << "normalized: " << result << std::endl;
  }

  // Kahan summation: works well  when -ffast-math is enabled
  {
    KahanAccumulation init;
    KahanAccumulation res = std::accumulate(numbers.begin(), numbers.end(), init, KahanSum);
    std::cout << "Kahan Sum: " << res.sum / numbers.size() << std::endl;
  }

  return 0;
}
