#include <stdio.h>
#include <stdint.h>


void set_bit(uint8_t *reg, uint8_t n) {
    *reg |= (uint8_t)(1U << n);
}

int main(void) {
    uint8_t status = 0x00;
    set_bit(&status, 3);     // status is now 0x08
    printf("status = 0x%02X\n", status);  // 0x08



    uint16_t arr[3] = {10, 20, 30};
    uint16_t *p = arr;           // an array name is the address of its first element

    printf("%u\n", *(p + 1));    // 20
    printf("%u\n", p[2]);        // 30, p[i] is the same as *(p + i)
    // p + 1 is 2 bytes away from p because sizeof(uint16_t) == 2

    uint32_t v = 0x12345678;
    uint8_t *bp = (uint8_t *)&v;

    for (int i = 0; i < 4; i++) {
        printf("%02X ", bp[i]);  // 78 56 34 12 on a little-endian machine
    }

    return 0;
}


