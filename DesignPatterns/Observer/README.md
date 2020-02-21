# Observers

- Observer.cpp: my own safe Observer pattern.
  ```
  g++ --std=c++11 -W -Wall -Wextra Observer.cpp -o prog -pthread
  ```

- Sigc.cpp: Test all features of the libsigc++ (used for GTKmm a C++ wrapper of GTK+)
  ```
  g++ -W -Wall -Wextra --std=c++11 Sigc.cpp -o Sigc `pkg-config --cflags --libs sigc++-2.0`
  ```
  You may have to install it before:
  `sudo apt-get install libsigc++-2.0-dev pkg-config`
