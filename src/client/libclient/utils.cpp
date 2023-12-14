#include <client/utils.hpp>
#include <cmath>
#include <random>
#include <cstdint>

namespace libclient {

static bool is_prime(int64_t prime)
{
    if (prime <= 1)
    {
        return false;
    }

    auto b = static_cast<int64_t>(std::sqrt(prime));

    for (int64_t i = 2; i <= b; ++i)
    {
        if ((prime % i) == 0)
        {
            return false;
        }
    }

    return true;
}

uint8_t gen_bits()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint16_t> bits_gen_range(0, UINT8_MAX);
    return bits_gen_range(mt);
}

uint32_t gen_mod()
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint32_t> prime_gen_range(UINT8_MAX, UINT16_MAX);

    uint16_t mod_part_P = 0;
    uint16_t mod_part_Q = 0;

    do
    {
        mod_part_P = prime_gen_range(mt);
    } while (!libclient::is_prime(mod_part_P));

    do
    {
        mod_part_Q = prime_gen_range(mt);
    } while (!libclient::is_prime(mod_part_Q) || (mod_part_Q == mod_part_P));

    return mod_part_P * mod_part_Q;
}

}  // namespace libclient