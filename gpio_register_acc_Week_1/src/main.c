#include <stdint.h>
/* ---------- Alamat dasar (DS8626 Table 10) ---------- */
#define RCC_BASE      0x40023800UL
#define GPIOB_BASE    0x40020400UL

/* ---------- Register (base + offset dari RM0090) ---------- */
#define RCC_AHB1ENR   (*(volatile uint32_t *)(RCC_BASE   + 0x30))
#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_PUPDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_IDR     (*(volatile uint32_t *)(GPIOB_BASE + 0x10))
#define GPIOB_BSRR    (*(volatile uint32_t *)(GPIOB_BASE + 0x18))

/* ---------- Pin ---------- */
#define RCC_AHB1ENR_GPIOBEN   (1U << 1)    /* clock GPIOB = bit 1 */

#define LED_PIN_RED           12U          /* PB12 */
#define LED_PIN_GREEN         13U          /* PB13 */
#define LED_PIN_BLUE          14U          /* PB14 */
#define BUTTON_PIN            15U          /* PB15  [PERBAIKAN] komen & nama */

/* ---------- Nilai field 2-bit ---------- */
#define MODER_OUTPUT          1U           /* 01 */
#define PUPDR_PULLUP          1U           /* 01 */

/* ---------- Jenis LED ----------
 * 0 = LED biasa (katoda ke GND)      → menyala saat pin HIGH
 * 1 = RGB common anode (common ke 3V3) → menyala saat pin LOW   */
#define LED_ACTIVE_LOW        1

/* Mask semua LED: bit 12, 13, 14 */
#define LED_ALL   ((1U << LED_PIN_RED) | (1U << LED_PIN_GREEN) | (1U << LED_PIN_BLUE))

/* [PERBAIKAN] Satu mask BSRR untuk nyala dan untuk mati.
 * Bagian set = bit n, bagian reset = bit n + 16 (LED_ALL << 16). */
#if LED_ACTIVE_LOW
  #define LEDS_ON   (LED_ALL << 16)        /* pin LOW  → LED menyala */
  #define LEDS_OFF  (LED_ALL)              /* pin HIGH → LED mati    */
#else
  #define LEDS_ON   (LED_ALL)              /* pin HIGH → LED menyala */
  #define LEDS_OFF  (LED_ALL << 16)        /* pin LOW  → LED mati    */
#endif

int main(void)
{
    /* 1. Nyalakan clock GPIOB */
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC_AHB1ENR;

    /* 2. PB12, PB13, PB14 jadi output (kosongkan, lalu isi 01) */
    GPIOB_MODER &= ~(3U << (LED_PIN_RED   * 2));
    GPIOB_MODER &= ~(3U << (LED_PIN_GREEN * 2));
    GPIOB_MODER &= ~(3U << (LED_PIN_BLUE  * 2));

    GPIOB_MODER |=  (MODER_OUTPUT << (LED_PIN_RED   * 2));
    GPIOB_MODER |=  (MODER_OUTPUT << (LED_PIN_GREEN * 2));
    GPIOB_MODER |=  (MODER_OUTPUT << (LED_PIN_BLUE  * 2));

    /* 3. PB15 jadi input: cukup dikosongkan → 00
     * [PERBAIKAN] baris "|= (0U << ...)" dihapus karena tidak berefek */
    GPIOB_MODER &= ~(3U << (BUTTON_PIN * 2));

    /* 4. [PERBAIKAN] Pull-up untuk PB15 (bit 30-31 = 01)
     * Tanpa ini pin floating saat tombol dilepas. */
    GPIOB_PUPDR &= ~(3U << (BUTTON_PIN * 2));
    GPIOB_PUPDR |=  (PUPDR_PULLUP << (BUTTON_PIN * 2));

    /* 5. Mulai dengan semua LED mati */
    GPIOB_BSRR = LEDS_OFF;

    while (1) {
        /* [PERBAIKAN] Tombol ke GND + pull-up → active-low:
         *   dilepas  → bit 15 = 1
         *   ditekan  → bit 15 = 0                                  */
        if ((GPIOB_IDR & (1U << BUTTON_PIN)) == 0) {
            GPIOB_BSRR = LEDS_ON;     /* [PERBAIKAN] '=' dan satu kali tulis */
        } else {
            GPIOB_BSRR = LEDS_OFF;
        }
    }
}