#include "../src/example.c"
#include "mocks.hpp"
#include <algorithm>

using namespace ::testing;

// Test the function with google test
TEST(TestExample, nominal)
{
    // First test: nominal case
    std::cout << "First test:" << std::endl;
 
        // Mocked class calling the mocked read() and open() methods
        OpenMock open_mock;
        ReadMock read_mock;

        // Check flags of open() is RD and makes this mock returns the file
        // descriptor 42.
        EXPECT_CALL(open_mock, open(_,Eq(O_RDONLY))).Times(1).WillOnce(Return(42));

        // Check we want to read 16 bytes and implement a fake read() fonction
        // returning some data, faking some reading in the file descriptor 42.
        EXPECT_CALL(read_mock, read(_,_,Eq(16u))).Times(1)
                .WillOnce(Invoke([]/*read*/(int /*fd*/, char* buf, size_t nbytes) -> int
                                 {
                                     char buffer[32u] = "This is a mocked data";
                                     size_t i = std::max(size_t(32), nbytes);
                                     while (i--) {
                                         buf[i] = buffer[i];
                                     }
                                     return nbytes;
                                 }));

        // Call real function to be tested
        char buffer[16] = {0};
        int fd = example(buffer, 16u);

        // Checks
        ASSERT_EQ(fd, 42);
        ASSERT_STREQ(buffer, "This is a mocked"); // " data" is not read
        close(fd);
}

TEST(TestExample, failure)
{
    // Second tests: failure
    std::cout << "Second test:" << std::endl;

        // Mocked class calling the mocked read() method but the real open() function
        OpenMock open_mock;
        ReadMock read_mock;

        open_mock.m_instance = nullptr;
        EXPECT_CALL(read_mock, read(_,_,_)).Times(0);

        // Call real function to be tested
        char buffer[16] = {0};
        int fd = example(buffer, 16u);

        // Checks
        ASSERT_EQ(fd, -1); // Real file descriptor ../src/main.c
        ASSERT_STREQ(buffer, "");
        close(fd);
}

// You have to compile and install google tests first:
//   https://github.com/google/googletest
// Compilation:
//   g++ -W -Wall -Wextra --std=c++11 -I../src mocks.cpp main.cpp -o tests `pkg-config gtest gmock --cflags --libs` -ldl
int main(int argc, char *argv[])
{
    // The following line must be executed to initialize Google Mock
    // (and Google Test) before running the tests.
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
