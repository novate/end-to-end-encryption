#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include "shared_library.hpp"
#include "client_communicate.hpp"

#include <semaphore.h>
#include <unistd.h>

const int kNumConcurrency = 10;

int loop_client_fork_high_concurrency(const Options &opt, int begin_dev_id, int n_devid);
int loop_client_fork(const Options &opt, int begin_dev_id, int n_devid);
int create_connection(const Options &opt);

#endif // CLIENT_LIB_H
