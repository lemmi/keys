#include <stdint.h>
#include <string.h>

#include "bits.h"

void bits_set(Bits_t bs, const uint32_t b) {
	if (b >= 8 * 32) {
		return; // error
	}

	uint32_t word = b / 32;
	uint32_t off  = b % 32;
	uint32_t mask = 1 << off;

	bs[word] |= mask;
}

uint32_t bits_get(const Bits_t bs, const uint32_t b) {
	if (b >= 8 * 32) {
		return 0; // error
	}

	uint32_t word = b / 32;
	uint32_t off  = b % 32;
	uint32_t mask = 1 << off;

	return (bs[word] & mask) > 0;
}

static inline uint32_t words(const uint32_t l) { return (l + 31) / 32; }

void bits_or(Bits_t bs1, const Bits_t bs2) {
	uint32_t i;
	for (i = 0; i < 8; i++) {
		bs1[i] |= bs2[i];
	}
}

static inline uint32_t count_word_bits(const uint32_t x) {
	uint32_t c;
	uint32_t v;

	v = x;

	for (c = 0; v != 0; c++) {
		v &= v - 1;
	}

	return c;
}

uint32_t bits_count(const Bits_t bs) {
	uint32_t sum;
	uint32_t i;

	sum = 0;
	for (i = 0; i < 8; i++) {
		sum += count_word_bits(bs[i]);
	}
	return sum;
}

void bits_clear(Bits_t bs) { memset(bs, 0, 32); }
