#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 0x0C;   // 0000 1100
    uint8_t b = 0x0A;   // 0000 1010

    printf("a & b  = 0x%02X\n", a & b);            // 0x08  0000 1000
    printf("a | b  = 0x%02X\n", a | b);            // 0x0E  0000 1110
    printf("a ^ b  = 0x%02X\n", a ^ b);            // 0x06  0000 0110
    printf("~a     = 0x%02X\n", (uint8_t)~a);      // 0xF3  1111 0011
    printf("a << 1 = 0x%02X\n", (uint8_t)(a << 1)); // 0x18  0001 1000
    printf("a >> 2 = 0x%02X\n", a >> 2);           // 0x03  0000 0011
    return 0;
}
