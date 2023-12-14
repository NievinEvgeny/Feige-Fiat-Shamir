#pragma once
#include <cstdint>
#include <vector>

namespace libclient {

uint8_t gen_bits();

uint32_t gen_mod();

int64_t mod(int64_t value, int64_t mod);

int64_t pow_mod(int64_t base, int64_t exp, int64_t mod);

std::vector<int64_t> extended_gcd(int64_t first, int64_t second);

}  // namespace libclient