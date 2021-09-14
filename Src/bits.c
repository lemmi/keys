#include <stdint.h>
#include <string.h>

#include "bits.h"

void bits_set(Bits_t bs, const uint32_t b) {
	if (b >= 8u * 32u) {
		return; // error
	}

	uint32_t word = b / 32u;
	uint32_t off  = b % 32u;
	uint32_t mask = 1u << off;

	bs[word] |= mask;
}

uint32_t bits_get(const Bits_t bs, const uint32_t b) {
	if (b >= 8u * 32u) {
		return 0u; // error
	}

	uint32_t word = b / 32u;
	uint32_t off  = b % 32u;
	uint32_t mask = 1u << off;

	return (bs[word] & mask) > 0u;
}

static inline uint32_t words(const uint32_t l) { return (l + 31u) / 32u; }

void bits_or(Bits_t bs1, const Bits_t bs2) {
	uint32_t i;
	for (i = 0u; i < 8u; i++) {
		bs1[i] |= bs2[i];
	}
}

static inline uint32_t count_word_bits(const uint32_t x) {
	uint32_t c;
	uint32_t v;

	v = x;

	for (c = 0u; v != 0u; c++) {
		v &= v - 1u;
	}

	return c;
}

uint32_t bits_count(const Bits_t bs) {
	uint32_t sum;
	uint32_t i;

	sum = 0u;
	for (i = 0u; i < 8u; i++) {
		sum += count_word_bits(bs[i]);
	}
	return sum;
}

void bits_clear(Bits_t bs) { memset(bs, 0, 32ul); }
