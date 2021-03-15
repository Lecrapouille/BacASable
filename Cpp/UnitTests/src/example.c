#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "example.h"

// The algorithm of this function is not very important, we
// only want to know how to unit test it by mocking functions
// open() and read() with google tests.
// See ../tests/mocks.cpp and ./tests/mocks.hpp
int example(char *buffer, size_t count)
{
   int fd;
   ssize_t n;

   fd = open("main.c", O_RDONLY);
   if (fd >= 0)
   {
      n = read(fd, buffer, count);
      if (n > 0u)
      {
         buffer[n] = '\0';
         fprintf(stdout, "Hello '%s'\n", buffer);
         return fd;
      }

      fprintf(stderr, "Failed reading. Reason: %s\n", strerror(errno));
      return -1;
   }

   fprintf(stderr, "Failed opening main.c. Reason: %s\n", strerror(errno));
   return -1;
}
