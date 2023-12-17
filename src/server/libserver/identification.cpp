#include <server/identification.hpp>
#include <utils/utils.hpp>
#include <string>
#include <pthread.h>
#include <iostream>
#include <netinet/in.h>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <cmath>

namespace libserver {

void protocol_end(libserver::pthread_data* thread_data)
{
    pthread_mutex_lock(thread_data->mutexcount);
    (*thread_data->users_count)--;
    pthread_mutex_unlock(thread_data->mutexcount);

    close(*thread_data->client_sockfd);

    delete thread_data->client_sockfd;
    delete thread_data;
}

template <typename T>
void serv_send(pthread_data* thread_data, T* buf, std::size_t buf_size)
{
    if (send(*thread_data->client_sockfd, buf, buf_size, 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        protocol_end(thread_data);
        pthread_exit(nullptr);
    }
}

template <typename T>
void serv_recv(pthread_data* thread_data, T* buf, std::size_t buf_size)
{
    if (recv(*thread_data->client_sockfd, buf, buf_size, 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        protocol_end(thread_data);
        pthread_exit(nullptr);
    }
}

void* identification(void* thread_args)
{
    auto* thread_data = reinterpret_cast<pthread_data*>(thread_args);

    constexpr uint16_t key_size = 8;
    constexpr uint8_t rounds = 5;

    serv_send(thread_data, &key_size, sizeof(key_size));
    serv_send(thread_data, &rounds, sizeof(rounds));

    bool signup = false;
    uint8_t login_size = 0;

    serv_recv(thread_data, &signup, sizeof(signup));
    serv_recv(thread_data, &login_size, sizeof(login_size));

    std::string login(login_size, '\0');

    serv_recv(thread_data, login.data(), login_size);

    std::vector<int64_t> shared_key;
    shared_key.reserve(key_size + 1);
    shared_key.resize(key_size + 1);
    constexpr std::size_t shared_key_size = (key_size + 1) * sizeof(int64_t);

    if (signup)
    {
        serv_recv(thread_data, shared_key.data(), shared_key_size);

        std::fstream database("serverDB.csv", std::ios::binary | std::ios::app | std::ios::in | std::ios::out);
        if (!database.is_open())
        {
            throw std::runtime_error{"Can't open database"};
        }

        database.seekg(0);

        while (!database.fail())
        {
            uint8_t tmp_login_size = 0;

            database.read(reinterpret_cast<char*>(&tmp_login_size), sizeof(tmp_login_size));

            std::string tmp_login(tmp_login_size, '\0');

            database.read(tmp_login.data(), tmp_login_size);

            if (tmp_login == login)
            {
                std::cerr << "Login already used\n";
                protocol_end(thread_data);
                pthread_exit(nullptr);
            }

            database.seekg(shared_key_size, std::ios::cur);
        }

        database.clear();
        database.seekg(0, std::ios::end);

        pthread_mutex_lock(thread_data->mutexcount);

        database.write(reinterpret_cast<const char*>(&login_size), sizeof(login_size));
        database.write(login.c_str(), login_size);

        for (const auto& element : shared_key)
        {
            database.write(reinterpret_cast<const char*>(&element), sizeof(element));
        }

        pthread_mutex_unlock(thread_data->mutexcount);
    }
    else
    {
        std::ifstream database("serverDB.csv", std::ios::binary);
        if (!database.is_open())
        {
            throw std::runtime_error{"Can't open database"};
        }

        while (!database.fail())
        {
            uint8_t tmp_login_size = 0;

            database.read(reinterpret_cast<char*>(&tmp_login_size), sizeof(tmp_login_size));

            std::string tmp_login(tmp_login_size, '\0');

            database.read(tmp_login.data(), tmp_login_size);

            if (tmp_login == login)
            {
                database.read(reinterpret_cast<char*>(shared_key.data()), shared_key_size);
                break;
            }

            database.seekg(shared_key_size, std::ios::cur);
        }

        if (database.fail() || database.eof())
        {
            std::cerr << "Login not found\n";
            protocol_end(thread_data);
            pthread_exit(nullptr);
        }

        serv_send(thread_data, &shared_key.back(), sizeof(shared_key.back()));
    }

    bool protocol_result = true;

    for (uint8_t i = 0; i < rounds; i++)
    {
        int64_t rand_num = 0;

        serv_recv(thread_data, &rand_num, sizeof(rand_num));

        uint8_t rand_bits = utils::gen_bits();

        serv_send(thread_data, &rand_bits, sizeof(rand_bits));

        int64_t masked_secret = 0;

        serv_recv(thread_data, &masked_secret, sizeof(masked_secret));

        int64_t secret_check = 1;

        for (uint16_t i = 0; i < key_size; i++)
        {
            if (rand_bits & (1 << i))
            {
                secret_check = utils::mod(secret_check * shared_key.at(i), shared_key.back());
            }
        }

        secret_check = utils::mod(secret_check * rand_num, shared_key.back());

        if ((secret_check == 0) || (secret_check != std::abs(utils::pow_mod(masked_secret, 2, shared_key.back()))))
        {
            protocol_result = false;
            serv_send(thread_data, &protocol_result, sizeof(protocol_result));
            std::cerr << "Client not approved\n";

            protocol_end(thread_data);
            pthread_exit(nullptr);
        }
    }

    serv_send(thread_data, &protocol_result, sizeof(protocol_result));
    std::cout << "Client approved\n";

    protocol_end(thread_data);
    pthread_exit(nullptr);
}

}  // namespace libserver