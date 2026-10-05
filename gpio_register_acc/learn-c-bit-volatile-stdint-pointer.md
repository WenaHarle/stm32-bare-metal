# Learning C: Bit Manipulation, volatile, stdint.h, and Pointers

## Progress

Check off an item by changing `[ ]` to `[x]`, then commit.

**Topics**

- [ ] 1. stdint.h
- [ ] 2. Basic bitwise operators
- [ ] 3. Set, clear, toggle, check
- [ ] 4. Pointers
- [ ] 5. volatile
- [ ] 6. Putting it together: hardware registers
- [ ] 7. Common mistakes and pitfalls

**Exercises**

- [ ] Exercise 1 (basic): trace set, clear, toggle
- [ ] Exercise 2 (basic): four one-line operations
- [ ] Exercise 3 (basic): `uint8_t` overflow
- [ ] Exercise 4 (basic): bit operations through a pointer
- [ ] Exercise 5 (basic): choosing a `stdint.h` type
- [ ] Exercise 6 (intermediate): `count_ones`
- [ ] Exercise 7 (intermediate): `is_power_of_two`
- [ ] Exercise 8 (intermediate): `swap_nibbles`
- [ ] Exercise 9 (intermediate): find three bugs
- [ ] Exercise 10 (intermediate): when `volatile` is needed
- [ ] Exercise 11 (intermediate): bytes and endianness
- [ ] Exercise 12 (advanced): `read_field` and `write_field`
- [ ] Exercise 13 (advanced): configuring the `CTRL` register
- [ ] Exercise 14 (advanced): `swap_endian32`
- [ ] Exercise 15 (advanced): the `volatile` concept

**Study notes**

| Date | What I learned | What is still confusing |
| --- | --- | --- |
|  |  |  |

## How to use this material

These four topics are closely related: all of them are used together when writing C for embedded and low-level work. Study them in order, because each section builds on the previous one.

1. `stdint.h`: data types with a guaranteed bit width.
2. Bitwise operators and the four core operations: set, clear, toggle, check.
3. Pointers: accessing data through its address.
4. `volatile`: forbidding the compiler from optimizing away accesses to a variable.
5. All four combined: accessing hardware registers.

Prerequisites: you already understand variables, `if`, loops, and functions in C. Every example can be run on a PC with:

```bash
gcc -std=c11 -Wall -Wextra -O2 example.c -o example && ./example
```

Retype the examples instead of copy-pasting. Do the exercises on paper first, then check them with the compiler and the answer key at the end.

## 1. stdint.h: fixed-width types

The size of `int` and `long` differs between platforms, so code that depends on the number of bits must use the types from `<stdint.h>`. For example, `int` can be 16 bits on an 8-bit microcontroller and 32 bits on a PC.

| Type | Width | Value range |
| --- | --- | --- |
| `uint8_t` | 8 bits | 0 to 255 |
| `int8_t` | 8 bits | -128 to 127 |
| `uint16_t` | 16 bits | 0 to 65,535 |
| `int16_t` | 16 bits | -32,768 to 32,767 |
| `uint32_t` | 32 bits | 0 to 4,294,967,295 |
| `int32_t` | 32 bits | -2,147,483,648 to 2,147,483,647 |
| `uint64_t` | 64 bits | 0 to 18,446,744,073,709,551,615 |

Rules of thumb:

- For bit manipulation, always use **unsigned** types (`uint8_t`, `uint16_t`, `uint32_t`). Shifting a signed type can produce undefined behavior.
- `uintptr_t` is an unsigned integer wide enough to hold an address.
- Limits are available as constants: `UINT8_MAX`, `INT8_MAX`, `UINT32_MAX`, and so on.
- For constants of a specific width use `UINT32_C(1)` or the `U` suffix, for example `1U`.
- For `printf`, use the macros from `<inttypes.h>`: `PRIu8`, `PRIu32`, `PRIX32`, and similar.

