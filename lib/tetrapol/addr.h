#pragma once

#include <tetrapol/bit_utils.h>

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint8_t z;
    uint8_t y;
    uint16_t x;
} addr_t;

typedef struct {
    int len;
    addr_t addrs[];
} addr_list_t;

static inline bool addr_is_cgi_all_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 0 && addr->x == 0xfff;
    } else {
        return addr->y == 0 && addr->x == 0xfff;
    }
};

static inline bool addr_is_tti_all_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 7 && addr->x == 0xfff;
    } else {
        return addr->y == 7 && addr->x == 0xfff;
    }
}

static inline bool addr_is_tti_no_st(const addr_t *addr, bool z)
{
    if (z) {
        return addr->z == 0 && addr->y == 7 && addr->x == 0;
    } else {
        return addr->y == 7 && addr->x == 0;
    }
};

static inline bool addr_is_coi_all_st(const addr_t *addr)
{
    return addr->y == 1 && addr->x == 0;
    // x=0 for all stations? it is not a bug in specification?
};

static inline void addr_parse(addr_t *addr, const uint8_t *buf, int skip)
{
    addr->z = get_bits(1,  buf, 0 + skip);
    addr->y = get_bits(3,  buf, 1 + skip);
    addr->x = get_bits(12, buf, 4 + skip);
}

// size of buffer required for printing any address
#define ADDR_PRINT_BUF_SIZE (15)

/**
  Nice print of address addr into buffer buf.
  @return pointer to start of output buffer (buf)
  */
char* addr_print(char *buf, const addr_t *addr);

