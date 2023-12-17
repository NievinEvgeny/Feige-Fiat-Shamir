#include <client/key_gen.hpp>
#include <utils/utils.hpp>
#include <PicoSHA2/picosha2.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

namespace libclient {

std::vector<int64_t> private_key_gen(const std::string& password, uint16_t key_size, int64_t mod)
{
    const std::string password_hash = picosha2::hash256_hex_string(password);
    std::vector<int64_t> private_key;
    private_key.reserve(key_size);
    private_key.resize(key_size);

    std::memcpy(private_key.data(), password_hash.data(), key_size * sizeof(int64_t));

    for (auto& val : private_key)
    {
        val = utils::mod(val, mod);
    }

    return private_key;
}

std::vector<int64_t> shared_key_gen(const std::vector<int64_t>& private_key, uint16_t key_size, int64_t mod)
{
    std::vector<int64_t> shared_key;
    shared_key.reserve(key_size + 1);

    for (uint16_t i = 0; i < key_size; i++)
    {
        shared_key.emplace_back(utils::pow_mod(private_key.at(i), 2, mod));
    }

    shared_key.emplace_back(mod);

    return shared_key;
}

}  // namespace libclient
