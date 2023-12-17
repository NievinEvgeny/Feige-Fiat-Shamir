#include <client/authentication.hpp>
#include <cxxopts.hpp>
#include <string>
#include <iostream>

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
        const auto parse_cmd_line = options.parse(argc, argv);

        if (parse_cmd_line.count("help"))
        {
            std::cout << options.help() << '\n';
            return 0;
        }

        return libclient::authentication(parse_cmd_line);
    }
    catch (const cxxopts::exceptions::exception& msg)
    {
        std::cerr << msg.what() << '\n';
        return -1;
    }
}
