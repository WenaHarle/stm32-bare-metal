#include "util.h"

static int helper(int x) { return x * 2; }
int twice(int x) { return helper(x); }
