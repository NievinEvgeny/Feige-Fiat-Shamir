#pragma once
#include <server/authentication.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace libserver {

void database_put_shared_key(
    pthread_data* thread_data,
    const std::string& database_name,
    const std::string& login,
    uint8_t login_size,
    const std::vector<int64_t>& shared_key,
    int64_t shared_key_size);

void database_get_shared_key(
    pthread_data* thread_data,
    const std::string& database_name,
    const std::string& login,
    std::vector<int64_t>& shared_key,
    int64_t shared_key_size);

}  // namespace libserver