### Example 1.1: sizes, limits, and overflow

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    printf("uint8_t  = %zu byte\n", sizeof(uint8_t));   // 1
    printf("uint16_t = %zu bytes\n", sizeof(uint16_t)); // 2
    printf("uint32_t = %zu bytes\n", sizeof(uint32_t)); // 4

    printf("INT8_MAX   = %d\n", INT8_MAX);               // 127
    printf("UINT32_MAX = %" PRIu32 "\n", UINT32_MAX);    // 4294967295

    uint8_t x = 255;
    x++;                              // unsigned: wraps around to 0
    printf("x = %" PRIu8 "\n", x);    // 0

    uint32_t reg = 0xDEADBEEF;
    printf("reg = 0x%08" PRIX32 "\n", reg);  // 0xDEADBEEF
    return 0;
}
```

Overflow on an unsigned type is well defined: the value wraps around (modulo 2^n). Overflow on a signed type is *undefined behavior* and must be avoided.

## 2. Basic bitwise operators

C has six operators that work bit by bit, and every bit manipulation trick is built from these six. Bits are counted from the right starting at 0: bit 0 is the LSB (rightmost), and bit 7 is the MSB of a `uint8_t`.

| Operator | Name | Per-bit rule |
| --- | --- | --- |
| `&` | AND | result is 1 only if both bits are 1 |
| `\|` | OR | result is 1 if either bit is 1 |
| `^` | XOR | result is 1 if the bits differ |
| `~` | NOT | flips every bit |
| `<<` | left shift | shift left, fill the right with 0 |
| `>>` | right shift | shift right, fill the left with 0 (for unsigned) |

Truth table:

```text
 A  B  |  A & B   A | B   A ^ B
 0  0  |    0       0       0
 0  1  |    0       1       1
 1  0  |    0       1       1
 1  1  |    1       1       0
```

The easiest way to remember them is by what each one *does to a bit* when the other operand is a fixed value:

| You want to... | Use | Because |
| --- | --- | --- |
| force a bit to 1 | `\| 1` | anything OR 1 is 1 |
| force a bit to 0 | `& 0` | anything AND 0 is 0 |
| flip a bit | `^ 1` | 0 XOR 1 is 1, 1 XOR 1 is 0 |
| leave a bit alone | `\| 0`, `& 1`, or `^ 0` | the bit passes through unchanged |

So the second operand acts like a **stencil**: positions holding the "force" value are affected, and positions holding the "leave alone" value are untouched. That stencil is called a mask, and everything in section 3 is just choosing the right mask.

How bit positions map to a number (here `0x5A`):

```text
bit position:   7   6   5   4   3   2   1   0
bit value:      0   1   0   1   1   0   1   0     = 0x5A
weight:       128  64  32  16   8   4   2   1     (64 + 16 + 8 + 2 = 90 = 0x5A)
```

Each hexadecimal digit is exactly 4 bits, so `0x5A` splits as `5` = `0101` and `A` = `1010`. This is why hex is used for registers: you can convert to binary one digit at a time.

### Example 2.1: all six operators on two numbers

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t a = 0x0C;   // 0000 1100
    uint8_t b = 0x0A;   // 0000 1010

    printf("a & b  = 0x%02X\n", a & b);            // 0x08  0000 1000
    printf("a | b  = 0x%02X\n", a | b);            // 0x0E  0000 1110
    printf("a ^ b  = 0x%02X\n", a ^ b);            // 0x06  0000 0110
    printf("~a     = 0x%02X\n", (uint8_t)~a);      // 0xF3  1111 0011
    printf("a << 1 = 0x%02X\n", (uint8_t)(a << 1)); // 0x18  0001 1000
    printf("a >> 2 = 0x%02X\n", a >> 2);           // 0x03  0000 0011
    return 0;
}
```

Three things to remember:

- `&` and `&&` are different. `&` works per bit, while `&&` is a true/false logical operator. The same applies to `|` and `||`.
- Shifting left by 1 position is the same as multiplying by 2, and shifting right by 1 position is the same as dividing by 2 (for unsigned).
- `1U << n` produces a number with only bit n set to 1. A number like this is called a **mask**.

## 3. The four core operations: set, clear, toggle, check

These four operations change or read a single bit without disturbing the others, and the pattern is always the same: build a mask with `1U << n`, then combine it using the right operator.

```c
reg |=  (1U << n);          // SET    : bit n becomes 1
reg &= ~(1U << n);          // CLEAR  : bit n becomes 0
reg ^=  (1U << n);          // TOGGLE : bit n is flipped
if (reg & (1U << n)) { }    // CHECK  : true if bit n is 1
```

Why each formula works:

- **Set** uses OR. `x OR 1` is always 1, `x OR 0` stays `x`, so only the target bit changes.
- **Clear** uses AND with an inverted mask. `~(1U << n)` is 1 at every position except bit n. `x AND 0` is always 0, `x AND 1` stays `x`.
- **Toggle** uses XOR. `x XOR 1` flips `x`, `x XOR 0` stays `x`.
- **Check** uses AND. The result is zero if bit n is 0, and nonzero if bit n is 1.

Here is each one worked out in binary, with `reg = 0101 1010` (`0x5A`) and `n = 3`. First, the mask:

```text
1U          = 0000 0001
1U << 3     = 0000 1000      <- only bit 3 is 1: this is the mask
```

