#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t reg = 0x5A;              // 0101 1010

    reg |= (1U << 0);                // set bit 0
    printf("0x%02X\n", reg);         // 0x5B  0101 1011

    reg &= ~(1U << 3);               // clear bit 3
    printf("0x%02X\n", reg);         // 0x53  0101 0011

    reg ^= (1U << 7);                // toggle bit 7
    printf("0x%02X\n", reg);         // 0xD3  1101 0011

    if (reg & (1U << 4)) {           // check bit 4
        printf("bit 4 is set\n");    // printed
    }
    return 0;
}
