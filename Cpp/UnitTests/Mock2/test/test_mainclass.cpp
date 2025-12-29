#include <gtest/gtest.h>
#include <gmock/gmock.h>

// INCLUDE HACK: Solution to intercept Publisher without changing the source code.
// 
// Strategy using #pragma once:
// test/Publisher.h uses a macro to hide the real Publisher class from Publisher.h:
// 1. It defines #define Publisher _Publisher_Real
// 2. Includes the real Publisher.h (class becomes _Publisher_Real)
// 3. Undefines Publisher and redefines Publisher as our mock
// 
// When MainClass.cpp does #include "Publisher.h", it finds test/Publisher.h
// thanks to the include directory order (test/ comes first)

// Include our mock that intercepts Publisher
#include "MockPublisher.h"

// Now include MainClass.cpp, which will use our mock
#include "MainClass.cpp"

using ::testing::_;
using ::testing::Eq;

class MainClassTest : public ::testing::Test {
protected:
    Publisher::Mock mockPublisher;

    void SetUp() override {
        // Set our mock as the global instance
        Publisher::currentMock = &mockPublisher;
    }

    void TearDown() override {
        // Clean up after each test
        Publisher::currentMock = nullptr;
    }
};

TEST_F(MainClassTest, PublishCallsPublisherWith42And100) {
    // Set expectation: publish(42, 100) must be called once
    EXPECT_CALL(mockPublisher, publish(42, 100))
        .Times(1);

    // Run code under test
    MainClass mainClass;
    mainClass.publish();

    // Google Mock verifies the expectations automatically
}

TEST_F(MainClassTest, PublishIsCalledOnce) {
    // Set expectation: publish called with any two integers, once
    EXPECT_CALL(mockPublisher, publish(_, _))
        .Times(1);

    MainClass mainClass;
    mainClass.publish();
}

TEST_F(MainClassTest, VerifyExactArguments) {
    // Set expectation: publish called with exactly 42 and 100, once
    EXPECT_CALL(mockPublisher, publish(Eq(42), Eq(100)))
        .Times(1);

    MainClass mainClass;
    mainClass.publish();
}