**Set bit 3** (`reg |= mask`). The mask has a 1 only at bit 3, so only bit 3 is forced to 1:

```text
reg         = 0101 1010
mask        = 0000 1000
reg | mask  = 0101 1010      <- bit 3 was already 1, so nothing visibly changes
```

Try it with bit 2 instead: `reg | (1U << 2)` gives `0101 1110`, and bit 2 turned on.

**Clear bit 3** (`reg &= ~mask`). The `~` flips the mask so it is 0 only at bit 3. AND with 0 forces that bit to 0, and AND with 1 keeps every other bit:

```text
mask        = 0000 1000
~mask       = 1111 0111      <- 0 only at bit 3
reg         = 0101 1010
reg & ~mask = 0101 0010      <- bit 3 is now 0, the rest are unchanged
```

**Toggle bit 3** (`reg ^= mask`). XOR with 1 flips, XOR with 0 keeps:

```text
reg         = 0101 1010
mask        = 0000 1000
reg ^ mask  = 0101 0010      <- bit 3 flipped from 1 to 0
```

**Check bit 3** (`reg & mask`). AND keeps only bit 3 and zeroes everything else, so the whole result is either 0 (bit was 0) or nonzero (bit was 1):

```text
reg         = 0101 1010
mask        = 0000 1000
reg & mask  = 0000 1000      <- nonzero, so bit 3 is 1  (value is 8, not 1!)
```

A C `if` treats any nonzero value as true, which is why `if (reg & mask)` works without comparing to 1.

### Example 3.1: step-by-step trace

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t reg = 0x5A;              // 0101 1010

    reg |= (1U << 0);                // set bit 0
    printf("0x%02X\n", reg);         // 0x5B  0101 1011

    reg &= ~(1U << 3);               // clear bit 3
    printf("0x%02X\n", reg);         // 0x53  0101 0011

    reg ^= (1U << 7);                // toggle bit 7
    printf("0x%02X\n", reg);         // 0xD3  1101 0011

    if (reg & (1U << 4)) {           // check bit 4
        printf("bit 4 is set\n");    // printed
    }
    return 0;
}
```

### Example 3.2: commonly used macros

```c
#define BIT(n)              (1U << (n))
#define SET_BIT(reg, n)     ((reg) |=  BIT(n))
#define CLEAR_BIT(reg, n)   ((reg) &= ~BIT(n))
#define TOGGLE_BIT(reg, n)  ((reg) ^=  BIT(n))
#define CHECK_BIT(reg, n)   (((reg) >> (n)) & 1U)   // result is exactly 0 or 1
```

Every macro parameter is wrapped in parentheses so it stays safe when the argument is an expression, for example `SET_BIT(x, a + 1)`.

### Several bits at once

Combine several masks with OR to change many bits in one operation.

```c
reg |=  (BIT(1) | BIT(4));     // set bits 1 and 4
reg &= ~(BIT(0) | BIT(7));     // clear bits 0 and 7
reg &= 0xF0;                   // clear the lower 4 bits, keep the upper 4
```

### Bit fields: reading and writing a group of bits

Hardware registers often store multi-bit values, for example 4 bits at positions 8 to 11. The pattern: shift then mask to read, clear then set to write.

```c
uint32_t reg = 0xABCD1234;

// READ a 4-bit field starting at bit 8
uint32_t val = (reg >> 8) & 0xFU;          // val = 0x2

// WRITE the value 0x7 into the same field
reg = (reg & ~(0xFU << 8)) | (0x7U << 8);  // reg = 0xABCD1734
```

The mask for a field `w` bits wide is `(1U << w) - 1`: for `w = 4` the result is `0xF`, for `w = 3` it is `0x7`.

Why does `(1U << w) - 1` work? `1U << 4` is `1 0000` (16), and subtracting 1 turns that into `0 1111` (15): a run of exactly `w` ones.

Here is the read and the write worked out in binary. `reg = 0xABCD1234`; only the low 16 bits matter, which are `0001 0010 0011 0100`. The field is 4 bits wide at position 8, meaning bits 11 down to 8:

```text
bit:        15 14 13 12 | 11 10  9  8 |  7  6  5  4 |  3  2  1  0
value:       0  0  0  1 |  0  0  1  0 |  0  0  1  1 |  0  1  0  0
                          \_________/
                          the field = 0010 = 0x2
