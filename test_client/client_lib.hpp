#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include "shared_library.hpp"
#include "client_communicate.hpp"

const int kNumProcess = 100;

int loop_client_fork(const Options &opt, int begin_dev_id, int n_devid);
int create_connection(const Options &opt);

#endif // CLIENT_LIB_H
