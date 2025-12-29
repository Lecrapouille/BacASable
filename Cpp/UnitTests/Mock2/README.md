# Intercept Arguments from a Non-Virtual Class Hidden Inside a PIMPL Method

Good practice says not to unit test private methods or PIMPL since they are an implementation detail of a given API (public methods). But sometimes we want to verify. So here's a hack!

## Overview

- `MainClass`: The class we want to unit test.
- `Publisher`: The nested class inside a PIMPL implementation with non-virtual methods. We want to capture arguments passed to its `publish` method.
- `MockPublisher.h`: A mock class to capture arguments passed to its `publish` method.
- `test_mainclass.cpp`: We include `#include "MainClass.cpp"` to access the PIMPL implementation. Inside `MockPublisher.h` we define a macro to mask the real Publisher class from the original Publisher.h:
  - `#define Publisher _Publisher_Real`
  - Then we include the real Publisher.h (which will be masked by the macro): `#include "Publisher.h"`
  - Followed by undefining the macro so we can define our mock Publisher class: `#undef Publisher`
