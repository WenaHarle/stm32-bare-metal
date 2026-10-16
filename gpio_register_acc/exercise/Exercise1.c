#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint8_t x = 0x3C;
    x |=  (1U << 0);
    printf("x = 0x%02X\n", x);  // 0x3D
    x &= ~(1U << 2);
    printf("x = 0x%02X\n", x);  // 0x39
    x ^=  (1U << 7);
    printf("x = 0x%02X\n", x);  // 0xBC
    return 0;
}