#ifndef bits_h

#include <stdint.h>

typedef uint32_t Bits_t[8];

void bits_set(Bits_t bs, const uint32_t b);
uint32_t bits_get(const Bits_t bs, const uint32_t b);
void bits_or(Bits_t bs1, const Bits_t bs2);
uint32_t bits_count(const Bits_t bs);
void bits_clear(Bits_t bs);


#define bits_h
#endif
