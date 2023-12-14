#include <server/connection.hpp>
#include <server/identification.hpp>
#include <cxxopts.hpp>
#include <pthread.h>
#include <iostream>

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

                auto* data = new libserver::pthread_data{&users_count, client_sockfd, &mutexcount};

                if (pthread_create(&new_thread, nullptr, libserver::identification, reinterpret_cast<void*>(data)))
                {
                    std::cerr << "Thread creation failed\n";
                    protocol_end(data);
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