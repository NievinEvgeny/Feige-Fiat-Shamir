#include <client/connection.hpp>
#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

namespace libclient {

int connect_to_server(const std::string& hostname, const std::string& port)
{
    addrinfo hints{}, *result = nullptr;
    int sockfd = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname.data(), port.data(), &hints, &result) != 0)
    {
        std::cerr << "Can't get address info for host: " << hostname << "\n";
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        std::cerr << "Can't open socket for server\n";
        return -1;
    }

    if (connect(sockfd, result->ai_addr, result->ai_addrlen) != 0)
    {
        close(sockfd);
        std::cerr << "Can't connect to server\n";
        return -1;
    }

    freeaddrinfo(result);

    return sockfd;
}

}  // namespace libclient