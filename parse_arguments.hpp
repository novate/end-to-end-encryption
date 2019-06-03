#ifndef PARSE_ARGUMENTS_H
#define PARSE_ARGUMENTS_H

#include <string>

struct Options {
    int num_options = 4;

    std::string ip; // dot seperated ip address
    std::string port;
    unsigned int num = 100; // number of connections. defaults to 100
    bool block = false; // block/nonblock. defaults to nonblock
    bool fork = false;  // fork/nofork. defaults to nonblock
};

Options parse_arguments(int argc, char **argv, bool is_client);

#endif // PARSE_ARGUMENTS_H
