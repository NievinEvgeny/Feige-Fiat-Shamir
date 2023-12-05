#include <server/connection.hpp>
#include <cstring>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>

struct pthread_data
{
    int* users_count;
    int* client_sockfd;
    pthread_mutex_t* mutexcount;
};

void* test(void* thread_args)
{
    constexpr int expected_data = 100;

    auto* thread_data = reinterpret_cast<pthread_data*>(thread_args);
    int recv_data = 0;

    while (true)
    {
        if (recv(*thread_data->client_sockfd, &recv_data, sizeof(int), 0) <= 0)
        {
            std::cerr << "User disconnected\n";

            pthread_mutex_lock(thread_data->mutexcount);
            (*thread_data->users_count)--;
            pthread_mutex_unlock(thread_data->mutexcount);

            delete thread_data->client_sockfd;
            delete thread_data;
            pthread_exit(nullptr);
        }

        if (recv_data == expected_data)
        {
            std::cout << *thread_data->client_sockfd << " done\n";
        }
    }
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

            auto* data = new pthread_data{&users_count, client_sockfd, &mutexcount};

            if (pthread_create(&new_thread, nullptr, test, reinterpret_cast<void*>(data)))
            {
                std::cerr << "Thread creation failed\n";

                close(listener_sockfd);
                pthread_mutex_destroy(&mutexcount);
                pthread_exit(nullptr);
            }
        }
    }

    close(listener_sockfd);
    pthread_mutex_destroy(&mutexcount);
    pthread_exit(nullptr);
}