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
