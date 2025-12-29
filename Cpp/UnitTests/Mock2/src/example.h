#ifndef FUNCTION_H
#  define FUNCTION_H

#  include <stdint.h>

//! \brief Example function calling C function open the main.c file,
//! read X bytes on it and printf the result. Return the fd that shall
//! be closed. This function is an example of what a C project can call.
//! The algorithm is not important, this function is calling functions
//! such as open() and read() and we want to unit test this function
//! with google tests API (gmock and gtest) and mock open() and read()
// functions.
//!
//! \param[in] buffer: buffer holding data from read() finction.
//! \param[in] size: buffer size.
//! \return the file descriptor
int example(char *buffer, size_t size);

#endif
