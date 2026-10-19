#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint16_t x = 0x1234;
    uint16_t *p = &x;        // p stores the address of x

    printf("address of x = %p\n", (void *)p);
    printf("value of x   = 0x%04X\n", *p);    // 0x1234

    *p = 0xBEEF;                               // write to x through p
    printf("x            = 0x%04X\n", x);      // 0xBEEF
    return 0;
}