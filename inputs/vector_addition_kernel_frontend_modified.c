#include "vector_addition_kernel_frontend_modified.h"
#include <stdint.h>

void vector_addition_kernel(
    uint64_t* vout, 
    uint64_t* vin0,
    uint64_t* vin1
)
{
    #pragma GCC unroll 64
    for(int i = 0; i < 64; i++)
    {
        vout[i] = vin0[i] + vin1[i];
    }
}
