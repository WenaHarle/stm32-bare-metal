#include <stdint.h>

/* ---------- Alamat dasar (DS8626 Table 10) ---------- */
#define RCC_BASE        0x40023800UL
#define GPIOB_BASE      0x40020400UL
#define SYSCFG_BASE     0x40013800UL
#define EXTI_BASE       0x40013C00UL
#define NVIC_ISER1_ADDR 0xE000E104UL     /* ISER0 + 0x04, IRQ 32-63 */

/* ---------- Register (base + offset dari RM0090) ---------- */
#define RCC_AHB1ENR    (*(volatile uint32_t *)(RCC_BASE    + 0x30))
#define RCC_APB2ENR    (*(volatile uint32_t *)(RCC_BASE    + 0x44))

#define GPIOB_MODER    (*(volatile uint32_t *)(GPIOB_BASE  + 0x00))
#define GPIOB_PUPDR    (*(volatile uint32_t *)(GPIOB_BASE  + 0x0C))
#define GPIOB_IDR      (*(volatile uint32_t *)(GPIOB_BASE  + 0x10))
#define GPIOB_BSRR     (*(volatile uint32_t *)(GPIOB_BASE  + 0x18))

#define SYSCFG_EXTICR4 (*(volatile uint32_t *)(SYSCFG_BASE + 0x14))

#define EXTI_IMR       (*(volatile uint32_t *)(EXTI_BASE   + 0x00))
#define EXTI_RTSR      (*(volatile uint32_t *)(EXTI_BASE   + 0x08))
#define EXTI_FTSR      (*(volatile uint32_t *)(EXTI_BASE   + 0x0C))
#define EXTI_PR        (*(volatile uint32_t *)(EXTI_BASE   + 0x14))

#define NVIC_ISER1     (*(volatile uint32_t *)NVIC_ISER1_ADDR)

/* ---------- Bit clock ---------- */
#define RCC_AHB1ENR_GPIOBEN   (1U << 1)    /* clock GPIOB = bit 1 */
#define RCC_APB2ENR_SYSCFGEN  (1U << 14)   /* clock SYSCFG = bit 14 */

/* ---------- Pin ---------- */
#define LED_PIN_RED     12U                /* PB12 */
#define LED_PIN_GREEN   13U                /* PB13 */
#define LED_PIN_BLUE    14U                /* PB14 */
#define BUTTON_PIN      15U                /* PB15 -> jalur EXTI15 */

/* ---------- Nilai field ---------- */
#define MODER_OUTPUT    1U                 /* 01 */
#define PUPDR_PULLUP    1U                 /* 01 */
#define EXTICR_PORT_B   1U                 /* 0 = PA, 1 = PB, 2 = PC ... */

/* EXTI15_10 = IRQ 40 -> ISER1 bit (40 - 32) = 8.
 * Cocokkan dengan tabel vector RM0090 / startup.c kamu. */
#define EXTI15_10_IRQ_BIT  (1U << (40U - 32U))

/* ---------- Jenis LED ----------
 * 0 = LED biasa (katoda ke GND)        -> menyala saat pin HIGH
 * 1 = RGB common anode (common ke 3V3) -> menyala saat pin LOW   */
#define LED_ACTIVE_LOW  1

/* Mask semua LED: bit 12, 13, 14 */
#define LED_ALL   ((1U << LED_PIN_RED) | (1U << LED_PIN_GREEN) | (1U << LED_PIN_BLUE))

/* BSRR: bit n = set (HIGH), bit n+16 = reset (LOW) */
#if LED_ACTIVE_LOW
  #define LEDS_ON   (LED_ALL << 16)        /* pin LOW  -> LED menyala */
  #define LEDS_OFF  (LED_ALL)              /* pin HIGH -> LED mati    */
#else
  #define LEDS_ON   (LED_ALL)              /* pin HIGH -> LED menyala */
  #define LEDS_OFF  (LED_ALL << 16)        /* pin LOW  -> LED mati    */
#endif

/* Baca level PB15 lalu atur LED.
 * Tombol ke GND + pull-up: ditekan = 0, dilepas = 1.
 * Dipakai oleh main (kondisi awal) dan ISR. Karena yang dibaca selalu
 * level terakhir pin, bouncing tombol tidak membuat LED salah state. */
static void update_leds(void)
{
    if ((GPIOB_IDR & (1U << BUTTON_PIN)) == 0) {
        GPIOB_BSRR = LEDS_ON;
    } else {
        GPIOB_BSRR = LEDS_OFF;
    }
}

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

    /* 3. PB15 jadi input (00) */
    GPIOB_MODER &= ~(3U << (BUTTON_PIN * 2));

    /* 4. Pull-up untuk PB15 (bit 31:30 = 01) */
    GPIOB_PUPDR &= ~(3U << (BUTTON_PIN * 2));
    GPIOB_PUPDR |=  (PUPDR_PULLUP << (BUTTON_PIN * 2));

    /* 5. Mulai dengan semua LED mati */
    GPIOB_BSRR = LEDS_OFF;

    /* 6. Nyalakan clock SYSCFG (wajib sebelum atur EXTICR) */
    RCC_APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    (void)RCC_APB2ENR;

    /* 7. Hubungkan jalur EXTI15 ke port B
     *    EXTICR4 mengatur EXTI12-15; EXTI15 ada di bit 15:12 */
    SYSCFG_EXTICR4 &= ~(0xFU << 12);
    SYSCFG_EXTICR4 |=  (EXTICR_PORT_B << 12);

    /* 8. Picu di tepi turun (ditekan) DAN tepi naik (dilepas),
     *    buka mask, hapus pending lama (tulis 1 = hapus) */
    EXTI_FTSR |= (1U << BUTTON_PIN);
    EXTI_RTSR |= (1U << BUTTON_PIN);
    EXTI_IMR  |= (1U << BUTTON_PIN);
    EXTI_PR    = (1U << BUTTON_PIN);

    /* 9. Sinkronkan LED dengan kondisi tombol saat ini */
    update_leds();

    /* 10. Enable interrupt di NVIC paling akhir */
    NVIC_ISER1 = EXTI15_10_IRQ_BIT;

    while (1) {
        /* Semua kerja dilakukan di ISR. CPU boleh tidur sampai ada interrupt: */
        __asm volatile ("wfi");
    }
}

/* ISR untuk EXTI10..EXTI15. Nama harus sama dengan entri di vector table. */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_PR & (1U << BUTTON_PIN)) {
        EXTI_PR = (1U << BUTTON_PIN);     /* hapus flag: tulis 1, pakai '=' bukan '|=' */
        update_leds();
    }
}