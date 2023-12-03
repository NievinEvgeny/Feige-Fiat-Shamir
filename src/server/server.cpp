#include <server/connection.hpp>
#include <cstring>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>

void* test(void* thread_data)
{
    constexpr int expected_data = 100;

    int* client_sockfd = reinterpret_cast<int*>(thread_data);
    int data = 0;

    while (true)
    {
        if (recv(*client_sockfd, &data, sizeof(int), 0) == -1)
        {
            std::cerr << "Can't recv from client\n";
            pthread_exit(nullptr);
        }

        if (data == expected_data)
        {
            std::cout << *client_sockfd << " done\n";
        }
    }

    close(*client_sockfd);
    delete client_sockfd;
    pthread_exit(nullptr);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Error arguments\nExample: " << argv[0] << " port\n";
        pthread_exit(nullptr);
    }

    int users_count = 0;

    pthread_mutex_t mutexcount;

    int listener_sockfd = libserver::setup_listener(atoi(argv[1]));
    pthread_mutex_init(&mutexcount, nullptr);

    while (true)
    {
        if (users_count < MAX_USERS)
        {
            int* client_sockfd = new int(0);

            libserver::get_client(listener_sockfd, client_sockfd, &users_count, mutexcount);

            pthread_t new_thread = 0;

            if (pthread_create(&new_thread, nullptr, test, reinterpret_cast<void*>(client_sockfd)))
            {
                std::cerr << "Thread creation failed\n";
                close(listener_sockfd);
                pthread_mutex_destroy(&mutexcount);
                pthread_exit(nullptr);
            }

            pthread_mutex_lock(&mutexcount);
            users_count--;
            pthread_mutex_unlock(&mutexcount);
        }
    }

    close(listener_sockfd);
    pthread_mutex_destroy(&mutexcount);
    pthread_exit(nullptr);
}