#include "vector_addition_kernel.h"
#include <stdint.h>

void vector_addition_kernel(
    uint32_t* vout, 
    uint32_t* vin0,
    uint32_t* vin1
)
{
    #pragma GCC unroll 64
    for(int i = 0; i < 64; i++)
    {
        vout[i] = vin0[i] + vin1[i];
    }
}
