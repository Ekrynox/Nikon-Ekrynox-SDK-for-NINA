#pragma once
#include <stdint.h>

struct int128_ {
	int64_t low;
	int64_t high;
};
typedef struct int128_ int128;

struct uint128_ {
	uint64_t low;
	uint64_t high;
};
typedef struct uint128_ uint128;