#ifndef COMMUNICATE_H
#define COMMUNICATE_H

#include "shared_library.hpp"

class Socket {
//     public:
//         Socket(int socketfd)
//         : socketfd(socketfd)
//         {
//         }

//         int socketfd;
//         bool has_been_active = false;
//         int stage = 1;
//         int bytes_processed = 0;

//         int stuNo = 0;
//         int pid = 0;
//         int random = 0;
//         char time_str[time_str_length] = {0};
//         unsigned char client_string[buffer_len] = {0};
//         //char client_string[buffer_len] = {0};

//         bool operator< (const Socket &s) const { return socketfd < s.socketfd; }
};

int client_communicate(int socketfd, Options opt);

int server_communicate(int socketfd, Options opt);

#endif // COMMUNICATE_H