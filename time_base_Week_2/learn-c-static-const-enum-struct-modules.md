# Learning C: static, const, enum, Structs, and Splitting Code into .c/.h

## Progress

Check off an item by changing `[ ]` to `[x]`, then commit.

**Topics**

- [ ] 1. Scope, lifetime, and linkage
- [ ] 2. static
- [ ] 3. const
- [ ] 4. enum
- [ ] 5. Structs and pointers to structs
- [ ] 6. Splitting code into .c and .h files
- [ ] 7. Putting it together: a small GPIO module
- [ ] 8. Common mistakes and pitfalls

**Exercises**

- [ ] Exercise 1 (basic): `static` local vs normal local
- [ ] Exercise 2 (basic): calling a `static` function from another file
- [ ] Exercise 3 (basic): which `const` lines fail to compile
- [ ] Exercise 4 (basic): `enum` values
- [ ] Exercise 5 (basic): `.` versus `->`
- [ ] Exercise 6 (intermediate): traffic light state machine
- [ ] Exercise 7 (intermediate): `make_id` and `reset_ids`
- [ ] Exercise 8 (intermediate): find three bugs in a header
- [ ] Exercise 9 (intermediate): a `Stats` struct
- [ ] Exercise 10 (intermediate): timer register map offsets
- [ ] Exercise 11 (intermediate): RGB color lookup table
- [ ] Exercise 12 (advanced): split a program into a module
- [ ] Exercise 13 (advanced): an LED handle with active-low support
- [ ] Exercise 14 (advanced): button debouncing with `static`
- [ ] Exercise 15 (advanced): three concept questions

**Study notes**

| Date | What I learned | What is still confusing |
| --- | --- | --- |
|  |  |  |

## How to use this material

Week 1 was about reaching a single register: `stdint.h`, bit operations, pointers, and `volatile`. This week is about **organizing** code once it grows past one file. You will need it immediately: SysTick, timers, EXTI, and UART each deserve their own `.c`/`.h` pair instead of one huge `main.c`.

1. Scope, lifetime, and linkage: the three properties every name in C has. `static` only makes sense once these are clear.
2. `static`: keeping a value between calls, and hiding things inside one file.
3. `const`: promises that data will not change, and where constant data is stored.
4. `enum`: named integer constants for modes, states, and status codes.
5. Structs and pointers to structs: grouping related data, and the `->` operator.
6. Splitting code into `.c` and `.h` files: declarations, definitions, header guards.
7. All of it combined into a small GPIO module, simulated on a PC.

Prerequisites: the Week 1 material (bit operations, pointers, `volatile`). Single-file examples run on a PC with:

```bash
gcc -std=c11 -Wall -Wextra -O2 example.c -o example && ./example
```

Multi-file examples list every `.c` file on the command line:

```bash
gcc -std=c11 -Wall -Wextra -O2 main.c counter.c -o app && ./app
```

Retype the examples instead of copy-pasting. Do the exercises on paper first, then check them with the compiler and the answer key at the end.

**How this connects to the hardware this week**

| Week 2 hardware task | C topic you will use |
| --- | --- |
| SysTick millisecond counter | `static volatile` variable inside `systick.c`, read through a function |
| Timer and EXTI interrupts | ISR-shared state, `.c`/`.h` per peripheral |
| Button debouncing | `static` locals or module state, `enum` for button states |
| UART driver | struct for the register map, `const` for read-only buffers |
| LED modes (off, on, blink) | `enum` plus `switch` |

## 1. Scope, lifetime, and linkage

Every name in C has three separate properties. Mixing them up is the main reason `static` feels confusing, because `static` changes a different property depending on where it is written.

| Property | Question it answers | Possible values |
| --- | --- | --- |
| **Scope** | Where in the source code can I use this name? | block (inside `{ }`), file |
| **Lifetime** (storage duration) | How long does the memory exist while the program runs? | automatic (until the block ends), static (the whole program) |
| **Linkage** | Can another `.c` file refer to the same thing? | external (yes), internal (no), none |

### Example 1.1: a local and a global

```c
#include <stdio.h>

int global_count = 0;              // file scope, static lifetime, external linkage

void f(void) {
    int local = 0;                 // block scope, automatic lifetime
    local++;
    global_count++;
    printf("local=%d global=%d\n", local, global_count);
}

int main(void) {
    f();
    f();
    f();
    return 0;
}
```

Output:

```text
local=1 global=1
local=1 global=2
local=1 global=3
```

`local` is created fresh on every call (on the stack) and disappears when `f` returns, so it is always 1. `global_count` exists for the whole program, so it keeps counting.

Where does a static-lifetime variable live? You already wrote the answer in Week 1. Your linker script has a `.data` section (variables with an initial value, copied from Flash to RAM by `Reset_Handler`) and a `.bss` section (variables that start at zero, cleared by `Reset_Handler`). Every global and every `static` variable lands in one of those two. Stack variables do not.

## 2. static

`static` means two different things depending on where you write it.

| Where | What `static` changes | Effect |
| --- | --- | --- |
| On a local variable (inside a function) | **lifetime** becomes static | the value survives between calls |
| On a global variable or a function (outside any function) | **linkage** becomes internal | the name is private to this `.c` file |

### 2.1 static on a local variable

```c
#include <stdio.h>

void on_press(void) {
    static int count = 0;          // initialized once, keeps its value between calls
    count++;
    printf("pressed %d time(s)\n", count);
}

int main(void) {
    on_press();
    on_press();
    on_press();
    return 0;
}
```

Output:

```text
pressed 1 time(s)
pressed 2 time(s)
pressed 3 time(s)
```

The line `static int count = 0;` is **not** executed on each call. The variable sits in `.bss` (or `.data` if the initializer is not zero) and is set up once by the startup code, before `main`. Every call to `on_press` sees the same variable. Its scope is still only the function, so no other function can touch it.

The initializer of a `static` local must be a constant. `static int x = some_function();` does not compile in C.

### 2.2 static on a global variable or function

Without `static`, every global variable and function has external linkage: any other `.c` file can reach it by declaring it. With `static`, it becomes private to its own file.

`counter.c`:

```c
#include <stdint.h>
#include <stdio.h>

static uint32_t count = 0;              // private: only counter.c can see it

static void log_change(void) {          // private helper
    printf("count is now %u\n", (unsigned)count);
}

void counter_increment(void) {          // public
    count++;
    log_change();
}

uint32_t counter_get(void) {            // public
    return count;
}
```

`main.c`:

```c
#include <stdint.h>
#include <stdio.h>

void counter_increment(void);
uint32_t counter_get(void);

int main(void) {
    counter_increment();
    counter_increment();
    printf("main sees %u\n", (unsigned)counter_get());
    return 0;
}
```

Output:

```text
count is now 1
count is now 2
main sees 2
```

If `main.c` tries to reach the private variable with `extern uint32_t count;` and writes `count = 100;`, each file compiles, but the **linker** fails:

