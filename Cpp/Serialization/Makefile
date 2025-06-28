CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = serialization_test
SOURCES = main.cpp
HEADERS = Serialization.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean