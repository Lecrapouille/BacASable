#include <iostream>

// Read/Write Property
struct RWProperty
{
    class {
        int value;
        public:
            int & operator = (const int &i) { return value = i; }
            operator int () const { return value; }
    } alpha;

    class {
        float value;
        public:
            float & operator = (const float &f) { return value = f; }
            operator float () const { return value; }
    } bravo;
};

// Read only Property
struct ROProperty
{
    class {
        int value = 43;
        public:
            operator int () const { return value; }
    } alpha;

    class {
        float value = 2.5f;
        public:
            operator float () const { return value; }
    } bravo;
};

// Write only Property
struct WOProperty
{
    class {
        int value;
        public:
            int & operator = (const int &i) { return value = i; }
    } alpha;

    class {
        float value;
        public:
            float & operator = (const float &f) { return value = f; }
    } bravo;
};

// g++ -W -Wall --std=c++11 Property.cpp -o prog
int main()
{
  // --------------- Read/Write Property
  RWProperty rw;

  // Setter
  rw.alpha = 42;
  rw.bravo = 1.5f;

  // Getter
  std::cout << "Alpha: " << rw.alpha << ", Bravo: " << rw.bravo << std::endl;

  // --------------- Read Only Property
  ROProperty ro;
  // Forbidden: ro.alpha = 42;
  std::cout << "Alpha: " << ro.alpha << ", Bravo: " << ro.bravo << std::endl;

  // --------------- Write Only Property
  WOProperty wo;
  wo.alpha = 42;
  wo.bravo = 1.5f;
  // Forbidden: std::cout << "Alpha: " << wo.alpha << ", Bravo: " << wo.bravo << std::endl;
  // Forbidden: int i = wo.alpha;

  return 0;
}