```text
undefined reference to `count'
```

That is exactly the protection you want: the only way to change `count` is through `counter_increment`, so it can never be set to a nonsense value from somewhere else.

### 2.3 Why this matters on a microcontroller

- **Module state.** `systick.c` owns `static volatile uint32_t s_ticks;`. Other files call `systick_get_ms()` instead of touching the variable directly.
- **Private helpers.** A function like `write_field2()` that only the GPIO module needs should be `static`, so its name cannot collide with a helper of the same name in `uart.c`.
- **Fewer surprises.** When a variable is `static`, you know every line that can change it is in one file. Debugging becomes much easier.

A common naming habit is an `s_` prefix for file-level `static` variables (`s_ticks`, `s_count`). It is optional, but it makes private state easy to spot.

Rule of thumb: **make every global variable and every helper function `static` unless another file truly needs it.**

## 3. const

`const` is a promise: "this data will not be changed through this name". The compiler enforces the promise, and on a microcontroller it also decides where the data is stored.

### 3.1 const variables and const parameters

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

static const uint8_t brightness[5] = {0, 4, 16, 64, 255};

static uint32_t sum_bytes(const uint8_t *data, size_t len) {
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    // data[0] = 1;   // error: assignment of read-only location
    return sum;
}

int main(void) {
    uint8_t ram_buf[3] = {1, 2, 3};
    printf("%u\n", (unsigned)sum_bytes(brightness, 5));   // 339
    printf("%u\n", (unsigned)sum_bytes(ram_buf, 3));      // 6: non-const data is fine too
    return 0;
}
```

`const uint8_t *data` in the parameter list tells every caller: "I will only read your buffer". It accepts both constant and ordinary data. Use it for every pointer parameter that the function does not write through. A UART send function is the classic example: `void uart_send(const uint8_t *buf, size_t len);`.

The Week 1 rule still applies to pointers: `const` on the **left of the `*`** locks the data, on the **right of the `*`** locks the pointer.

### 3.2 const data lives in Flash

This is the most important practical effect of `const` on an STM32. The compiler puts `const` globals in the `.rodata` section, and your linker script places `.rodata` in **FLASH**. Data that is not `const` goes in `.data`, which costs space in Flash (for the initial values) **and** in RAM (for the working copy).

Measured with `arm-none-eabi-gcc -Os` on a 256-byte table:

| Declaration | `text` (Flash) | `data` (Flash + RAM) |
| --- | --- | --- |
| `const uint8_t table[256] = {1, 2, 3};` | 256 | 0 |
| `uint8_t table[256] = {1, 2, 3};` | 0 | 256 |

The F407 has 1 MB of Flash but only 128 KB of main RAM, so lookup tables (gamma curves, sine tables, font bitmaps, color masks) should always be `const`. Try it in your own project: add a table, build, and watch the numbers printed by `arm-none-eabi-size` after each build.

### 3.3 const versus #define, and a surprise

In C, a `const` variable is **not** a compile-time constant. It cannot size an array at file scope and cannot be a `case` label:

```c
const int SIZE = 4;
int table[SIZE];                 // error: variably modified 'table' at file scope

int f(int x) {
    switch (x) {
    case SIZE: return 1;         // error: case label does not reduce to an integer constant
    default:   return 0;
    }
}
```

(C++ behaves differently here, which is a common source of confusion.)

| Tool | Has a type | Respects scope | Compile-time constant | Visible in the debugger |
| --- | --- | --- | --- | --- |
| `#define SIZE 4` | no | no (text replacement) | yes | no |
| `const int SIZE = 4;` | yes | yes | **no** | yes |
| `enum { SIZE = 4 };` | yes (`int`) | yes | yes | yes, usually |

So: use `const` for data (tables, configuration structs), `enum` for groups of related integer constants, and `#define` for register addresses, bit masks, and values that must work in `#if`.

## 4. enum

An `enum` gives names to a set of integer constants. It turns magic numbers into words.

### 4.1 Values

Values start at 0 and go up by 1. You can set a value explicitly, and the next unnamed one continues from there:

```c
#include <stdio.h>

enum Color { RED, GREEN = 5, BLUE, WHITE = 1, BLACK };

int main(void) {
    printf("RED=%d GREEN=%d BLUE=%d WHITE=%d BLACK=%d\n",
           RED, GREEN, BLUE, WHITE, BLACK);
    printf("sizeof(enum Color) = %zu\n", sizeof(enum Color));
    return 0;
}
```

Output:

```text
RED=0 GREEN=5 BLUE=6 WHITE=1 BLACK=2
sizeof(enum Color) = 4          (on the PC)
```

Duplicate values are allowed (`BLACK` and `RED` could both be 0).

The size is a surprise worth remembering. On the PC an enum is stored like an `int` (4 bytes). Your `arm-none-eabi-gcc`, however, uses **short enums** by default: the compiler picks the smallest type that fits all values, so this enum is **1 byte** on the STM32. Compile `int sz = sizeof(enum Color);` with both compilers and compare. Section 4.5 explains why this matters.

### 4.2 enum for hardware field values

You met the MODER field last week: `00` input, `01` output, `10` alternate function, `11` analog. With an enum the meaning is written into the code:

```c
typedef enum {
    GPIO_MODE_INPUT  = 0,   // 00
    GPIO_MODE_OUTPUT = 1,   // 01
    GPIO_MODE_AF     = 2,   // 10
    GPIO_MODE_ANALOG = 3    // 11
} GpioMode;
```

`gpio_set_mode(GPIOB, 12, GPIO_MODE_OUTPUT)` is far easier to read than writing `1U << 24` by hand, and the function signature tells the reader which values make sense.

`typedef` lets you write `GpioMode` instead of `enum GpioMode` everywhere. The same trick works for structs in section 5.

### 4.3 enum plus switch: a state machine

```c
#include <stdio.h>
#include <stdbool.h>

typedef enum {
    LED_MODE_OFF,
    LED_MODE_ON,
    LED_MODE_BLINK
} LedMode;

static const char *mode_name(LedMode m) {
    switch (m) {
    case LED_MODE_OFF:   return "OFF";
    case LED_MODE_ON:    return "ON";
    case LED_MODE_BLINK: return "BLINK";
    }
    return "?";                      // reached only with an invalid value
}

static LedMode next_mode(LedMode m) {     // called on every button press
    switch (m) {
    case LED_MODE_OFF:   return LED_MODE_ON;
    case LED_MODE_ON:    return LED_MODE_BLINK;
    case LED_MODE_BLINK: return LED_MODE_OFF;
    }
    return LED_MODE_OFF;
}

int main(void) {
    LedMode mode = LED_MODE_OFF;
    for (int press = 1; press <= 4; press++) {
        mode = next_mode(mode);
        printf("press %d -> %s\n", press, mode_name(mode));
    }
    return 0;
}
```

Output:

```text
press 1 -> ON
press 2 -> BLINK
press 3 -> OFF
press 4 -> ON
```

A bonus from the compiler: if a `switch` on an enum has no `default` and you forget a value, `-Wall` warns you:

```text
warning: enumeration value 'LED_MODE_BLINK' not handled in switch [-Wswitch]
```

That is a good reason to leave out `default` when you handle every value: when you add `LED_MODE_FADE` later, the compiler lists every `switch` you still need to update.

### 4.4 The COUNT trick and status codes

Add one last entry to get the number of values for free, and use it to size arrays:

```c
typedef enum {
    COLOR_OFF,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_COUNT        // = 4, not a real color
} Color;

static const uint32_t color_mask[COLOR_COUNT] = { ... };
```

Enums are also the standard way to report errors (you will use this heavily in Week 5):

