#pragma once

// Trick to intercept Publisher with #pragma once
// Define a macro to mask the real Publisher class from the original Publisher.h
#define Publisher _Publisher_Real

// Include the real Publisher.h (which will be masked by the macro)
#include "Publisher.h"

// Undefine the macro so we can define our mock Publisher class
#undef Publisher

#include <gmock/gmock.h>

// Publisher mock for tests
class Publisher {
public:
    Publisher() = default;

    void publish(int arg1, int arg2) {
        if (currentMock) {
            currentMock->publish(arg1, arg2);
        }
    }

    // Mock interface
    class Mock {
    public:
        MOCK_METHOD(void, publish, (int arg1, int arg2));
    };

    static Mock* currentMock;
};

// Static member definition
inline Publisher::Mock* Publisher::currentMock = nullptr;