```

**Read:** shift right by 8 so the field lands at bits 3..0, then mask off everything above it.

```text
reg >> 8        = ... 1100 1101 0001 0010      (the field 0010 is now the lowest 4 bits)
0xF             =                    1111
(reg >> 8) & 0xF=                    0010      = 0x2
```

**Write 0x7:** two steps. First clear the old field, then put the new value in.

```text
0xFU << 8       = 0000 0000 0000 0000 0000 1111 0000 0000   (mask over the field)
~(0xFU << 8)    = 1111 1111 1111 1111 1111 0000 1111 1111   (0 over the field)
reg & ~(...)    = 0xABCD1034                                (field cleared to 0000)
0x7U << 8       = 0x00000700
 ... | 0x700    = 0xABCD1734                                (field is now 0111)
```

Always clear first, then set. If you only OR the new value in, leftover old bits corrupt the result (OR can add bits but never remove them).

## 4. Pointers

A pointer is a variable whose contents are a memory address, not data. Two key operators: `&x` gives the address of `x`, and `*p` accesses the data at the address stored in `p` (dereference).

Think of memory as a long street of numbered mailboxes. A normal variable is the *letter inside* a mailbox. A pointer is a *note with a mailbox number written on it*. `&x` means "write down the mailbox number of x", and `*p` means "go to the mailbox number written on p and open it".

```text
 address     contents
 0x2000      0x1234     <- variable x  (uint16_t)
 0x2002      0x2000     <- variable p  (a pointer; it holds the ADDRESS of x)

 p  is 0x2000      (the note)
 *p is 0x1234      (what is in mailbox 0x2000, i.e. x itself)
```

Because `*p` *is* `x`, reading or writing `*p` reads or writes `x`.

### Example 4.1: address and dereference

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint16_t x = 0x1234;
    uint16_t *p = &x;        // p stores the address of x

    printf("address of x = %p\n", (void *)p);
    printf("value of x   = 0x%04X\n", *p);    // 0x1234

    *p = 0xBEEF;                               // write to x through p
    printf("x            = 0x%04X\n", x);      // 0xBEEF
    return 0;
}
```

The `*` symbol has two meanings. In a declaration (`uint16_t *p`) it means "p is a pointer". In an expression (`*p = 5`) it means "the data p points to".

### Example 4.2: a pointer as a function parameter

C passes arguments as copies, so a function can only modify the caller's variable if it receives its address.

```c
void set_bit(uint8_t *reg, uint8_t n) {
    *reg |= (uint8_t)(1U << n);
}

int main(void) {
    uint8_t status = 0x00;
    set_bit(&status, 3);     // status is now 0x08
    return 0;
}
```

### Example 4.3: pointer arithmetic and arrays

`p + 1` advances by one **element**, not one byte. The distance in bytes depends on the type being pointed to.

```c
uint16_t arr[3] = {10, 20, 30};
uint16_t *p = arr;           // an array name is the address of its first element

printf("%u\n", *(p + 1));    // 20
printf("%u\n", p[2]);        // 30, p[i] is the same as *(p + i)
// p + 1 is 2 bytes away from p because sizeof(uint16_t) == 2
```

### Example 4.4: looking at the bytes of an integer

A pointer to `uint8_t` may be used to read any object byte by byte.

```c
uint32_t v = 0x12345678;
uint8_t *bp = (uint8_t *)&v;

for (int i = 0; i < 4; i++) {
    printf("%02X ", bp[i]);  // 78 56 34 12 on a little-endian machine
}
```

PCs (x86, and ARM in most configurations) use little-endian: the lowest byte is stored at the lowest address.

A 32-bit value `0x12345678` is four bytes: `12`, `34`, `56`, `78`, where `12` is the most significant. Memory stores them like this:

```text
 address:        +0   +1   +2   +3
 little-endian:  78   56   34   12     <- least significant byte first (PC, STM32)
 big-endian:     12   34   56   78     <- most significant byte first (written the way humans read it)
```

So `bp[0]` is the *lowest* byte (`0x78`), not the first digit you see in the hex literal. Endianness only matters when you look at a multi-byte value byte by byte (as here) or exchange data with something that uses the other order, such as a network protocol.

### const and pointers

There are two separate things that can be locked: **the data being pointed to**, and **the pointer itself**. Read the declaration from right to left, saying "is" for each word, and the meaning falls out.

```c
const uint8_t *p1;         // p1 is a pointer to a uint8_t that is const
uint8_t *const p2 = &x;    // p2 is a const pointer to a uint8_t
const uint8_t *const p3 = &x;  // p3 is a const pointer to a const uint8_t
```

What that means in practice:

```c
const uint8_t *p1 = &x;
*p1 = 5;       // ERROR: the data is locked
p1 = &y;       // OK: the pointer itself may move

uint8_t *const p2 = &x;
*p2 = 5;       // OK: the data is writable
p2 = &y;       // ERROR: the pointer is locked to x
```