```c
typedef enum {
    STATUS_OK = 0,
    STATUS_TIMEOUT,
    STATUS_NO_DEVICE,
    STATUS_BAD_ARG
} Status;

Status bme280_read_id(uint8_t *id);
```

### 4.5 What enum does not protect you from

- **No range check.** `GpioMode m = 7;` compiles without a warning, even with `-Wall -Wextra`. If a value comes from outside (a function argument, a received byte), check it: `if (m > GPIO_MODE_ANALOG) return STATUS_BAD_ARG;`.
- **Unknown size for layouts.** The size of an enum is chosen by the compiler: 4 bytes with GCC on the PC, but 1 byte for small enums with `arm-none-eabi-gcc` (short enums), and it can change when you add a value above 255. Never use an enum type as a member of a register-map struct or of a binary packet format; use `uint32_t` or `uint8_t` there and convert.

## 5. Structs and pointers to structs

A struct groups related values under one name. Combined with pointers it is the backbone of every driver: the register map of a peripheral is a struct, and a driver "handle" (Week 4) is a struct.

### 5.1 Defining, initializing, and accessing

```c
typedef struct {
    uint8_t pin;
    bool    active_low;
    bool    is_on;
} Led;

Led red = { .pin = 12, .active_low = false };   // designated initializer
```

Members you do not mention in the initializer are set to zero, so `red.is_on` is `false`.

| You have | Access a member with | Example |
| --- | --- | --- |
| the struct itself | `.` | `red.pin` |
| a pointer to the struct | `->` | `p->pin` |

`p->pin` is just shorthand for `(*p).pin`: "go to the address in `p`, then take the `pin` member". The parentheses are required because `.` binds tighter than `*`.

### 5.2 Passing structs to functions

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t pin;
    bool    active_low;
    bool    is_on;
} Led;

static void led_set_copy(Led led, bool on) {   // receives a COPY
    led.is_on = on;                            // changes the copy only
}

static void led_set(Led *led, bool on) {       // receives the ADDRESS
    led->is_on = on;                           // changes the caller's struct
}

static bool led_is_on(const Led *led) {        // read-only access
    return led->is_on;
}

int main(void) {
    Led red = { .pin = 12, .active_low = false };   // is_on is zero (false)

    led_set_copy(red, true);
    printf("after led_set_copy: %d\n", led_is_on(&red));   // 0

    led_set(&red, true);
    printf("after led_set:      %d\n", led_is_on(&red));   // 1

    Led *p = &red;
    printf("pin via p->pin:     %u\n", p->pin);            // 12
    printf("pin via (*p).pin:   %u\n", (*p).pin);          // 12
    return 0;
}
```

The compiler even warns about the useless copy: `parameter 'led' set but not used`. Pay attention to that warning; it often means "you meant to pass a pointer".

The three ways to pass a struct:

| Signature | Copies the struct | Can change the caller's data | Use when |
| --- | --- | --- | --- |
| `void f(Led led)` | yes | no | almost never (tiny structs only) |
| `void f(Led *led)` | no | yes | the function modifies the struct |
| `void f(const Led *led)` | no | no | the function only reads |

On a microcontroller, copying a large struct costs stack space and time, so pointers are the norm.

### 5.3 Arrays of structs and walking them with a pointer

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t pin;
    bool    is_on;
} Led;

int main(void) {
    Led leds[3] = {
        { .pin = 12 },
        { .pin = 13, .is_on = true },
        { .pin = 14 },
    };

    for (Led *p = leds; p < leds + 3; p++) {     // p walks one Led at a time
        printf("pin %u is %s\n", p->pin, p->is_on ? "on" : "off");
    }
    return 0;
}
```

Output:

```text
pin 12 is off
pin 13 is on
pin 14 is off
```

`p++` moves by `sizeof(Led)` bytes, one whole element, just like the pointer arithmetic from Week 1.

### 5.4 Memory layout and padding

The compiler may insert unused bytes between members so that each member sits at an address that suits its size (a `uint32_t` wants an address divisible by 4). The order of members therefore changes the size of the struct:

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t  a;
    uint32_t b;
    uint8_t  c;
} Messy;

typedef struct {
    uint32_t b;
    uint8_t  a;
    uint8_t  c;
} Tidy;

int main(void) {
    printf("Messy: a@%zu b@%zu c@%zu size=%zu\n",
           offsetof(Messy, a), offsetof(Messy, b), offsetof(Messy, c), sizeof(Messy));
    printf("Tidy:  b@%zu a@%zu c@%zu size=%zu\n",
           offsetof(Tidy, b), offsetof(Tidy, a), offsetof(Tidy, c), sizeof(Tidy));
    return 0;
}
```

Output (same on a PC and on the Cortex-M4):

```text
Messy: a@0 b@4 c@8 size=12
Tidy:  b@0 a@4 c@5 size=8
```

```text
Messy:  | a | pad pad pad | b b b b | c | pad pad pad |    12 bytes
Tidy:   | b b b b | a | c | pad pad |                    8 bytes
```

`offsetof(type, member)` from `<stddef.h>` gives the byte offset of a member. You will use it in a moment to check register maps.

### 5.5 A register map as a struct

Last week you wrote one `#define` per register. A struct describes a whole peripheral at once, because its registers sit one after another at fixed offsets:

```c
#include <stdint.h>
#include <stddef.h>

typedef struct {
    volatile uint32_t MODER;      /* 0x00 */
    volatile uint32_t OTYPER;     /* 0x04 */
    volatile uint32_t OSPEEDR;    /* 0x08 */
    volatile uint32_t PUPDR;      /* 0x0C */
    volatile uint32_t IDR;        /* 0x10 */
    volatile uint32_t ODR;        /* 0x14 */
    volatile uint32_t BSRR;       /* 0x18 */
    volatile uint32_t LCKR;       /* 0x1C */
    volatile uint32_t AFR[2];     /* 0x20 AFRL, 0x24 AFRH */
} GPIO_TypeDef;

_Static_assert(offsetof(GPIO_TypeDef, IDR)  == 0x10, "IDR offset");
_Static_assert(offsetof(GPIO_TypeDef, BSRR) == 0x18, "BSRR offset");
_Static_assert(offsetof(GPIO_TypeDef, AFR)  == 0x20, "AFR offset");
_Static_assert(sizeof(GPIO_TypeDef) == 0x28, "GPIO size");

#define GPIOB  ((GPIO_TypeDef *)0x40020400UL)

void led_on(void) { GPIOB->BSRR = (1U << 12); }
```

`GPIOB` is a pointer to a struct placed at the base address, and `GPIOB->BSRR` lands at `0x40020400 + 0x18` automatically. One `#define` per port replaces ten per port, and the same functions work for every port: just pass `GPIOA`, `GPIOB`, or `GPIOD`.

Why every member is `uint32_t`: all members have the same size and alignment, so the compiler never inserts padding, and the offsets match the reference manual exactly. `AFR[2]` shows that an array member works too.

**`_Static_assert` is your safety net.** It is checked at compile time and costs nothing at run time. If someone forgets a member, the build stops:

```text
error: static assertion failed: "BSRR must be at offset 0x18"
```

Without the check, `GPIOB->BSRR` would quietly write to the wrong register. Put a few `_Static_assert` lines next to every register map you write.

## 6. Splitting code into .c and .h files

### 6.1 Why split

