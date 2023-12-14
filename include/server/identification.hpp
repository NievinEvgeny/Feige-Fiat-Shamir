#pragma once
#include <sys/types.h>

namespace libserver {

struct pthread_data
{
    int* users_count;
    int* client_sockfd;
    pthread_mutex_t* mutexcount;
};

void protocol_end(libserver::pthread_data* thread_data);

void* identification(void* thread_args);

}  // namespace libserver