A quick rule: whatever is on the **left of the `*`** locks the data, and whatever is on the **right of the `*`** locks the pointer.

A pointer that has not been given an address holds a random value. Initialize it with a valid address or `NULL`, and never dereference a `NULL` pointer.

## 5. volatile

`volatile` tells the compiler that a variable's value can change outside the visible program flow, so every read and write in the source code must actually be performed on memory. Without `volatile`, the compiler may keep the value in a CPU register, remove repeated reads, or discard writes it considers useless.

An analogy: you are checking whether a package has arrived. A careful compiler assumes "nobody else touches my variables", so it checks the doorstep once, sees nothing, and decides "no package, and nothing in this loop can change that", then stops looking. `volatile` says "someone else (hardware, an interrupt) can drop a package at any moment, so look every single time".

Concretely, the compiler transforms code like this when optimizing:

```c
// What you wrote:                    // What the optimizer may turn it into:
while (!flag) { }                     if (!flag) { for (;;) { } }   // read once, then spin forever

*REG = 1;                             *REG = 2;                     // first write dropped,
*REG = 2;                                                           // "it was overwritten anyway"
```

The second case is just as harmful for hardware: writing `1` then `2` to a register may be two distinct commands to a peripheral, and dropping the first one changes behavior. With `volatile`, both writes are kept, in order.

Three situations that require `volatile`:

1. **Hardware registers** (memory-mapped I/O): their values are changed by the device, not by the program.
2. **Variables shared between an ISR (interrupt) and the main program.**
3. **Variables modified by a signal handler** (on a PC, of type `volatile sig_atomic_t`).

### Example 5.1: the classic interrupt flag bug

```c
#include <stdint.h>
#include <stdbool.h>

bool data_ready = false;          // WRONG: not volatile

void UART_IRQHandler(void) {      // called by hardware, not by main
    data_ready = true;
}

int main(void) {
    while (!data_ready) {
        // waiting
    }
    // process data
}
```

The compiler sees that nothing inside the loop changes `data_ready`. With `-O2` optimization, it may read the variable once and turn the loop into `while (true)`. The program hangs forever even though the interrupt has already happened.

The fix is a single word:

```c
volatile bool data_ready = false;   // CORRECT: re-read from memory on every iteration
```

### Example 5.2: the disappearing delay loop

```c
void delay(void) {
    for (uint32_t i = 0; i < 100000; i++) { }           // the compiler may delete this entirely
}

void delay_ok(void) {
    for (volatile uint32_t i = 0; i < 100000; i++) { }  // still executed
}
```

### Position of volatile on pointers

Just like `const`, the position of `volatile` decides what is volatile.

```c
volatile uint32_t *p;         // the pointed-to DATA is volatile (most common)
uint32_t *volatile q;         // the POINTER itself is volatile (rare)
volatile uint32_t *const r = (volatile uint32_t *)0x40020014;  // volatile data, fixed address
```

### What volatile does not do

- **It does not make operations atomic.** `counter++` on a volatile variable is still a read, an add, and a write. An interrupt can arrive in the middle and cause lost updates.
- **It does not replace a mutex or atomics** for multi-threaded code. Use `<stdatomic.h>` or a locking mechanism.
- **It is not the same as `const`.** The two can be combined: `const volatile uint32_t *` suits a status register that the program may only read but whose value is changed by hardware.

## 6. Putting it together: accessing hardware registers

This single line uses all four topics at once, and is the most common pattern in embedded code:

```c
#define GPIOA_ODR  (*(volatile uint32_t *)0x40020014U)
```

How to read it from the inside out:

1. `0x40020014U` is the register address according to the datasheet (for example: the GPIOA output data register on an STM32F4). To the compiler it is just a plain number.
2. `(volatile uint32_t *)` is a **cast**: it says "treat this number as a pointer to volatile 32-bit data". Nothing is converted; you are telling the compiler how to interpret the number.
3. The leading `*` dereferences the pointer, so `GPIOA_ODR` can be used like an ordinary variable.

Built up in stages, it is the same as writing:

```c
volatile uint32_t *p = (volatile uint32_t *)0x40020014U;  // p holds the address
*p |= (1U << 5);                                          // go to that address, change bit 5
```

The `#define` just glues the `*` and the cast into one name, so you do not have to write `*p` every time. After macro expansion, `GPIOA_ODR |= (1U << 5);` becomes `(*(volatile uint32_t *)0x40020014U) |= (1U << 5);`. The macro name has no storage of its own: every use means "read or write the 4 bytes at this fixed address".

