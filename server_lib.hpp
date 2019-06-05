#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include "shared_library.hpp"

int get_listener(const Options &opt);
int loop_server(int listener, const Options &opt);

#endif // SERVER_LIB_H
