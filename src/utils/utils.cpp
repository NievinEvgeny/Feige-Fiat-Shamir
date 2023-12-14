#include <utils/utils.hpp>
#include <cmath>
#include <random>
#include <vector>
#include <cstdint>

namespace utils {

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
    } while (!utils::is_prime(mod_part_P));

    do
    {
        mod_part_Q = prime_gen_range(mt);
    } while (!utils::is_prime(mod_part_Q) || (mod_part_Q == mod_part_P));

    return mod_part_P * mod_part_Q;
}

int64_t mod(int64_t value, int64_t mod)
{
    int64_t m = value % mod;
    mod &= m >> std::numeric_limits<int64_t>::digits;
    return m + mod;
}

int64_t pow_mod(int64_t base, int64_t exp, int64_t mod)
{
    {
        int64_t result = 1;
        base %= mod;
        while (exp)
        {
            if (exp & 1)
            {
                result = (result * base) % mod;
            }
            base = (base * base) % mod;
            exp >>= 1;
        }
        return result;
    }
}

std::vector<int64_t> extended_gcd(int64_t first, int64_t second)
{
    if (first < second)
    {
        std::swap(first, second);
    }

    std::vector<int64_t> u{first, 1, 0};
    std::vector<int64_t> v{second, 0, 1};

    while (v.front() != 0)
    {
        int64_t q = u.front() / v.front();
        std::vector<int64_t> t{u.front() % v.front(), u.at(1) - q * v.at(1), u.back() - q * v.back()};
        u = std::move(v);
        v = std::move(t);
    }

    return u;
}

int64_t gen_rand(int64_t mod)
{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int64_t> rand_gen_range(1, mod - 1);
    return rand_gen_range(mt);
}

}  // namespace utils