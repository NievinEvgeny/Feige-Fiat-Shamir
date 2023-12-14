#include <client/connection.hpp>
#include <client/utils.hpp>
#include <PicoSHA2/picosha2.h>
#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <vector>
#include <cstdlib>

int main(int argc, char* argv[])
{
    cxxopts::Options options("cryptography");

    // clang-format off
    options.add_options()
        ("ip", "server ip", cxxopts::value<std::string>()->default_value("localhost"))
        ("port", "server port", cxxopts::value<std::string>()->default_value("5001"))
        ("signup", "registration")
        ("l,login", "client login", cxxopts::value<std::string>())
        ("p,password", "client password", cxxopts::value<std::string>());
    // clang-format on

    try
    {
        constexpr uint8_t max_login_size = 255;

        const auto parse_cmd_line = options.parse(argc, argv);

        if (parse_cmd_line.count("help"))
        {
            std::cout << options.help() << '\n';
            return 0;
        }

        if (parse_cmd_line["login"].as<std::string>().size() > max_login_size)
        {
            std::cerr << "Login length must not exceed " << max_login_size << " characters";
            return -1;
        }

        int sockfd = libclient::connect_to_server(
            parse_cmd_line["ip"].as<std::string>(), parse_cmd_line["port"].as<std::string>());

        if (sockfd == -1)
        {
            return -1;
        }

        // start protocol

        uint16_t key_size = 0;

        if (recv(sockfd, &key_size, sizeof(key_size), 0) <= 0)
        {
            return -1;
        }

        bool signup = parse_cmd_line["signup"].as<bool>();

        if (send(sockfd, &signup, sizeof(signup), 0) <= 0)
        {
            return -1;
        }

        const std::string login = parse_cmd_line["login"].as<std::string>();

        if (send(sockfd, login.c_str(), login.size(), 0) <= 0)
        {
            return -1;
        }

        uint32_t mod = libclient::gen_mod();
        uint8_t bits = libclient::gen_bits();

        const std::string password_hash = picosha2::hash256_hex_string(parse_cmd_line["password"].as<std::string>());
        std::vector<uint32_t> private_key;
        private_key.reserve(key_size);
        private_key.resize(key_size);

        std::memcpy(private_key.data(), password_hash.data(), key_size * sizeof(uint32_t));

        for (auto& val : private_key)
        {
            val = libclient::mod(val, mod);
        }

        if (signup)
        {
            std::vector<uint32_t> shared_key;
            shared_key.reserve(key_size + 1);

            for (uint16_t i = 0; i < key_size; i++)
            {
                int8_t sign = (bits & i) ? -1 : 1;

                shared_key.emplace_back(libclient::mod(
                    sign * libclient::extended_gcd(libclient::pow_mod(private_key.at(i), 2, mod), mod).back(), mod));
            }

            shared_key.emplace_back(mod);

            if (send(sockfd, shared_key.data(), shared_key.size() * sizeof(uint32_t), 0) <= 0)
            {
                return -1;
            }
        }

        // end protocol

        close(sockfd);

        return 0;
    }
    catch (const cxxopts::exceptions::exception& msg)
    {
        std::cerr << msg.what() << '\n';
        return -1;
    }
}
