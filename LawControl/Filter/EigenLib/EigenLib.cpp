#include <iostream>
#include <Eigen/Dense>

// Already defined
// using Vector3f = Eigen::Matrix<float, 3, 1>;

// g++ --std=c++11 -Wall -Wextra EigenLib.cpp `pkg-config --cflags --libs eigen3`
int main()
{
  Eigen::Vector3f a(1.0f, 2.0f, 3.0f);
  Eigen::Vector3f b(1.0f, 2.0f, 3.0f);

  Eigen::Vector3f c = (a + b);
  std::cout << c << std::endl;

  return 0;
}
