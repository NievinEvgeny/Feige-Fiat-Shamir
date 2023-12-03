#include <server/connection.hpp>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/types.h>

namespace libserver {

int setup_listener(int port)
{
    int sockfd = 0;

    struct sockaddr_in serv_addr
    {
    };

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        std::cerr << "Can't open listener socket\n";
        pthread_exit(nullptr);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr)) < 0)
    {
        std::cerr << "Can't bind listener socket\n";
        pthread_exit(nullptr);
    }

    return sockfd;
}

void get_client(int listener_sockfd, int* client_sockfd, int* users_count, pthread_mutex_t& mutexcount)
{
    socklen_t client_len = 0;

    struct sockaddr_in client_addr
    {
    };

    listen(listener_sockfd, MAX_USERS - *users_count);

    memset(&client_addr, 0, sizeof(client_addr));

    client_len = sizeof(client_addr);

    *client_sockfd
        = accept4(listener_sockfd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len, SOCK_CLOEXEC);

    if (*client_sockfd < 0)
    {
        std::cerr << "Can't accept connection from a client\n";
        pthread_exit(nullptr);
    }

    pthread_mutex_lock(&mutexcount);
    (*users_count)++;
    pthread_mutex_unlock(&mutexcount);
}

}  // namespace libserver