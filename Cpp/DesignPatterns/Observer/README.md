# Observer Pattern

Implementation of the Observer design pattern with safe memory management.

## Observer.cpp

My own safe Observer pattern implementation that handles the case where the Observer can be destroyed before the Observable.

### Compilation

```bash
g++ --std=c++11 -W -Wall -Wextra Observer.cpp -o prog -pthread
```

## Sigc.cpp

Test of all features of libsigc++ (used for GTKmm, a C++ wrapper of GTK+).

### Compilation

```bash
g++ -W -Wall -Wextra --std=c++11 Sigc.cpp -o Sigc `pkg-config --cflags --libs sigc++-2.0`
```

### Prerequisites

You may need to install it first:

```bash
sudo apt-get install libsigc++-2.0-dev pkg-config
```