- **Reuse:** `gpio.c` works in every project.
- **Privacy:** `static` state and helpers stay inside the module.
- **Testing:** in Week 8 you will compile a module on the PC with fake hardware, which only works if the logic is in its own file.
- **Faster builds:** only changed files are recompiled.
- **Readability:** `main.c` describes *what* the program does; modules describe *how*.

### 6.2 Declaration versus definition

| | Declaration | Definition |
| --- | --- | --- |
| Says | "this exists somewhere" | "here it is" |
| Function | `void counter_increment(void);` (prototype) | `void counter_increment(void) { ... }` (body) |
| Variable | `extern uint32_t retry_count;` | `uint32_t retry_count = 3;` |
| How many allowed | as many as you like | exactly one in the whole program |
| Where it goes | `.h` file | `.c` file |

The whole system rests on one rule: **headers contain declarations, `.c` files contain definitions.**

### 6.3 What happens when you build

```text
main.c ──► preprocessor (pastes counter.h in) ──► compiler ──► main.o    ─┐
                                                                         ├─► linker ──► app / blink.elf
counter.c ──► preprocessor (pastes counter.h in) ──► compiler ──► counter.o ─┘
```

- `#include "counter.h"` literally copies the text of the header into the `.c` file.
- Each `.c` file is compiled **alone** into an object file (`.o`). While compiling `main.c`, the compiler knows nothing about `counter.c` except what the header declared.
- The **linker** joins the `.o` files and connects each use of a name to its single definition.

Two linker errors you will meet, and what they mean:

| Error | Meaning | Usual cause |
| --- | --- | --- |
| `undefined reference to 'x'` | declared, but no definition was found | the `.c` file was not added to the build, the name is `static`, or a typo |
| `multiple definition of 'x'` | more than one definition | a variable or function body written in a header included by several `.c` files |

### 6.4 Anatomy of a module

`counter.h`:

```c
#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

/* Public interface of the counter module. */
void     counter_increment(void);
void     counter_reset(void);
uint32_t counter_get(void);

#endif /* COUNTER_H */
```

`counter.c`:

```c
#include "counter.h"           /* own header first */

static uint32_t s_count = 0;   /* private state */

void counter_increment(void) {
    s_count++;
}

void counter_reset(void) {
    s_count = 0;
}

uint32_t counter_get(void) {
    return s_count;
}
```

`main.c`:

```c
#include <stdio.h>
#include "counter.h"

int main(void) {
    counter_increment();
    counter_increment();
    counter_increment();
    printf("count = %u\n", (unsigned)counter_get());   /* 3 */

    counter_reset();
    printf("count = %u\n", (unsigned)counter_get());   /* 0 */
    return 0;
}
```

Build in one step, or in the three steps the build system performs for you:

```bash
gcc -std=c11 -Wall -Wextra -O2 main.c counter.c -o app && ./app

gcc -std=c11 -Wall -Wextra -O2 -c counter.c -o counter.o
gcc -std=c11 -Wall -Wextra -O2 -c main.c    -o main.o
gcc main.o counter.o -o app
```

The pieces of the header:

- **Include guard** (`#ifndef COUNTER_H` / `#define COUNTER_H` / `#endif`). The first time the header is pasted in, `COUNTER_H` is not yet defined, so the content is used and `COUNTER_H` gets defined. Any later copy in the same `.c` file is skipped. Without it, including the header twice (which easily happens through other headers) redefines the types and fails to compile. `#pragma once` does the same job and is supported by GCC, but the `#ifndef` form is standard C.
- **Only the includes the header itself needs** (`<stdint.h>` because it uses `uint32_t`).
- **Types, prototypes, `extern` declarations, macros** that users of the module need.

What does **not** belong in a header: function bodies (except `static inline` helpers, which come later), variable definitions, and anything private.

Why include the module's own header first in its `.c` file: the compiler then checks that the prototypes in the header match the definitions, and it proves the header compiles on its own without depending on something included before it.

### 6.5 Sharing a variable between files

Sometimes a variable really has to be global. Declare it with `extern` in the header and define it in exactly one `.c` file:

```c
/* config.h */
extern uint32_t retry_count;     /* declaration: no memory is created */

/* config.c */
uint32_t retry_count = 3;        /* definition: the memory lives here */
```

Prefer a `static` variable with getter and setter functions when you can. Then you control every change in one place.

### 6.6 Adding modules to your CMake project

A structure that matches your roadmap (Week 5 asks for `drivers/`, `app/`, `docs/`):

```text
blink/
├── CMakeLists.txt
├── app/
│   └── main.c
├── drivers/
│   ├── gpio.c
│   └── gpio.h
└── startup/
    ├── startup.c
    └── stm32f407.ld
```

In `CMakeLists.txt`, list every `.c` file and tell the compiler where the headers are:

```cmake
add_executable(blink
    startup/startup.c
    app/main.c
    drivers/gpio.c
)

target_include_directories(blink PRIVATE drivers)
```

Forgetting the new `.c` file in `add_executable` is the classic cause of `undefined reference`.

## 7. Putting it together: a small GPIO module

This module uses everything from this week: a struct register map, enums for field values, a `static` helper, `const` pointers, and a `.c`/`.h` split. It runs on a PC with a fake GPIO port, exactly like Example 6.1 from Week 1.

`gpio.h`:

```c
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>

/* ---------- Register map (RM0090, GPIO register map) ---------- */
typedef struct {
    volatile uint32_t MODER;      /* 0x00 */
    volatile uint32_t OTYPER;     /* 0x04 */
    volatile uint32_t OSPEEDR;    /* 0x08 */
    volatile uint32_t PUPDR;      /* 0x0C */
    volatile uint32_t IDR;        /* 0x10 */
    volatile uint32_t ODR;        /* 0x14 */
    volatile uint32_t BSRR;       /* 0x18 */
    volatile uint32_t LCKR;       /* 0x1C */
    volatile uint32_t AFR[2];     /* 0x20, 0x24 */
} GPIO_TypeDef;

#define GPIOA  ((GPIO_TypeDef *)0x40020000UL)
#define GPIOB  ((GPIO_TypeDef *)0x40020400UL)

/* ---------- Values of the 2-bit fields ---------- */
typedef enum {
    GPIO_MODE_INPUT  = 0,   /* 00 */
    GPIO_MODE_OUTPUT = 1,   /* 01 */
    GPIO_MODE_AF     = 2,   /* 10 */
    GPIO_MODE_ANALOG = 3    /* 11 */
} GpioMode;

typedef enum {
    GPIO_PULL_NONE = 0,     /* 00 */
    GPIO_PULL_UP   = 1,     /* 01 */
    GPIO_PULL_DOWN = 2      /* 10 */
} GpioPull;

/* ---------- Public functions ---------- */
void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, GpioMode mode);
void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GpioPull pull);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool high);
bool gpio_read(const GPIO_TypeDef *port, uint8_t pin);

#endif /* GPIO_H */
```

`gpio.c`:

