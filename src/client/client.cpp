#include <client/connection.hpp>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>

int test_send(int sockfd)
{
    constexpr int data = 100;

    while (true)
    {
        if (send(sockfd, &data, sizeof(int), 0) == -1)
        {
            std::cerr << "Can't send to server\n";
            return -1;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cerr << "Error arguments\nExample: " << argv[0] << " ip port\n";
        return -1;
    }

    int sockfd = libclient::connect_to_server(argv[1], atoi(argv[2]));

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