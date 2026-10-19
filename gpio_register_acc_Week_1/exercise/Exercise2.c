#include <stdint.h>
#include <stdio.h>

uint8_t flags =  0xf;
void main(void)
{
    flags |= (1U << 6);
    printf("flags = %b\n", flags); 
    flags &= ~(1U << 1);
    printf("flags = %b\n", flags); 
    flags ^= (1U << 4);
    printf("flags = %b\n", flags);  

    if (flags & (1U << 2))
    {
        printf("bit 2 is set\n");
    }
    else
    {
        printf("bit 2 is not set\n");
    }
}


