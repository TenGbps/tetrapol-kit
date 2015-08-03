#pragma once

#include <stdint.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))
#define SIZEOF(s, i) (sizeof(((s*)(NULL))->i))

char *sprint_hex(char *str, const uint8_t *bytes, int n);

