# Tidak ada sistem operasi (bare-metal)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Program-program dari toolchain ARM
set(CMAKE_C_COMPILER   arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY      arm-none-eabi-objcopy)
set(CMAKE_SIZE         arm-none-eabi-size)

# Saat tes compiler, CMake jangan mencoba membuat program utuh (pasti gagal di bare-metal)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# CPU STM32F407: Cortex-M4 dengan FPU
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS_INIT "${CPU_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG_INIT "-Og -g3")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Os -g0 -DNDEBUG")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS} --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections")