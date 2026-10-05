#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static volatile uint32_t fake_odr = 0;      // stands in for the real register

#define LED_PIN  5U

int main(void) {
    volatile uint32_t *const ODR = &fake_odr;

    *ODR |= (1U << LED_PIN);
    printf("ODR = 0x%08" PRIX32 "\n", *ODR);   // 0x00000020

    *ODR ^= (1U << LED_PIN);
    printf("ODR = 0x%08" PRIX32 "\n", *ODR);   // 0x00000000
    return 0;
}
