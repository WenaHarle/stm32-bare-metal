# 🛠️ Firmware Learning Roadmap: 3 Bulan

Rencana belajar firmware STM32 selama 12 minggu, **1 jam per hari**.

**Pola mingguan:** 6 hari ngoding + 1 hari review (rapikan catatan, update README, latihan menjelaskan progres dalam bahasa Inggris).

**Target akhir bulan ke-3:**
- 2 repo portofolio yang rapi (driver sensor + sensor node FreeRTOS)
- Pemahaman C untuk embedded yang cukup kuat untuk interview teknis dasar
- 1 tulisan teknis publik

---

## 📦 Persiapan

- [ ] Board Nucleo STM32 (F401RE / F411 / yang tersedia)
- [ ] Sensor I2C (BME280 atau MPU6050)
- [ ] Logic analyzer murah + PulseView
- [ ] Reference Manual (RM) dan datasheet chip STM32
- [ ] Datasheet sensor

---

## ⏱️ Rutinitas Harian (1 jam)

| Durasi | Aktivitas |
|---|---|
| 10 menit | Baca Reference Manual / datasheet |
| 40 menit | Ngoding |
| 10 menit | Tulis catatan di `docs/notes.md` |

> Kalau sedang capek, 20 menit tetap lebih baik daripada nol. Yang penting streak-nya jalan.

---

## 📅 Bulan 1: Fondasi C + Bare-Metal

### Minggu 1: Rapikan GitHub + GPIO lewat register
- [ ] Archive / private repo lama yang tidak relevan
- [ ] Update profile README dan bio (fokus firmware)
- [ ] Ganti repo yang di-pin
- [ ] Baca bab RCC dan GPIO di Reference Manual
- [ ] Nyalakan LED lewat register (tanpa HAL)
- [ ] Baca tombol lewat register
- **C:** bit manipulation (set, clear, toggle, cek bit), `volatile`, `stdint.h`
- **C (+15 menit/hari):** latihan pointer di PC

### Minggu 2: Interrupt, timer, UART
- [ ] Interrupt tombol dengan EXTI
- [ ] SysTick untuk delay presisi
- [ ] Timer untuk blink LED
- [ ] UART polling untuk debug (`printf`)
- **C:** `static`, `const`, `enum`, memisahkan kode ke `.c` / `.h`
- **C (+15 menit/hari):** latihan pointer ke struct

### Minggu 3: UART interrupt + ring buffer
- [ ] UART dengan interrupt
- [ ] Implementasi ring buffer
- [ ] Rekam sinyal UART dengan logic analyzer
- [ ] Simpan screenshot hasil capture ke `docs/`
- **C:** array, indeks melingkar, shared variable antara ISR dan main loop (kenapa `volatile` wajib)

### Minggu 4: Driver sensor I2C (mulai)
- [ ] Baca datasheet sensor
- [ ] Buat `bme280.c` / `bme280.h` terpisah dari HAL
- [ ] Fungsi I2C read/write dilewatkan sebagai function pointer
- [ ] Inisialisasi sensor + baca ID chip
- **C:** function pointer, struct sebagai "handle" driver

---

## 📅 Bulan 2: Peripheral Lanjut + Kebiasaan Profesional

### Minggu 5: Selesaikan driver sensor → **Repo #1**
- [ ] Baca data mentah + kompensasi
- [ ] Error handling (timeout, sensor tidak terdeteksi)
- [ ] Rekam transaksi I2C dengan logic analyzer
- [ ] README: tujuan, diagram koneksi, cara build, screenshot, hal yang dipelajari
- [ ] Struktur folder: `drivers/`, `app/`, `docs/`
- [ ] 🎯 **Repo #1 publish**
- **C:** error handling dengan return code dan `enum` status

### Minggu 6: DMA + SPI
- [ ] UART atau ADC dengan DMA
- [ ] Komunikasi SPI (sensor / display / loopback)
- [ ] Bandingkan beban CPU: polling vs interrupt vs DMA
- **C:** `union`, struct packing, menyusun data per byte

### Minggu 7: Debugging + build system
- [ ] Breakpoint, watch variable, step di debugger
- [ ] Sengaja picu HardFault, lalu lacak penyebabnya
- [ ] Build proyek dengan CMake atau Makefile di luar IDE
- **C:** proses compile & link, `extern`, dasar linker script dan memory map

### Minggu 8: Unit test + CI
- [ ] Setup Unity atau Ceedling
- [ ] Unit test driver sensor di PC (dengan mock I2C)
- [ ] GitHub Actions: build + test otomatis
- [ ] Tambahkan badge CI di README Repo #1
- **C:** menulis kode yang testable, memisahkan logika dari hardware

---

## 📅 Bulan 3: RTOS + Proyek Integrasi + Persiapan C++

### Minggu 9: Dasar FreeRTOS
- [ ] Setup FreeRTOS di STM32
- [ ] Buat beberapa task dengan prioritas berbeda
- [ ] `vTaskDelay` vs `vTaskDelayUntil`
- [ ] Pantau stack usage tiap task

### Minggu 10: Sinkronisasi
- [ ] Queue antar task
- [ ] Semaphore (termasuk dari ISR)
- [ ] Mutex untuk resource bersama (UART)
- [ ] Sengaja buat race condition, lalu perbaiki
- [ ] Tulis catatan tentang priority inversion

### Minggu 11: Proyek sensor node
- [ ] Task sensor (baca periodik)
- [ ] Task logging via UART
- [ ] Tombol untuk ganti mode
- [ ] Low-power mode saat idle
- [ ] Ukur dan dokumentasikan konsumsi arus

### Minggu 12: Finalisasi → **Repo #2**
- [ ] README lengkap + diagram arsitektur task
- [ ] 🎯 **Repo #2 publish**
- [ ] Tulis 1 artikel / post LinkedIn tentang proyek ini
- [ ] **C++ (pengenalan, 2–3 sesi):** class vs struct + function pointer, konsep RAII
- [ ] Evaluasi dan pilih jalur berikutnya

---

## 🧭 Setelah Bulan ke-3

Pilih satu jalur spesialisasi:

| Jalur | Materi berikutnya |
|---|---|
| IoT | Zephyr RTOS + BLE, OTA update |
| Automotive | CAN bus, AUTOSAR dasar, C++ embedded |
| Embedded Linux | Yocto, device driver, bootloader |

---

## 📏 Aturan Main

1. **Boleh maju lebih cepat** kalau bisa menjelaskan materi tanpa catatan dan menulis ulang kodenya dari nol.
2. **Kalau molor, geser jadwalnya.** Jangan skip materi demi mengejar target.
3. **AI sebagai guru, bukan penulis kode.** Jangan commit kode yang tidak bisa kamu jelaskan baris per baris.
4. **Tidak ada kode kantor** di repo pribadi.
5. **Commit message yang jelas**, misalnya `Add I2C read with timeout`, bukan `update`.

---

## 📝 Progress Log

| Minggu | Tanggal | Selesai | Kendala / Catatan |
|---|---|---|---|
| 1 | | ⬜ | |
| 2 | | ⬜ | |
| 3 | | ⬜ | |
| 4 | | ⬜ | |
| 5 | | ⬜ | |
| 6 | | ⬜ | |
| 7 | | ⬜ | |
| 8 | | ⬜ | |
| 9 | | ⬜ | |
| 10 | | ⬜ | |
| 11 | | ⬜ | |
| 12 | | ⬜ | |

---

## 📚 Referensi

- *Making Embedded Systems*, Elecia White
- *Effective C*, Robert C. Seacord
- STM32 Reference Manual & datasheet sesuai chip
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/)
