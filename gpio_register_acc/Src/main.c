#include <stdint.h>

#define PERIPH_BASE_AHB1   0x40020000U
#define GPIOA_BASE         (PERIPH_BASE_AHB1 + 0x0000U)
#define GPIOD_BASE         (PERIPH_BASE_AHB1 + 0x0C00U)
#define RCC_BASE           (PERIPH_BASE_AHB1 + 0x3800U)

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
} GPIO_Regs;

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t          RESERVED0;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t          RESERVED1[2];
    volatile uint32_t AHB1ENR;
} RCC_Regs;

#define GPIOA  ((GPIO_Regs *)GPIOA_BASE)
#define GPIOD  ((GPIO_Regs *)GPIOD_BASE)
#define RCC    ((RCC_Regs *)RCC_BASE)

#define BIT(n)  (1U << (n))

#define RCC_AHB1ENR_GPIOAEN  BIT(0)
#define RCC_AHB1ENR_GPIODEN  BIT(3)

#define BUTTON_PIN      0U
#define LED_GREEN_PIN   12U
#define LED_ORANGE_PIN  13U
#define LED_BLUE_PIN    15U

#define MODER_INPUT    0x0U
#define MODER_OUTPUT   0x1U

static inline void write_field(volatile uint32_t *reg, uint32_t mask, uint32_t pos, uint32_t val)
{
    *reg = (*reg & ~(mask << pos)) | ((val & mask) << pos);
}

static inline void gpio_set_mode(GPIO_Regs *port, uint32_t pin, uint32_t mode)
{
    write_field(&port->MODER, 0x3U, pin * 2U, mode);
}

static inline void gpio_write(GPIO_Regs *port, uint32_t pin, int high)
{
    port->BSRR = high ? BIT(pin) : BIT(pin + 16U);
}

static inline int gpio_read(const GPIO_Regs *port, uint32_t pin)
{
    return (int)((port->IDR >> pin) & 1U);
}

static inline void gpio_toggle(GPIO_Regs *port, uint32_t pin)
{
    port->ODR ^= BIT(pin);
}

static void delay(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++) { }
}

static void gpio_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIODEN;
    (void)RCC->AHB1ENR;

    gpio_set_mode(GPIOD, LED_GREEN_PIN,  MODER_OUTPUT);
    gpio_set_mode(GPIOD, LED_ORANGE_PIN, MODER_OUTPUT);
    gpio_set_mode(GPIOD, LED_BLUE_PIN,   MODER_OUTPUT);
    gpio_set_mode(GPIOA, BUTTON_PIN,     MODER_INPUT);
}

int main(void)
{
    gpio_init();

    int prev_pressed = 0;
    uint32_t blink_ticks = 0;

    for (;;)
    {
        int pressed = gpio_read(GPIOA, BUTTON_PIN);

        gpio_write(GPIOD, LED_BLUE_PIN, pressed);

        if (pressed && !prev_pressed) {
            gpio_toggle(GPIOD, LED_ORANGE_PIN);
            delay(20000U);
        }
        prev_pressed = pressed;

        if (++blink_ticks >= 100000U) {
            blink_ticks = 0;
            gpio_toggle(GPIOD, LED_GREEN_PIN);
        }
    }
}