After that, the bit operations from section 3 apply directly:

```c
GPIOA_ODR |=  (1U << 5);          // turn on the LED on pin 5
GPIOA_ODR &= ~(1U << 5);          // turn it off
GPIOA_ODR ^=  (1U << 5);          // flip its state
if (GPIOA_ODR & (1U << 5)) { }    // check whether it is on
```

The address above is valid only on the matching microcontroller. Running it on a PC will crash (segmentation fault).

### Example 6.1: a simulated version that runs on a PC

Replace the hardware address with the address of an ordinary variable. The logic is identical.

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static volatile uint32_t fake_odr = 0;      // stands in for the real register

#define LED_PIN  5U

int main(void) {
    volatile uint32_t *const ODR = &fake_odr;

    *ODR |= (1U << LED_PIN);
    printf("ODR = 0x%08" PRIX32 "\n", *ODR);   // 0x00000020

    *ODR ^= (1U << LED_PIN);
    printf("ODR = 0x%08" PRIX32 "\n", *ODR);   // 0x00000000
    return 0;
}
```

### Example 6.2: a group of registers with a struct

The registers of a peripheral usually sit consecutively in memory, so they can be mapped with a struct. This is how vendor header files (for example CMSIS) define them.

```c
typedef struct {
    volatile uint32_t MODER;    // offset 0x00
    volatile uint32_t OTYPER;   // offset 0x04
    volatile uint32_t OSPEEDR;  // offset 0x08
    volatile uint32_t PUPDR;    // offset 0x0C
    volatile uint32_t IDR;      // offset 0x10
    volatile uint32_t ODR;      // offset 0x14
} GPIO_TypeDef;

#define GPIOA  ((GPIO_TypeDef *)0x40020000U)

// Pin 5 as output: the 2-bit field at positions 10..11 is set to 01
GPIOA->MODER = (GPIOA->MODER & ~(0x3U << (5 * 2))) | (0x1U << (5 * 2));
GPIOA->ODR  |= (1U << 5);
```

`GPIOA->ODR` means the same as `(*GPIOA).ODR`. Because each `uint32_t` member is 4 bytes, `ODR` automatically lands at address `0x40020014`.

## 7. Common mistakes and pitfalls

These seven mistakes appear most often, and most of them produce no compile error.

**1. Writing `=` when `|=` was meant.** All the other bits get erased.

```c
reg = (1U << 3);     // WRONG: reg becomes 0x08, other bits are lost
reg |= (1U << 3);    // CORRECT
```

**2. Forgetting `~` when clearing.** `reg &= (1U << 3)` actually clears all bits *except* bit 3.

**3. Operator precedence.** `==` and `!=` bind tighter than `&`, so always add parentheses.

```c
if (reg & MASK == 0)      // WRONG: parsed as reg & (MASK == 0)
if ((reg & MASK) == 0)    // CORRECT
```

**4. Comparing the check result with 1.** `reg & (1U << 4)` is `0x10` when the bit is set, not 1.

```c
if ((reg & (1U << 4)) == 1)    // WRONG: never true
if (reg & (1U << 4))           // CORRECT
if ((reg >> 4) & 1U)           // CORRECT, the result is exactly 0 or 1
```

**5. Shifting a signed type or by more than the type's width.** `1 << 31` is undefined behavior because `1` is an `int`. Use `1U << 31`. For 64 bits use `1ULL << 40`. Shifting by the type's width or more (`x << 32` on a 32-bit type) is also undefined.

**6. Integer promotion.** C never does arithmetic or bitwise math directly on a `uint8_t` or `uint16_t`. Before the operation, the value is first widened to an `int` (32 bits on most platforms). The result is also an `int`, and it only shrinks back if you store it in a small variable or cast it.

```c
uint8_t m = 0x0F;
if (~m == 0xF0)            // WRONG: ~m is computed as an int, giving 0xFFFFFFF0
if ((uint8_t)~m == 0xF0)   // CORRECT
```

Step by step for the wrong version:

```text
m            = 0x0F                        (uint8_t)
promoted     = 0x0000000F                  (int, widened first)
~m           = 0xFFFFFFF0                  (all 32 bits flipped, not just 8)
0xFFFFFFF0 == 0xF0 ?  no, so the comparison is false
```

Casting to `uint8_t` first throws away the upper 24 bits, leaving `0xF0`. The same effect explains Exercise 3: `a + b` is computed as an `int` (300) and only wraps to 44 when stored back into a `uint8_t`.

**7. Macros without parentheses.**

```c
#define BIT(n) 1U << n          // WRONG: BIT(2) + 1 becomes 1U << (2 + 1)
#define BIT(n) (1U << (n))      // CORRECT
```

Two more mistakes from earlier sections: dereferencing an uninitialized pointer, and forgetting `volatile` on a variable changed by an ISR or hardware. `volatile` bugs often only show up once optimization is turned on, which is why a program can "work in debug but hang in release".

## 8. Exercises

There are 15 exercises at three levels. Do the "guess the output" exercises on paper first by writing out the binary, then run them in the compiler.

### Basic level

**Exercise 1.** What is the final value of `x` in hexadecimal?

```c
uint8_t x = 0x3C;
x |=  (1U << 0);
x &= ~(1U << 2);
x ^=  (1U << 7);
```

**Exercise 2.** Given `uint8_t flags;`, write one line of code for each: (a) set bit 6, (b) clear bit 1, (c) toggle bit 4, (d) an `if` condition that is true when bit 2 is 1.

**Exercise 3.** What does this program output, and why?

```c
uint8_t a = 200, b = 100;
uint8_t c = a + b;
printf("%u\n", c);
```

**Exercise 4.** What is the final value of `q`?

```c
uint16_t q = 0x1234;
uint16_t *p = &q;
*p >>= 4;
*p |= 0xF000;
```

**Exercise 5.** Choose the most suitable `stdint.h` type for: (a) humidity from 0 to 100 percent, (b) temperature from -40 to 125 degrees, (c) the contents of a 32-bit register, (d) a memory address stored as an integer.

### Intermediate level

**Exercise 6.** Write a function `uint8_t count_ones(uint32_t x)` that returns the number of bits set to 1. Example: `count_ones(0xF0F0)` gives 8.

**Exercise 7.** Write a function `bool is_power_of_two(uint32_t x)` without a loop. Example: 64 is true, 6 is false, 0 is false.

**Exercise 8.** Write a function `uint8_t swap_nibbles(uint8_t x)` that swaps the upper 4 bits with the lower 4 bits. Example: `0xA5` becomes `0x5A`.

**Exercise 9.** Find the three bugs in this snippet and fix them.

```c
#define BIT(n) 1 << n

