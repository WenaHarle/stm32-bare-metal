#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint8_t count_ones(uint32_t x);


int main(){
    uint8_t count;
    uint32_t num = 0xF0F0F0F0; // Example number to count ones in
    count = count_ones(num);
    printf("Number of ones in 0x%X: %d\n", num, count);
    return 0;
}



uint8_t count_ones(uint32_t x) {
    uint8_t count = 0;
    while (x) {
        x &= x - 1;
        count++;
    }
    return count;
}
