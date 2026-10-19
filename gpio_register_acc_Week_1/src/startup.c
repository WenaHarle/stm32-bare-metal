#include <stdint.h>

/* Simbol-simbol ini dibuat oleh linker script */
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;

int main(void);
void Reset_Handler(void);

void Default_Handler(void) { while (1) {} }

/* weak: bisa ditimpa oleh fungsi bernama sama di file lain */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

typedef void (*vector_t)(void);

/* CPU membaca tabel ini dari alamat 0x08000000 saat reset */
__attribute__((section(".isr_vector"), used))
const vector_t vector_table[] = {
    (vector_t)&_estack,     /* nilai awal stack pointer */
    Reset_Handler,          /* dijalankan setelah reset */
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,             /* reserved */
    SVC_Handler,
    DebugMon_Handler,
    0,                      /* reserved */
    PendSV_Handler,
    SysTick_Handler,
    /* IRQ peripheral ditambahkan nanti di Minggu 2 */
};

void Reset_Handler(void)
{
    /* 1. Salin nilai awal .data dari Flash ke RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* 2. Isi .bss dengan nol */
    for (dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0;
    }

    /* 3. Nyalakan FPU (register CPACR) */
    *(volatile uint32_t *)0xE000ED88 |= (0xFu << 20);
    __asm volatile ("dsb\n isb");

    /* 4. Masuk ke program utama */
    main();

    while (1) {}
}