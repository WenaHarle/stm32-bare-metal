#include <stdio.h>
#include <stdint.h> 

int main() {
uint16_t q = 0x1234;
uint16_t *p = &q;
*p >>= 4;
*p |= 0xF000;
printf("q = 0x%04X\n", q);
return 0;
}