#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    printf("uint8_t  = %zu byte\n", sizeof(uint8_t));   // 1
    printf("uint16_t = %zu bytes\n", sizeof(uint16_t)); // 2
    printf("uint32_t = %zu bytes\n", sizeof(uint32_t)); // 4

    printf("INT8_MAX   = %d\n", INT8_MAX);               // 127
    printf("UINT32_MAX = %" PRIu32 "\n", UINT32_MAX);    // 4294967295

    uint8_t x = 255;
    x++;                              // unsigned: wraps around to 0
    printf("x = %" PRIu8 "\n", x);    // 0

    uint32_t reg = 0xDEADBEEF;
    printf("reg = 0x%08" PRIX32 "\n", reg);  // 0xDEADBEEF
    return 0;
}
