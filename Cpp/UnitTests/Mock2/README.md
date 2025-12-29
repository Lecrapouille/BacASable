# Intercept arguments from a non virtual class hidden inside a PIMPL method

Good practice said not to unit test private methods or PIMPL since they are an implementation of a given API (public methods). But sometimes we want to verify. So here a hack !
- `MainClass`: the class we want to unit test.
- `Publisher`: the nested class inside a PIMPL method with non virtual methods. We want to capture arguments passed to its `publish` method.
- `MockPublisher.h` a mock class to capture arguments passed to its `publish` method.
- `test_mainclass.cpp` we include `#include "MainClass.cpp"` to access to PIMPL implementation. Inside `MockPublisher.h` we define a macro to mask the real Publisher class from the original Publisher.h
`#define Publisher _Publisher_Real` Then we include the real Publisher.h (which will be masked by the macro)
`#include "Publisher.h"`
Followed by undefine the macro so we can define our mock Publisher class
`#undef Publisher`
