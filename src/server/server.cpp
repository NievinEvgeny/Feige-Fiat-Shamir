#include <server/connection.hpp>
#include <cxxopts.hpp>
#include <string>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <vector>

struct pthread_data
{
    int* users_count;
    int* client_sockfd;
    pthread_mutex_t* mutexcount;
};

void test_end(pthread_data* thread_data)
{
    pthread_mutex_lock(thread_data->mutexcount);
    (*thread_data->users_count)--;
    pthread_mutex_unlock(thread_data->mutexcount);

    close(*thread_data->client_sockfd);

    delete thread_data->client_sockfd;
    delete thread_data;
}

void* test(void* thread_args)
{
    auto* thread_data = reinterpret_cast<pthread_data*>(thread_args);

    constexpr uint16_t max_login_size = 255;
    constexpr uint16_t key_size = 8;
    // constexpr uint8_t rounds = 5;

    if (send(*thread_data->client_sockfd, &key_size, sizeof(key_size), 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        test_end(thread_data);
        pthread_exit(nullptr);
    }

    bool signup = false;

    if (recv(*thread_data->client_sockfd, &signup, sizeof(signup), 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        test_end(thread_data);
        pthread_exit(nullptr);
    }

    std::string login(max_login_size, '\0');

    if (recv(*thread_data->client_sockfd, login.data(), max_login_size, 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        test_end(thread_data);
        pthread_exit(nullptr);
    }

    if (signup)
    {
        std::vector<uint32_t> client_shared_key;
        client_shared_key.reserve(key_size + 1);
        client_shared_key.resize(key_size + 1);

        if (recv(*thread_data->client_sockfd, client_shared_key.data(), (key_size + 1) * sizeof(uint32_t), 0) <= 0)
        {
            std::cerr << "User disconnected\n";
            test_end(thread_data);
            pthread_exit(nullptr);
        }
    }

    test_end(thread_data);
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
                    test_end(data);
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