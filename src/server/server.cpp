#include <server/connection.hpp>
#include <cxxopts.hpp>
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

    if (recv(*thread_data->client_sockfd, &recv_data, sizeof(int), 0) <= 0)
    {
        std::cerr << "User disconnected\n";
    }

    if (recv_data == expected_data)
    {
        std::cout << *thread_data->client_sockfd << " done\n";
    }

    pthread_mutex_lock(thread_data->mutexcount);
    (*thread_data->users_count)--;
    pthread_mutex_unlock(thread_data->mutexcount);

    close(*thread_data->client_sockfd);

    delete thread_data->client_sockfd;
    delete thread_data;

    pthread_exit(nullptr);
}

int main(int argc, char* argv[])
{
    cxxopts::Options options("cryptography");

    options.add_options()("p,port", "server port", cxxopts::value<uint16_t>()->default_value("5001"));

    try
    {
        const auto parse_cmd_line = options.parse(argc, argv);

        if (parse_cmd_line.count("help"))
        {
            std::cout << options.help() << '\n';
            return 0;
        }

        int users_count = 0;
        pthread_mutex_t mutexcount;

        int listener_sockfd = libserver::setup_listener(parse_cmd_line["port"].as<uint16_t>());
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

                    pthread_mutex_lock(&mutexcount);
                    users_count--;
                    pthread_mutex_unlock(&mutexcount);

                    close(*client_sockfd);

                    delete client_sockfd;
                    delete data;

                    pthread_exit(nullptr);
                }
            }
        }
    }
    catch (const cxxopts::exceptions::exception& msg)
    {
        std::cerr << msg.what() << '\n';
        return -1;
    }
}