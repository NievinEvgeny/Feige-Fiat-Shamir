#pragma once
#include <sys/types.h>

constexpr int8_t MAX_USERS = 10;

namespace libserver {

int setup_listener(int port);

void get_client(int listener_sockfd, int* client_sockfd, int* users_count, pthread_mutex_t& mutexcount);

}  // namespace libserver