```c
#include "gpio.h"

/* Private helper: write a 2-bit field for one pin (clear, then set). */
static void write_field2(volatile uint32_t *reg, uint8_t pin, uint32_t value) {
    uint32_t shift = pin * 2U;
    *reg = (*reg & ~(3U << shift)) | ((value & 3U) << shift);
}

void gpio_set_mode(GPIO_TypeDef *port, uint8_t pin, GpioMode mode) {
    write_field2(&port->MODER, pin, (uint32_t)mode);
}

void gpio_set_pull(GPIO_TypeDef *port, uint8_t pin, GpioPull pull) {
    write_field2(&port->PUPDR, pin, (uint32_t)pull);
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, bool high) {
    port->BSRR = high ? (1U << pin) : (1U << (pin + 16U));
}

bool gpio_read(const GPIO_TypeDef *port, uint8_t pin) {
    return ((port->IDR >> pin) & 1U) != 0U;
}
```

`main.c` (PC simulation):

```c
#include <stdio.h>
#include <inttypes.h>
#include "gpio.h"

/* On a PC there is no GPIO hardware, so an ordinary struct stands in for it. */
static GPIO_TypeDef fake_gpiob;

/* Imitates what the real hardware does after a BSRR write. */
static void sim_apply_bsrr(GPIO_TypeDef *g) {
    uint32_t v = g->BSRR;
    g->ODR &= ~(v >> 16);       /* reset bits first ...              */
    g->ODR |=  (v & 0xFFFFU);   /* ... then set bits, so set wins    */
    g->BSRR = 0;                /* BSRR keeps nothing                */
}

int main(void) {
    GPIO_TypeDef *port = &fake_gpiob;   /* on the STM32: GPIO_TypeDef *port = GPIOB; */

    gpio_set_mode(port, 12, GPIO_MODE_OUTPUT);   /* LED    */
    gpio_set_mode(port, 15, GPIO_MODE_INPUT);    /* button */
    gpio_set_pull(port, 15, GPIO_PULL_UP);
    printf("MODER = 0x%08" PRIX32 "\n", port->MODER);   /* 0x01000000 */
    printf("PUPDR = 0x%08" PRIX32 "\n", port->PUPDR);   /* 0x40000000 */

    gpio_write(port, 12, true);
    sim_apply_bsrr(port);
    printf("ODR   = 0x%08" PRIX32 "\n", port->ODR);     /* 0x00001000 */

    gpio_write(port, 12, false);
    sim_apply_bsrr(port);
    printf("ODR   = 0x%08" PRIX32 "\n", port->ODR);     /* 0x00000000 */

    fake_gpiob.IDR = (1U << 15);                        /* button released */
    printf("button pin = %d\n", gpio_read(port, 15));   /* 1 */
    fake_gpiob.IDR = 0;                                 /* button pressed  */
    printf("button pin = %d\n", gpio_read(port, 15));   /* 0 */
    return 0;
}
```

```bash
gcc -std=c11 -Wall -Wextra -O2 main.c gpio.c -o app && ./app
```

Things to notice:

- `gpio.c` never mentions an address. It only works through the `port` pointer, so the same code drives the fake struct on the PC and the real `GPIOB` on the board. This is the idea behind Week 8 unit tests.
- `write_field2` is `static`: an internal detail, invisible to `main.c`.
- `gpio_read` takes `const GPIO_TypeDef *`: reading a pin never changes the port. The members are still `volatile`, so each read really goes to the hardware.
- The `sim_apply_bsrr` function is a tiny model of the hardware you studied last week: BSRR stores nothing, and set wins over reset.
- Clock enable is not part of this module. On the board, `main` still has to switch on the GPIOB clock in RCC before calling `gpio_set_mode`. A good follow-up exercise is an `rcc.c`/`rcc.h` pair.

To use it on the STM32, move `gpio.h` and `gpio.c` into `drivers/`, add them to CMake, and rewrite your LED-and-button `main.c` with `GPIOB` instead of `&fake_gpiob`.

## 8. Common mistakes and pitfalls

**1. Defining a variable in a header.** `int retry_count = 3;` in a header included by two `.c` files gives `multiple definition of 'retry_count'`. Put `extern` in the header and the definition in one `.c` file.

**2. Missing include guard.** Including a header twice in the same file redefines its types. With an anonymous struct the error looks strange: `conflicting types for 'Button'; have 'struct <anonymous>'`.

**3. A `static` function declared in a header.** Every file that includes the header gets its own private declaration, but the body exists in only one file. The others get `warning: 'f' used but never defined` and then `undefined reference to 'f'`. If a function is in the header, it is public: no `static`.

**4. Forgetting `static` on private globals.** Two modules that both define a global `count` collide at link time. Make module state `static`.

**5. Expecting a `static` local to reset on every call.** The initializer runs once, before `main`. If you need a reset, move the variable to file scope and add a reset function (Exercise 7).

**6. Mixing up `.` and `->`.** `ptr.member` and `obj->member` are both errors. GCC is kind here: `'p' is a pointer; did you mean to use '->'?`

**7. Passing a struct by value when you meant to change it.** It compiles and runs, and the change silently disappears. Watch for `set but not used` warnings on parameters.

**8. Returning a pointer to a local variable.** The local dies when the function returns, so the pointer points to garbage. Return the value, write through a pointer parameter, or use a `static` variable if that really fits.

**9. Trusting an enum's range.** `GpioMode m = 7;` compiles. Check values that come from outside before using them as an array index or a field value.

**10. Register maps that drift.** One missing or wrongly sized member shifts every register after it. Use `uint32_t` members only and add `_Static_assert(offsetof(...))` checks.

**11. Large tables without `const`.** They cost RAM for no reason. Measure with `arm-none-eabi-size`.

**12. Including a `.c` file.** `#include "gpio.c"` "works" until two files do it, then everything is defined twice. Include headers; list `.c` files in the build.

## 9. Exercises

There are 15 exercises at three levels. For the multi-file exercises, write each file separately and build with all `.c` files on the command line.

### Basic level

**Exercise 1.** What does this program print?

```c
#include <stdio.h>

void tick(void) {
    static int s = 0;
    int a = 0;
    s += 2;
    a += 2;
    printf("s=%d a=%d\n", s, a);
}

int main(void) {
    tick();
    tick();
    tick();
    return 0;
}
```

**Exercise 2.** Two files are built together. Does each file compile? Does the link succeed? Explain, and give two ways to fix it.

`util.c`:

```c
static int helper(int x) { return x * 2; }
int twice(int x) { return helper(x); }
```

`main.c`:

```c
int twice(int x);
int helper(int x);

int main(void) {
    return twice(3) + helper(3);
}
```

**Exercise 3.** Which of the numbered lines fail to compile?

```c
uint8_t a = 1, b = 2;
const uint8_t *p = &a;
uint8_t *const q = &a;
const uint8_t *const r = &a;
*p = 5;   // (1)
p = &b;   // (2)
*q = 5;   // (3)
q = &b;   // (4)
*r = 5;   // (5)
r = &b;   // (6)
```

**Exercise 4.** What is the value of each constant, and what is `EV_RELEASE + EV_TIMEOUT`?

```c
enum Event { EV_NONE, EV_PRESS = 10, EV_RELEASE, EV_HOLD = 20, EV_TIMEOUT };
```

**Exercise 5.** Which lines are wrong? Fix them. What are `red.pin` and `red.on` at the end?

```c
typedef struct { uint8_t pin; bool on; } Led;

Led red = { .pin = 12, .on = false };
Led *p = &red;
red->on = true;      // (a)
p.on = false;        // (b)
(*p).pin = 13;       // (c)
p->pin = 14;         // (d)
```

### Intermediate level

**Exercise 6.** Write a traffic light state machine:

