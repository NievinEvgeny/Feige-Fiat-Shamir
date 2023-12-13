#include <client/connection.hpp>
#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

int test_send(int sockfd)
{
    constexpr int data = 100;

    if (send(sockfd, &data, sizeof(int), 0) == -1)
    {
        std::cerr << "Can't send to server\n";
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    cxxopts::Options options("cryptography");

    // clang-format off
    options.add_options()
        ("i,ip", "server ip", cxxopts::value<std::string>()->default_value("localhost"))
        ("p,port", "server port", cxxopts::value<std::string>()->default_value("5001"));
    // clang-format on

    try
    {
        const auto parse_cmd_line = options.parse(argc, argv);

        if (parse_cmd_line.count("help"))
        {
            std::cout << options.help() << '\n';
            return 0;
        }

        int sockfd = libclient::connect_to_server(
            parse_cmd_line["ip"].as<std::string>(), parse_cmd_line["port"].as<std::string>());

        if (sockfd == -1)
        {
            return -1;
        }

        if (test_send(sockfd))
        {
            close(sockfd);
            std::cerr << "Can't send to server\n";
            return -1;
        }

        close(sockfd);
        return 0;
    }
    catch (const cxxopts::exceptions::exception& msg)
    {
        std::cerr << msg.what() << '\n';
        return -1;
    }
}
