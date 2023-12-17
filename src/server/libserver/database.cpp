#include <server/database.hpp>
#include <server/authentication.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

namespace libserver {

void database_put_shared_key(
    pthread_data* thread_data,
    const std::string& database_name,
    const std::string& login,
    uint8_t login_size,
    const std::vector<int64_t>& shared_key,
    int64_t shared_key_size)
{
    std::fstream database(database_name, std::ios::binary | std::ios::in | std::ios::out);
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

    database.close();
}

void database_get_shared_key(
    pthread_data* thread_data,
    const std::string& database_name,
    const std::string& login,
    std::vector<int64_t>& shared_key,
    int64_t shared_key_size)
{
    std::ifstream database(database_name, std::ios::binary);
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

    database.close();
}

}  // namespace libserver