```c
typedef enum { LIGHT_RED, LIGHT_GREEN, LIGHT_YELLOW, LIGHT_COUNT } Light;

Light light_next(Light current);       /* RED -> GREEN -> YELLOW -> RED */
uint32_t light_duration_ms(Light l);   /* RED 5000, GREEN 4000, YELLOW 1000, invalid 0 */
```

Use a `switch` for `light_next` and a `const` array indexed by the enum for `light_duration_ms`. An invalid input to `light_next` should return `LIGHT_RED`.

**Exercise 7.** Write `uint32_t make_id(void)`, which returns 1, 2, 3, ... on successive calls, and `void reset_ids(void)`, which makes the next `make_id()` return 1 again. Why can the counter not be a `static` local inside `make_id`?

**Exercise 8.** This project should exit with code 18 (pin 15 plus 3 retries). Find the three bugs and fix them.

`config.h`:

```c
#include <stdint.h>

int retry_count = 3;

typedef struct {
    uint8_t pin;
} Button;

void button_init(Button b);
```

`button.c`:

```c
#include "config.h"

void button_init(Button b) {
    b.pin = 15;
}
```

`main.c`:

```c
#include "config.h"
#include "config.h"     /* in a real project this happens through another header */

int main(void) {
    Button btn = {0};
    button_init(btn);
    return btn.pin + retry_count;     /* expected 15 + 3 */
}
```

**Exercise 9.** Write the functions for a temperature statistics struct:

```c
typedef struct {
    int16_t  min;
    int16_t  max;
    int32_t  sum;
    uint16_t count;
} Stats;

void    stats_reset(Stats *s);
void    stats_add(Stats *s, int16_t sample);
int16_t stats_average(const Stats *s);     /* 0 when count is 0 */
```

After `stats_reset` and samples `20, 25, -5, 40`, the result must be min = -5, max = 40, average = 20. What should `stats_reset` put in `min` and `max` so the first sample always replaces them?

**Exercise 10.** Here is the start of the timer register map:

```c
typedef struct {
    volatile uint32_t CR1;     /* 0x00 */
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
} TIM_TypeDef;

#define TIM2  ((TIM_TypeDef *)0x40000000UL)
```

(a) What are the offsets of `PSC` and `ARR`, and the address of `TIM2->ARR`? Compare with the *Register* sheet of your spreadsheet.
(b) Write `_Static_assert` checks for `SR`, `CNT`, `PSC`, and `ARR`.
(c) Someone deletes the `SMCR` line by accident. Without the checks, what happens when the code writes `TIM2->PSC = 15999;`?

**Exercise 11.** Your RGB LED uses PB12 (red), PB13 (green), and PB14 (blue), common cathode. Given:

```c
typedef enum { COLOR_OFF, COLOR_RED, COLOR_GREEN, COLOR_BLUE, COLOR_WHITE, COLOR_COUNT } Color;
```

Write `uint32_t color_to_bsrr(Color c)` that returns a single BSRR value which turns on exactly the pins for `c` and turns off the other two. Use a `const` lookup table indexed by the enum. Invalid colors must return "all off". What is the result for `COLOR_RED` and `COLOR_WHITE`?

### Advanced level

**Exercise 12.** Split this single-file program into `avg.h`, `avg.c`, and `main.c`. The internal state must be private, the header needs a guard, and add a `void avg_reset(void)` function. The output must stay `15` then `35`, followed by `0` after a reset.

```c
#include <stdio.h>
#include <stdint.h>

#define AVG_SIZE 4

uint16_t samples[AVG_SIZE];
uint8_t  pos = 0;
uint8_t  filled = 0;

void avg_add(uint16_t s) {
    samples[pos] = s;
    pos = (uint8_t)((pos + 1U) % AVG_SIZE);
    if (filled < AVG_SIZE) {
        filled++;
    }
}

uint16_t avg_get(void) {
    if (filled == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (uint8_t i = 0; i < filled; i++) {
        sum += samples[i];
    }
    return (uint16_t)(sum / filled);
}

int main(void) {
    avg_add(10);
    avg_add(20);
    printf("%u\n", avg_get());     /* 15 */
    avg_add(30);
    avg_add(40);
    avg_add(50);
    printf("%u\n", avg_get());     /* 35 */
    return 0;
}
```

(This array with a wrapping index is a small preview of the ring buffer in Week 3.)

**Exercise 13.** Using the GPIO module from section 7, write an `led.h`/`led.c` module:

```c
typedef struct {
    GPIO_TypeDef *port;
    uint8_t       pin;
    bool          active_low;    /* true for a common-anode RGB LED */
} Led;

void led_init(const Led *led);           /* pin to output, LED off */
void led_set(const Led *led, bool on);   /* on/off, respecting active_low */
```

`led_set(&led, true)` must turn the LED on for both wiring types. Test it on the PC with two LEDs on the fake port: a normal one on pin 12 and an active-low one on pin 13. After `led_init` on both, what is `ODR`? After turning both on?

**Exercise 14.** Write a debouncer that is called every 1 ms:

```c
bool button_pressed_event(bool raw_pressed);
```

`raw_pressed` is true while the contact is closed (already inverted for active-low wiring). The debounced state may only change after the raw input has disagreed with it for 5 calls in a row. The function returns true exactly once per real press (when the debounced state goes from released to pressed). Keep the state in `static` locals. Test it with this input and print the sample indices where an event happens:

```c
const bool input[] = {
    0, 0, 1, 0, 1, 1, 1, 1, 1, 1,     /* 0-9   press with one bounce   */
    0, 1, 1, 1, 1, 1,                 /* 10-15 bounce while held       */
    0, 0, 0, 0, 0, 0,                 /* 16-21 release                 */
    1, 1, 1, 1, 1, 1,                 /* 22-27 second press            */
};
```

What is one weakness of keeping the state in `static` locals, compared with a module-level `static` and a `reset` function?

**Exercise 15.** Answer three conceptual questions:

- (a) `gamma.h` contains `static const uint8_t gamma_table[256] = { ... };` and is included by three `.c` files that each use the table. How much Flash does the table cost, and how do you make it cost 256 bytes?
- (b) `systick.c` contains `static volatile uint32_t s_ticks;`, incremented in `SysTick_Handler` and returned by `uint32_t systick_get_ms(void)`. Explain what `static` and `volatile` each contribute, and why a getter is better than a global variable.
- (c) Why should an `enum` type not be used for a member of a register-map struct, and why does `GpioMode m = 7;` compile without complaint?

## 10. Answer key and explanations

All code answers below were compiled and run with `gcc -std=c11 -Wall -Wextra -O2`. The Flash measurements were made with `arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os`.

### Basic level

**Answer 1.**

```text
s=2 a=2
s=4 a=2
s=6 a=2
```

`s` is `static`, created once and kept between calls. `a` is created fresh as 0 on every call.

**Answer 2.** Both files compile, because each one is compiled alone and `main.c` only sees a prototype for `helper`. The link fails:

