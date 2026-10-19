cmake_minimum_required(VERSION 3.20)
project(blink C ASM)

set(CMAKE_C_STANDARD 11)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32f407.ld)

# Daftar file .c. Kalau nanti menambah file, tambahkan di sini.
add_executable(blink
    src/startup.c
    src/main.c
)

set_target_properties(blink PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(blink PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/blink.map
)

# Setelah build: buat file .bin dan tampilkan pemakaian Flash/RAM
add_custom_command(TARGET blink POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:blink> ${CMAKE_BINARY_DIR}/blink.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:blink>
)

# Target untuk flash ke board
add_custom_target(flash
    COMMAND st-flash --reset write ${CMAKE_BINARY_DIR}/blink.bin 0x08000000
    DEPENDS blink
)