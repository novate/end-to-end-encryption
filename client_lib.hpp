#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include "shared_library.hpp"

int loop_client_fork(const Options &opt, int n_devid);
int create_connection(const Options &opt);

#endif // CLIENT_LIB_H
