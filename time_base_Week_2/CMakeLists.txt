cmake_minimum_required(VERSION 3.20)
project(timer C ASM)

set(CMAKE_C_STANDARD 11)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32f407.ld)

# Daftar file .c. Kalau nanti menambah file, tambahkan di sini.
add_executable(timer
    src/startup.c
    src/main.c
)

set_target_properties(timer PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(timer PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_BINARY_DIR}/timer.map
)

# Setelah build: buat file .bin dan tampilkan pemakaian Flash/RAM
add_custom_command(TARGET timer POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:timer> ${CMAKE_BINARY_DIR}/timer.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:timer>
)

# Target untuk flash ke board
add_custom_target(flash
    COMMAND st-flash --reset write ${CMAKE_BINARY_DIR}/timer.bin 0x08000000
    DEPENDS timer
)