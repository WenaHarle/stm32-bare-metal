ENTRY(Reset_Handler)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Stack mulai dari ujung atas RAM, tumbuh ke bawah */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  /* Vector table wajib paling awal di Flash */
  .isr_vector : { KEEP(*(.isr_vector)) } > FLASH

  /* Kode program dan konstanta */
  .text : {
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .ARM.exidx : { *(.ARM.exidx*) } > FLASH

  /* Lokasi nilai awal .data di Flash */
  _sidata = LOADADDR(.data);

  /* Variabel global yang punya nilai awal: jalan di RAM, disimpan di Flash */
  .data : {
    _sdata = .;
    *(.data*)
    . = ALIGN(4);
    _edata = .;
  } > RAM AT > FLASH

  /* Variabel global tanpa nilai awal: diisi nol saat startup */
  .bss : {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
  } > RAM

  PROVIDE(end = _ebss);
}