#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace libclient {

std::vector<int64_t> private_key_gen(const std::string& password, uint16_t key_size, int64_t mod);

std::vector<int64_t> shared_key_gen(const std::vector<int64_t>& private_key, uint16_t key_size, int64_t mod);

}  // namespace libclient