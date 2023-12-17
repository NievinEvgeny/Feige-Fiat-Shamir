#include <server/authentication.hpp>
#include <server/database.hpp>
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
static void serv_send(pthread_data* thread_data, T* buf, std::size_t buf_size)
{
    if (send(*thread_data->client_sockfd, buf, buf_size, 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        protocol_end(thread_data);
        pthread_exit(nullptr);
    }
}

template <typename T>
static void serv_recv(pthread_data* thread_data, T* buf, std::size_t buf_size)
{
    if (recv(*thread_data->client_sockfd, buf, buf_size, 0) <= 0)
    {
        std::cerr << "User disconnected\n";
        protocol_end(thread_data);
        pthread_exit(nullptr);
    }
}

static bool check_single_round(pthread_data* thread_data, const std::vector<int64_t>& shared_key, uint16_t key_size)
{
    int64_t rand_num = 0;

    libserver::serv_recv(thread_data, &rand_num, sizeof(rand_num));

    uint8_t rand_bits = utils::gen_bits();

    libserver::serv_send(thread_data, &rand_bits, sizeof(rand_bits));

    int64_t masked_secret = 0;

    libserver::serv_recv(thread_data, &masked_secret, sizeof(masked_secret));

    int64_t secret_check = 1;

    for (uint16_t i = 0; i < key_size; i++)
    {
        if (rand_bits & (1 << i))
        {
            secret_check = utils::mod(secret_check * shared_key.at(i), shared_key.back());
        }
    }

    secret_check = utils::mod(secret_check * rand_num, shared_key.back());

    return ((secret_check == 0) || (secret_check != std::abs(utils::pow_mod(masked_secret, 2, shared_key.back()))));
}

void* authentication(void* thread_args)
{
    auto* thread_data = reinterpret_cast<pthread_data*>(thread_args);

    constexpr uint16_t key_size = 8;
    constexpr uint8_t rounds = 5;

    libserver::serv_send(thread_data, &key_size, sizeof(key_size));
    libserver::serv_send(thread_data, &rounds, sizeof(rounds));

    bool signup = false;
    uint8_t login_size = 0;

    libserver::serv_recv(thread_data, &signup, sizeof(signup));
    libserver::serv_recv(thread_data, &login_size, sizeof(login_size));

    std::string login(login_size, '\0');

    libserver::serv_recv(thread_data, login.data(), login_size);

    std::vector<int64_t> shared_key(key_size + 1);
    constexpr std::size_t shared_key_size = (key_size + 1) * sizeof(int64_t);

    const std::string database_name = "serverDB.csv";

    if (signup)
    {
        libserver::serv_recv(thread_data, shared_key.data(), shared_key_size);

        libserver::database_put_shared_key(thread_data, database_name, login, login_size, shared_key, shared_key_size);
    }
    else
    {
        libserver::database_get_shared_key(thread_data, database_name, login, shared_key, shared_key_size);

        libserver::serv_send(thread_data, &shared_key.back(), sizeof(shared_key.back()));
    }

    bool protocol_result = true;

    for (uint8_t i = 0; i < rounds; i++)
    {
        if (libserver::check_single_round(thread_data, shared_key, key_size))
        {
            protocol_result = false;
            libserver::serv_send(thread_data, &protocol_result, sizeof(protocol_result));
            std::cerr << "Client not approved\n";

            libserver::protocol_end(thread_data);
            pthread_exit(nullptr);
        }
    }

    libserver::serv_send(thread_data, &protocol_result, sizeof(protocol_result));
    std::cout << "Client approved\n";

    libserver::protocol_end(thread_data);
    pthread_exit(nullptr);
}

}  // namespace libserver