uint8_t status = read_status();

if (status & BIT(3) == 0) {
    // run if bit 3 is 0
}

status = BIT(5);   // intended: set bit 5, leave the other bits unchanged
```

**Exercise 10.** Which of these need `volatile`, and why?

- (a) `uint32_t tick_count;` that is incremented in a timer ISR and read in `main`.
- (b) A local variable `sum` used to add up the contents of an array.
- (c) A pointer to the ADC data register.
- (d) A constant table `const uint8_t sine_table[256]`.

**Exercise 11.** On a little-endian machine, what does this program output?

```c
uint32_t v = 0xAABBCCDD;
uint8_t *bp = (uint8_t *)&v;
printf("%02X %02X\n", bp[0], *(bp + 3));
```

### Advanced level

**Exercise 12.** Write two functions for bit fields:

```c
uint32_t read_field(uint32_t reg, uint8_t pos, uint8_t width);
void write_field(uint32_t *reg, uint8_t pos, uint8_t width, uint32_t val);
```

`write_field` must not change bits outside the field, and must ignore bits of `val` that exceed `width`.

**Exercise 13.** A 32-bit `CTRL` register has this layout: bit 0 = `EN`, bits 1 to 3 = `MODE`, bits 8 to 15 = `DIV`. Starting from `CTRL = 0`, enable `EN`, set `MODE = 5`, and set `DIV = 0x4F`. Write the code and compute the final value of `CTRL`.

**Exercise 14.** Write a function `uint32_t swap_endian32(uint32_t x)` that reverses the byte order. Example: `0x12345678` becomes `0x78563412`.

**Exercise 15.** Answer two conceptual questions:

- (a) `volatile uint32_t ticks;` is incremented by an ISR with `ticks++`, and `main` also runs `ticks++`. Why is this still unsafe even though it is `volatile`, and how do you fix it?
- (b) Explain the meaning of each word in the declaration `const volatile uint32_t *const STATUS = (const volatile uint32_t *)0x40001000U;`

## 9. Answer key and explanations

All code answers below were compiled and run with `gcc -std=c11 -Wall -Wextra -O2`.

### Basic level

**Answer 1.** `0xB9`.

```text
start           0011 1100   0x3C
set bit 0       0011 1101   0x3D
clear bit 2     0011 1001   0x39
toggle bit 7    1011 1001   0xB9
```

**Answer 2.**

```c
flags |=  (1U << 6);          // (a)
flags &= ~(1U << 1);          // (b)
flags ^=  (1U << 4);          // (c)
if (flags & (1U << 2)) { }    // (d)
```

**Answer 3.** The output is `44`. The addition `a + b` is computed as an `int` and gives 300. When stored into a `uint8_t`, only the lowest 8 bits remain: 300 - 256 = 44.

**Answer 4.** `0xF123`. Shifting right by 4 turns `0x1234` into `0x0123`, then OR with `0xF000` fills the top 4 bits.

**Answer 5.** (a) `uint8_t`, (b) `int8_t` because the range -128 to 127 is enough, (c) `uint32_t`, (d) `uintptr_t`.

### Intermediate level

**Answer 6.** The trick `x &= x - 1` removes the rightmost 1 bit, so the loop runs only as many times as there are 1 bits.

```c
uint8_t count_ones(uint32_t x) {
    uint8_t count = 0;
    while (x) {
        x &= x - 1;
        count++;
    }
    return count;
}
```

An easier-to-follow version: loop 32 times, add `x & 1U` to `count`, then `x >>= 1`.

**Answer 7.** A power of two has exactly one bit set to 1, so `x & (x - 1)` is 0. Zero must be excluded.

```c
bool is_power_of_two(uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}
```

**Answer 8.** The cast to `uint8_t` is needed because the shift result is an `int` and the bits above bit 7 must be discarded.

```c
uint8_t swap_nibbles(uint8_t x) {
    return (uint8_t)((x << 4) | (x >> 4));
}
```

**Answer 9.** Three bugs:

1. The macro has no parentheses and uses a signed `1`. Fix: `#define BIT(n) (1U << (n))`.
2. Operator precedence: `==` is evaluated before `&`. Fix: `if ((status & BIT(3)) == 0)`.
3. `status = BIT(5)` erases the other bits. Fix: `status |= BIT(5);`.

