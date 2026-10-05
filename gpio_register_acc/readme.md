# gpio_register_acc

Proyek STM32CubeIDE untuk **STM32F407VGTx** (Cortex-M4, Flash 1 MB, RAM 128 KB + CCMRAM 64 KB) yang mengakses GPIO **langsung lewat register**, tanpa HAL/LL.

## Tujuan

1. Membaca bab **RCC** dan **GPIO** di Reference Manual (RM0090).
2. Menyalakan LED lewat register (tanpa HAL).
3. Membaca tombol lewat register.
4. Berlatih C untuk bare-metal: bit manipulation (set, clear, toggle, cek bit), `volatile`, dan `stdint.h`.

## Isi Folder

| Path | Fungsi |
|------|--------|
| [Src/main.c](Src/main.c) | Program utama. Saat ini masih kerangka kosong (`for(;;);`). Di sinilah kode akses register RCC dan GPIO ditulis. |
| [Src/syscalls.c](Src/syscalls.c) | Stub syscall newlib (`_write`, `_read`, `_sbrk`, dll.) hasil generate CubeIDE agar linking dengan libc berhasil. |
| [Src/sysmem.c](Src/sysmem.c) | Implementasi `_sbrk` untuk heap (`malloc`). |
| [Inc/](Inc) | Tempat header buatan sendiri, misalnya definisi alamat register dan macro bit. Saat ini kosong. |
| [Startup/startup_stm32f407vgtx.s](Startup/startup_stm32f407vgtx.s) | Kode startup assembly: vector table, set stack pointer awal, `Reset_Handler` (menyalin `.data`, mengosongkan `.bss`), lalu lompat ke `main()`. |
| [STM32F407VGTX_FLASH.ld](STM32F407VGTX_FLASH.ld) | Linker script untuk menjalankan program dari Flash (0x08000000). Dipakai untuk build normal. |
| [STM32F407VGTX_RAM.ld](STM32F407VGTX_RAM.ld) | Linker script untuk menjalankan program dari RAM (0x20000000), berguna untuk debug. |
| [basic/](basic) | Folder untuk latihan dasar C (bit manipulation, `volatile`, `stdint.h`) sebelum menyentuh hardware. Saat ini kosong. |
| `.project`, `.cproject`, `.settings/` | Konfigurasi proyek Eclipse/STM32CubeIDE (toolchain, MCU, flag compiler). Biasanya tidak diedit manual. |

## Alur Kerja yang Direncanakan

1. **Aktifkan clock** peripheral GPIO lewat `RCC->AHB1ENR` (bit GPIOxEN). Tanpa ini register GPIO tidak berfungsi.
2. **Atur mode pin** lewat `GPIOx->MODER`: output untuk LED, input untuk tombol.
3. **Nyalakan LED** lewat `GPIOx->ODR` atau `GPIOx->BSRR` (set/reset atomik).
4. **Baca tombol** lewat `GPIOx->IDR`, dan atur pull-up/pull-down lewat `GPIOx->PUPDR` bila perlu.

## Catatan C yang Dipakai

- `volatile`: register hardware bisa berubah di luar program, jadi compiler tidak boleh mengoptimasi aksesnya.
- `stdint.h`: tipe berukuran pasti (`uint32_t`) agar sesuai dengan register 32-bit.
- Bit manipulation:
  - Set bit: `REG |= (1U << n);`
  - Clear bit: `REG &= ~(1U << n);`
  - Toggle bit: `REG ^= (1U << n);`
  - Cek bit: `if (REG & (1U << n)) { ... }`

## Build

Buka sebagai proyek STM32CubeIDE, lalu build dengan konfigurasi Debug. Gunakan linker script Flash untuk menjalankan di board.