```text
undefined reference to `helper'
```

`helper` is `static` in `util.c`, so its name is not visible outside that file. Fix 1 (usually right): `main.c` should only call the public `twice`; remove the `helper` prototype and call. Fix 2: if `helper` really should be public, remove `static` and put its prototype in a `util.h`.

**Answer 3.** Lines (1), (4), (5), and (6) fail. (2) and (3) compile.

- `p` points to `const` data: the data is locked, the pointer can move. (1) fails, (2) is fine.
- `q` is a `const` pointer: the pointer is locked, the data is writable. (3) is fine, (4) fails.
- `r` locks both, so (5) and (6) fail.

**Answer 4.** `EV_NONE = 0`, `EV_PRESS = 10`, `EV_RELEASE = 11`, `EV_HOLD = 20`, `EV_TIMEOUT = 21`. `EV_RELEASE + EV_TIMEOUT = 32`.

**Answer 5.** (a) and (b) are wrong; (c) and (d) are correct.

```c
red.on = true;       // (a) red is a struct: use '.'
p->on = false;       // (b) p is a pointer: use '->'
```

At the end `red.pin` is 14 (line (d) runs after (c)) and `red.on` is `false` (line (b) runs after (a)).

### Intermediate level

**Answer 6.**

```c
#include <stdio.h>
#include <stdint.h>

typedef enum {
    LIGHT_RED,
    LIGHT_GREEN,
    LIGHT_YELLOW,
    LIGHT_COUNT            /* not a state: the number of states */
} Light;

static const uint32_t duration_ms[LIGHT_COUNT] = {
    [LIGHT_RED]    = 5000,
    [LIGHT_GREEN]  = 4000,
    [LIGHT_YELLOW] = 1000,
};

Light light_next(Light current) {
    switch (current) {
    case LIGHT_RED:    return LIGHT_GREEN;
    case LIGHT_GREEN:  return LIGHT_YELLOW;
    case LIGHT_YELLOW: return LIGHT_RED;
    case LIGHT_COUNT:  break;
    }
    return LIGHT_RED;      /* invalid input: fall back to a safe state */
}

uint32_t light_duration_ms(Light l) {
    return (l < LIGHT_COUNT) ? duration_ms[l] : 0;
}

int main(void) {
    static const char *const names[LIGHT_COUNT] = { "RED", "GREEN", "YELLOW" };
    Light l = LIGHT_RED;
    for (int i = 0; i < 4; i++) {
        printf("%-6s %u ms\n", names[l], (unsigned)light_duration_ms(l));
        l = light_next(l);
    }
    return 0;
}
```

Output:

```text
RED    5000 ms
GREEN  4000 ms
YELLOW 1000 ms
RED    5000 ms
```

`[LIGHT_RED] = 5000` is a designated initializer for arrays: each value is tied to its enum name, so reordering the enum cannot silently mismatch the table. Listing `case LIGHT_COUNT:` keeps `-Wswitch` quiet while still handling every value.

**Answer 7.**

```c
#include <stdint.h>

static uint32_t s_next_id = 1;     /* file scope: shared by both functions, private to this file */

uint32_t make_id(void) {
    return s_next_id++;            /* return the current value, then advance */
}

void reset_ids(void) {
    s_next_id = 1;
}
```

Calling `make_id` three times gives `1 2 3`; after `reset_ids`, the next call gives `1`. A `static` local inside `make_id` would have block scope: `reset_ids` could not see it, so it could never reset it. Both functions need the variable, so it moves to file scope, and `static` keeps it private to the file.

**Answer 8.** The three bugs and their symptoms:

1. **No include guard.** `main.c` includes the header twice, so the compiler sees everything twice: `redefinition of 'retry_count'`, `conflicting types for 'Button'; have 'struct <anonymous>'`. Fix: wrap the header in `#ifndef CONFIG_H` / `#define CONFIG_H` / `#endif`.
2. **A variable defined in the header.** Once the guard is added, the compile errors disappear, but now both `main.o` and `button.o` contain `retry_count`, and the linker reports `multiple definition of 'retry_count'`. Fix: `extern int retry_count;` in the header, `int retry_count = 3;` in `button.c`.
3. **Struct passed by value.** Now everything builds and runs, but the program exits with 3 instead of 18, because `button_init` changes a copy. GCC hints at it: `parameter 'b' set but not used`. Fix: `void button_init(Button *b)` with `b->pin = 15;`, called as `button_init(&btn);`.

Fixed `config.h`:

```c
#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

extern int retry_count;              /* declaration only */

typedef struct {
    uint8_t pin;
} Button;

void button_init(Button *b);         /* pointer, so the caller's struct changes */

#endif /* CONFIG_H */
```

Fixed `button.c`:

```c
#include "config.h"

int retry_count = 3;                 /* the one and only definition */

void button_init(Button *b) {
    b->pin = 15;
}
```

And in `main.c`: `button_init(&btn);`. The exit code is now 18. Notice the order: the bugs show up one at a time (compiler, then linker, then wrong behavior), which is typical.

**Answer 9.**

```c
void stats_reset(Stats *s) {
    s->min   = INT16_MAX;      /* any real sample will be smaller */
    s->max   = INT16_MIN;      /* any real sample will be larger  */
    s->sum   = 0;
    s->count = 0;
}

void stats_add(Stats *s, int16_t sample) {
    if (sample < s->min) { s->min = sample; }
    if (sample > s->max) { s->max = sample; }
    s->sum += sample;
    s->count++;
}

int16_t stats_average(const Stats *s) {
    if (s->count == 0) {
        return 0;
    }
    return (int16_t)(s->sum / s->count);
}
```

Output for `20, 25, -5, 40`: `min=-5 max=40 avg=20 count=4`. Starting `min` at the largest possible value and `max` at the smallest guarantees the first sample replaces both. Starting them at 0 would be wrong for a series that is all positive (min stays 0) or all negative (max stays 0). `sum` is `int32_t` so that many `int16_t` samples cannot overflow it.

**Answer 10.**

(a) Every member is 4 bytes, so the offset is 4 times the index. `PSC` is the 11th member (index 10): `0x28`. `ARR` is `0x2C`. `&TIM2->ARR` is `0x4000002C`. These match the spreadsheet.

(b)

```c
_Static_assert(offsetof(TIM_TypeDef, SR)  == 0x10, "SR offset");
_Static_assert(offsetof(TIM_TypeDef, CNT) == 0x24, "CNT offset");
_Static_assert(offsetof(TIM_TypeDef, PSC) == 0x28, "PSC offset");
_Static_assert(offsetof(TIM_TypeDef, ARR) == 0x2C, "ARR offset");
```

(c) Every member after `CR2` shifts down by 4 bytes. `PSC` ends up at offset `0x24`, which is really `CNT`, so `TIM2->PSC = 15999;` writes the counter instead of the prescaler. The timer then runs at full speed and the LED blinks far too fast, with no compiler error. With the checks from (b), all four assertions fail at compile time instead.

**Answer 11.**

```c
#define PIN_RED    12U
#define PIN_GREEN  13U
#define PIN_BLUE   14U
#define RGB_ALL    ((1U << PIN_RED) | (1U << PIN_GREEN) | (1U << PIN_BLUE))

/* Which pins are ON for each color. Lives in flash because it is const. */
static const uint32_t color_mask[COLOR_COUNT] = {
    [COLOR_OFF]   = 0,
    [COLOR_RED]   = (1U << PIN_RED),
    [COLOR_GREEN] = (1U << PIN_GREEN),
    [COLOR_BLUE]  = (1U << PIN_BLUE),
    [COLOR_WHITE] = RGB_ALL,
};

uint32_t color_to_bsrr(Color c) {
    uint32_t on = (c < COLOR_COUNT) ? color_mask[c] : 0U;   /* invalid -> all off */
    uint32_t off = RGB_ALL & ~on;
    return on | (off << 16);         /* set the ON pins, reset the others */
}
```

