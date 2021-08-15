// gcc foo.c -fpic -shared -o libfoo.so

#include <stdio.h>
#include <assert.h>

struct mystruct
{
    double* data;
    size_t size;
};

typedef struct mystruct mystruct_t;

void recvstruct(mystruct_t const* st)
{
    assert(st != NULL);
    printf("mystruct=%g, %g, %g (%zu)\n",
           st->data[0], st->data[1], st->data[2], st->size);
}

void updatestruct(mystruct_t* st)
{
    static double values[16] = { 1.0, 2.0, 3.0 };

    assert(st != NULL);
    st->data = values;
    st->size = 3u;
}