**Answer 10.**

- (a) Needed. The variable is changed by an ISR outside `main`'s flow, so the compiler must re-read it every time.
- (b) Not needed. It is used by a single flow of code, and `volatile` would only slow it down.
- (c) Needed, on the pointed-to data: `volatile uint32_t *`. The register value is changed by hardware.
- (d) Not needed. Its contents never change.

**Answer 11.** `DD AA`. On little-endian the lowest byte (`0xDD`) is at the lowest address, and the highest byte (`0xAA`) is at offset 3.

### Advanced level

**Answer 12.** The `width` of 32 case is handled separately because `1U << 32` is undefined behavior.

```c
static uint32_t field_mask(uint8_t width) {
    return (width >= 32) ? UINT32_MAX : ((UINT32_C(1) << width) - 1U);
}

uint32_t read_field(uint32_t reg, uint8_t pos, uint8_t width) {
    return (reg >> pos) & field_mask(width);
}

void write_field(uint32_t *reg, uint8_t pos, uint8_t width, uint32_t val) {
    uint32_t mask = field_mask(width);
    *reg = (*reg & ~(mask << pos)) | ((val & mask) << pos);
}
```

**Answer 13.** The final value is `CTRL = 0x00004F0B`.

```c
uint32_t CTRL = 0;
CTRL |= (1U << 0);                              // EN   -> 0x00000001
CTRL = (CTRL & ~(0x7U  << 1)) | (5U    << 1);   // MODE -> 0x0000000B
CTRL = (CTRL & ~(0xFFU << 8)) | (0x4FU << 8);   // DIV  -> 0x00004F0B
```

`MODE = 5` is binary `101`, shifted by 1 it becomes `1010` (`0xA`), plus the `EN` bit gives `0xB`.

**Answer 14.** Extract each byte with a mask, then shift it to its mirrored position.

```c
uint32_t swap_endian32(uint32_t x) {
    return ((x & 0x000000FFU) << 24) |
           ((x & 0x0000FF00U) <<  8) |
           ((x & 0x00FF0000U) >>  8) |
           ((x & 0xFF000000U) >> 24);
}
```

**Answer 15.**

- (a) `ticks++` consists of three steps: read, add, write. If an interrupt arrives after `main` has read but before it writes, the ISR's increment is overwritten and lost. `volatile` only guarantees that the memory accesses really happen, not that the three steps cannot be interrupted. Fixes: briefly disable interrupts around `ticks++` in `main` (a critical section), or arrange for only the ISR to write and `main` to only read, or use an atomic type from `<stdatomic.h>` if the platform supports it.
- (b) Read from right to left: `STATUS` is a pointer with a fixed address (`*const`), pointing to a `uint32_t` that is `volatile` (its value can change through hardware, so it must always be re-read) and `const` (the program may not write through this pointer). This suits a read-only status register.