Results:

```text
COLOR_OFF    -> 0x70000000
COLOR_RED    -> 0x60001000
COLOR_GREEN  -> 0x50002000
COLOR_BLUE   -> 0x30004000
COLOR_WHITE  -> 0x00007000
invalid (5)  -> 0x70000000
```

`COLOR_RED`: the set half is `0x1000` (bit 12), and the reset half is `0x6000 << 16 = 0x60000000` (bits 29 and 30, which reset pins 13 and 14). One write to `GPIOB->BSRR` changes all three pins at the same instant, so there is never a moment where two colors are on together. For a common-anode LED, swap the two halves.

### Advanced level

**Answer 12.**

`avg.h`:

```c
#ifndef AVG_H
#define AVG_H

#include <stdint.h>

/* Moving average of the last AVG_SIZE samples. */
#define AVG_SIZE 4U

void     avg_reset(void);
void     avg_add(uint16_t sample);
uint16_t avg_get(void);

#endif /* AVG_H */
```

`avg.c`:

```c
#include "avg.h"

/* Private state: no other file can touch these. */
static uint16_t s_samples[AVG_SIZE];
static uint8_t  s_pos = 0;
static uint8_t  s_filled = 0;

void avg_reset(void) {
    s_pos = 0;
    s_filled = 0;
}

void avg_add(uint16_t sample) {
    s_samples[s_pos] = sample;
    s_pos = (uint8_t)((s_pos + 1U) % AVG_SIZE);
    if (s_filled < AVG_SIZE) {
        s_filled++;
    }
}

uint16_t avg_get(void) {
    if (s_filled == 0) {
        return 0;
    }
    uint32_t sum = 0;
    for (uint8_t i = 0; i < s_filled; i++) {
        sum += s_samples[i];
    }
    return (uint16_t)(sum / s_filled);
}
```

`main.c`:

```c
#include <stdio.h>
#include "avg.h"

int main(void) {
    avg_add(10);
    avg_add(20);
    printf("%u\n", avg_get());     /* 15 */
    avg_add(30);
    avg_add(40);
    avg_add(50);
    printf("%u\n", avg_get());     /* 35 */
    avg_reset();
    printf("%u\n", avg_get());     /* 0 */
    return 0;
}
```

Decisions worth noticing: `AVG_SIZE` goes in the header because a user might want to know it; the three state variables become `static` and get an `s_` prefix; `avg_reset` does not need to clear the array, because `s_filled = 0` already means "no valid samples". After 5 samples the buffer holds 50, 20, 30, 40 (the 50 overwrote the 10), so the average is 35.

**Answer 13.**

`led.h`:

```c
#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"

typedef struct {
    GPIO_TypeDef *port;
    uint8_t       pin;
    bool          active_low;    /* true for a common-anode RGB LED */
} Led;

void led_init(const Led *led);
void led_set(const Led *led, bool on);

#endif /* LED_H */
```

`led.c`:

```c
#include "led.h"

void led_init(const Led *led) {
    gpio_set_mode(led->port, led->pin, GPIO_MODE_OUTPUT);
    led_set(led, false);                     /* start in a known state: off */
}

void led_set(const Led *led, bool on) {
    bool level_high = (on != led->active_low);   /* flip the logic for active-low */
    gpio_write(led->port, led->pin, level_high);
}
```

Test results on the fake port (with `sim_apply_bsrr` after each write):

```text
after init : ODR = 0x2000      (pin 12 low = off, pin 13 high = off for active-low)
both on    : ODR = 0x1000      (pin 12 high = on, pin 13 low = on for active-low)
MODER      = 0x05000000        (pins 12 and 13 are outputs)
```

`on != active_low` is an XOR on booleans: when `active_low` is false the level equals `on`, and when it is true the level is inverted. The `Led` parameter is `const Led *` because neither function changes the description of the LED; they only change the hardware it points to. This is the "driver handle" pattern you will extend with function pointers in Week 4.

**Answer 14.**

```c
#include <stdbool.h>
#include <stdint.h>

#define DEBOUNCE_SAMPLES 5U

/* Call every 1 ms. raw_pressed: true while the contact is closed.
 * Returns true exactly once per real press. */
bool button_pressed_event(bool raw_pressed) {
    static bool    stable = false;   /* debounced state, survives between calls */
    static uint8_t count  = 0;       /* how many samples in a row disagree with stable */

    if (raw_pressed == stable) {
        count = 0;                   /* agrees: any bounce run is over */
        return false;
    }

    count++;
    if (count < DEBOUNCE_SAMPLES) {
        return false;                /* not convinced yet */
    }

    count = 0;
    stable = raw_pressed;            /* accept the new state */
    return stable;                   /* event only on released -> pressed */
}
```

Output for the test input:

```text
event at sample 8
event at sample 26
```

Sample 2 is a bounce (it is followed by a 0), so the counter restarts. Samples 4 to 8 are five 1s in a row, so the press is accepted at sample 8. The single 0 at sample 10 is a bounce while held and is ignored. Samples 16 to 20 accept the release (no event, because only presses are reported). Samples 22 to 26 accept the second press.

The weakness of `static` locals: nothing outside the function can reset or inspect the state. A unit test that runs several scenarios cannot start each one from a clean state, and you cannot have two buttons, because they would share the same `stable` and `count`. Moving the state into a struct (`typedef struct { bool stable; uint8_t count; } Debouncer;`) and passing a pointer solves both problems. That is the same step as Exercise 13.

On the board, call this from the 1 ms SysTick interrupt or from a loop paced by `systick_get_ms()`, and pass `(GPIOB->IDR & (1U << 15)) == 0` as `raw_pressed`.

**Answer 15.**

- (a) `static` gives each `.c` file that includes the header its own private copy of the table. With three files using it, the build contains three copies. Measured with `arm-none-eabi-gcc -Os`: 804 bytes of Flash for the three object files, compared with 292 bytes with a single copy. Fix: in the header write `extern const uint8_t gamma_table[256];`, and put the one definition `const uint8_t gamma_table[256] = { ... };` in `gamma.c`.
- (b) `static` makes `s_ticks` private to `systick.c` (internal linkage), so no other file can write it; its lifetime is the whole program anyway, because it is at file scope. `volatile` is needed because the value changes inside an interrupt handler, outside the normal flow of whatever code is reading it. Without it, a loop like `while (systick_get_ms() - start < 500)` can be optimized into reading the value once and spinning forever, typically only in a release build. The getter keeps the variable read-only for everyone else, gives one place to add protection later (for example, if the counter becomes 64-bit and needs interrupts briefly disabled to be read safely), and lets you replace the implementation without touching the callers.
- (c) The C standard leaves the size of an enum type to the compiler. GCC on the PC uses 4 bytes, but `arm-none-eabi-gcc` uses short enums by default, so an enum with small values is only 1 byte on the STM32. A 1-byte member in a register map would put the next register at the wrong offset (and make the access itself the wrong width). Register maps need exact sizes, so they use `uint32_t`. `GpioMode m = 7;` compiles because in C an enum is just an integer type: the compiler does not check that the value is one of the listed constants. Validate values that come from outside before using them.
