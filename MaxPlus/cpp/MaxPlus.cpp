// Compilation: g++ -W -Wall --std=c++11 MaxPlus.cpp -o prog
#include <chrono>
#include <ctime>
#include <algorithm> // for std::max
#include <iostream>
#include <cassert>
#include <limits>

#include "ThirdPart/Matrix.tpp"

template<class T>
class MaxPlus
{
public:

  MaxPlus() : val() {};
  MaxPlus(const MaxPlus & d) : val(d.val){}
  MaxPlus(const T t) : val(t){}
  inline bool operator==(const MaxPlus &rhs) const { return val == rhs.val; }
  inline bool operator==(const T &rhs) const { return val == rhs; }
  inline MaxPlus & operator=(const MaxPlus & rhs) { val = rhs.val; return *this;}
  inline MaxPlus & operator=(const T rhs) { val = rhs; return *this;}
  inline T operator*=(const MaxPlus & rhs) { val += rhs.val; return val; }
  inline T operator+=(const MaxPlus & rhs) { val = std::max(val, rhs.val); return val; }
  inline T operator*(const MaxPlus & rhs) const { return val + rhs.val; }
  inline T operator+(const MaxPlus & rhs) const { return std::max(val, rhs.val); }
  inline T operator/(const MaxPlus & rhs) const { return val - rhs.val; }
  inline T operator-(const MaxPlus & rhs) const { return val - rhs.val; }
  inline T operator-() const { return -val; }
  inline T operator+() const { return val; }
  //inline operator const T & () const { return val; }
  //inline operator T & () { return val; }

  T val;
};

template<> MaxPlus<float>  zero<MaxPlus<float>>()  { return std::numeric_limits<float>::infinity(); }
template<> MaxPlus<double> zero<MaxPlus<double>>() { return std::numeric_limits<double>::infinity(); }
template<> MaxPlus<int>    zero<MaxPlus<int>>()    { return std::numeric_limits<int>::min(); }
template<> MaxPlus<float>  one<MaxPlus<float>>()   { return 0; }
template<> MaxPlus<double> one<MaxPlus<double>>()  { return 0; }
template<> MaxPlus<int>    one<MaxPlus<int>>()     { return 0; }

inline std::ostream& operator<<(std::ostream& os, MaxPlus<float> const& m)
{
  std::cout << m.val;
  return os;
}

inline std::ostream& operator<<(std::ostream& os, MaxPlus<int> const& m)
{
  std::cout << m.val;
  return os;
}

int main()
{
  MaxPlus<float> a(3.0f);
  MaxPlus<float> b(5.0f);

  assert(a == (a + a));
  assert(6.0f == (a * a));
  assert(b == (a + b));
  assert(8.0f == (a * b));

  assert(0.0f == (b / b));
  assert(0.0f == (b - b));

  MaxPlus<float> inf(std::numeric_limits<float>::infinity());
  Matrix<MaxPlus<float>, 2u, 2u> A = {4.0f, 3.0f, 7.0f, -inf};
  //assert(A, (A + A)); // FIXME bug in Matrix.tpp
  std::cout << A <<std::endl;
  std::cout << (A + A) << std::endl;

  Matrix<MaxPlus<float>, 2u, 2u> ResMul = {10.0f, 7.0f, 11.0f, 10.0f};
  //assert(ResMul, (A + A)); // FIXME bug in Matrix.tpp
  std::cout << ResMul <<std::endl;
  std::cout << (A * A) << std::endl;

  std::cout << "Float Identity matrix" << std::endl;
  std::cout << Matrix<MaxPlus<float>, 2u, 2u>(matrix::Identity) << std::endl;

  std::cout << "Integer Identity matrix" << std::endl;
  std::cout << Matrix<MaxPlus<int>, 2u, 2u>(matrix::Identity) << std::endl;

  return 0;
}
