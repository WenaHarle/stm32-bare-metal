#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

bool is_power_of_two(uint32_t x) 
{
    // A power of two must be greater than 0 AND have only one bit set
    return (x > 0) && ((x & (x - 1)) == 0);
}

int main() {
    uint32_t num = 64; 
    
    if (is_power_of_two(num)) {
        printf("%u is a power of two.\n", num);
    } else {
        printf("%u is not a power of two.\n", num);
    }
    
    return 0;
}
