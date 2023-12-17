#include <client/key_gen.hpp>
#include <client/connection.hpp>
#include <client/authentication.hpp>
#include <utils/utils.hpp>
#include <cxxopts.hpp>
#include <cstdint>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include <vector>

namespace libclient {

bool authentication(const cxxopts::ParseResult& parse_cmd_line)
{
    constexpr uint8_t max_login_size = 255;

    int sockfd = libclient::connect_to_server(
        parse_cmd_line["ip"].as<std::string>(), parse_cmd_line["port"].as<std::string>());

    if (sockfd == -1)
    {
        return -1;
    }

    const std::string login = parse_cmd_line["login"].as<std::string>();
    const uint8_t login_size = login.size();

    if (login_size > max_login_size)
    {
        std::cerr << "Login length must not exceed " << max_login_size << " characters";
        return -1;
    }

    bool signup = parse_cmd_line["signup"].as<bool>();

    uint16_t key_size = 0;
    uint8_t rounds = 0;

    if ((recv(sockfd, &key_size, sizeof(key_size), 0) <= 0) || (recv(sockfd, &rounds, sizeof(rounds), 0) <= 0)
        || (send(sockfd, &signup, sizeof(signup), 0) <= 0) || (send(sockfd, &login_size, sizeof(login_size), 0) <= 0)
        || send(sockfd, login.c_str(), login.size(), 0) <= 0)
    {
        return -1;
    }

    int64_t mod = 0;

    if (!signup)
    {
        if (recv(sockfd, &mod, sizeof(mod), 0) <= 0)
        {
            return -1;
        }
    }
    else
    {
        mod = utils::gen_mod();
    }

    std::vector<int64_t> private_key
        = libclient::private_key_gen(parse_cmd_line["password"].as<std::string>(), key_size, mod);

    if (signup)
    {
        std::vector<int64_t> shared_key = libclient::shared_key_gen(private_key, key_size, mod);

        if (send(sockfd, shared_key.data(), shared_key.size() * sizeof(int64_t), 0) <= 0)
        {
            return -1;
        }
    }

    for (uint8_t i = 0; i < rounds; i++)
    {
        int64_t rand_num = utils::gen_rand(mod);
        int64_t pow_rand_num = utils::pow_mod(rand_num, 2, mod);

        uint8_t server_bits = 0;

        if ((send(sockfd, &pow_rand_num, sizeof(pow_rand_num), 0) <= 0)
            || (recv(sockfd, &server_bits, sizeof(server_bits), 0) <= 0))
        {
            break;
        }

        int64_t masked_secret = 1;

        for (uint16_t i = 0; i < key_size; i++)
        {
            if (server_bits & (1 << i))
            {
                masked_secret = utils::mod(masked_secret * private_key.at(i), mod);
            }
        }

        masked_secret = utils::mod(masked_secret * rand_num, mod);

        if (send(sockfd, &masked_secret, sizeof(masked_secret), 0) <= 0)
        {
            break;
        }
    }

    bool result = false;

    if (recv(sockfd, &result, sizeof(result), 0) <= 0)
    {
        result = false;
    }

    result ? (std::cout << "You were approved\n") : (std::cout << "You weren't approved\n");

    close(sockfd);

    return false;
}

}  // namespace libclient