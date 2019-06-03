#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>
#include <queue>
#include <set>
#include <algorithm>

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include "parse_arguments.hpp"

const int max_sendlen = 32768;
const int max_recvlen = 32768;
const int buffer_len = 100000;
const int time_str_length = 20;
const int timeout_seconds = 2;
const int timeout_microseconds = 0;
const int max_conn = 1000;
const int max_active_connections = 50;

const int stu_no = 1551713;

using namespace std;

#define STR_1           "StuNo"
#define STR_2           "pid"
#define STR_3           "TIME"
#define STR_4           "end"

// gracefully perror and return
#define stage_done(socket) {\
    (socket).stage++;\
    return(1); }

// gracefully perror and exit
inline void graceful(const char *s, int x) { perror(s); exit(x); }

// gracefully perror and return
#define graceful_return(s, x) {\
    perror((s));\
    return((x)); }

class Socket {
    public:
        Socket(int socketfd) 
        : socketfd(socketfd)
        {
        }

        int socketfd;
        bool has_been_active = false;
        int stage = 1;
        int bytes_processed = 0;

        int stuNo = 0;
        int pid = 0;
        int random = 0;
        char time_str[time_str_length] = {0};
        unsigned char client_string[buffer_len] = {0};
        //char client_string[buffer_len] = {0};

        bool operator< (const Socket &s) const { return socketfd < s.socketfd; }
};

int get_listener(const Options &opt);
    // socket(), set blocking/nonblocking, bind(), listen()
    // Precondition: the port field and block field of opt is properly set
    // Postcondition: an ipv4 socket that is initialized, bound, and set to listening to all interfaces is returned

int server_bind_port(int listener, int listen_port);
    // this function binds socket listener to listen_port on all interfaces
    // Precondition: listener is a valid ipv4 socket and listen_port is not used
    // Postconfition: socket listener is bound to listen_port on all interfaces

void loop_server_nofork(int listener, const Options &opt);
    // must be non-blocking otherwise simultaneous connections can't be handled

int remove_dead_connections(fd_set &master, int &fdmax, set<Socket> &set_data_socket, queue<Socket> *socket_q);
    // remove connections that has been unresponsive in a certain period of time
    // precondition: master is a set of sockets the connections to be removed from

int server_accept_client(int listener, bool block, fd_set *master, int *fdmax, set<Socket> *set_data_socket, queue<Socket> *socket_q);

int server_communicate_new(Socket &socket);
// function: 
//      1. error handling: directly close socket
//      2. good result: don't close socket
// precondition: Socket socket, after readability/writability check
// postcondition: result code
    // remember to handle partial sends here
    // return 1: stage done
    // return 0: no error
    // return -1: select error
    // return -2: time up
    // return -3: peer offline
    // return -4: not permitted to send
    // return -5: ready_to_send error
    // return -6: send error
    // return -7: message sent is of wrong quantity of byte
    // return -8: not permitted to recv
    // return -9: ready_to_recv error
    // return -10: not received exact designated quantity of bytes
    // return -11: write_file error
    // return -12: not received correct string
    // return -13: stage number beyond index

void fill_up_sets(fd_set &master, int &fdmax, set<Socket> &set_data_socket, queue<Socket> &socket_q);

void *get_in_addr(struct sockaddr *sa);

int send_thing_new(Socket &socket, const char *str, const int send_len);
// function: 
//      1. send things in str, with length send_len.
//      2. if all bytes are processed, clear socket.bytes_processed, and return 1
// precondition: Socket socket, after readability/writability check
// postcondition: result code
    // remember to handle partial sends here
    // return 1: all bytes processed
    // return 0: no error, but not all bytes processed
    // return -3: peer offline
    // return -6: send error
    // return -7: message sent is of wrong quantity of byte

int recv_thing_new(Socket &socket, char *buffer, const int recv_len);
// function: 
//      1. recv things to buffer, with length recv_len.
//      2. if all bytes are processed, clear socket.bytes_processed, and return 1
// precondition: Socket socket, after readability/writability check
// postcondition: result code
    // remember to handle partial sends here
    // return 1: all bytes processed
    // return 0: no error, but not all bytes processed
    // return -3: peer offline
    // return -10: not received exact designated quantity of bytes

int write_file_new(const char *str_filename, Socket &socket);

int client_nofork(const Options &opt);
    // must be non-blocking 

int create_connection(const Options &opt);

int client_communicate_new(Socket &socket, const Options &opt);
// function: 
//      1. error handling: directly close socket
//      2. good result: don't close socket
// precondition: Socket socket, after readability/writability check
// postcondition: result code
    // remember to handle partial sends here
    // return 1: stage done
    // return 0: no error
    // return -1: select error
    // return -2: time up
    // return -3: peer offline
    // return -4: not permitted to send
    // return -5: ready_to_send error
    // return -6: send error
    // return -7: message sent is of wrong quantity of byte
    // return -8: not permitted to recv
    // return -9: ready_to_recv error
    // return -10: not received exact designated quantity of bytes
    // return -11: write_file error
    // return -12: not received correct string
    // return -13: stage number beyond index

bool same_string(const char *str1, const char *str2, const int cmp_len);
    // compare strings from first byte to designated length

int str_current_time(char *time_str);
    // format: yyyy-mm-dd hh:mm:ss, 19 bytes

int create_random_str(const int length, unsigned char *random_string);
    // create random string with designated length

int parse_str(const char *str);
    // parse "str*****" into a 5-digit number

int loop_server_fork(int listener, const Options &opt);
    // can be either blocking or non-blocking

int check_child();     
    //function:
    //      wait() child process if and only if receive SIGCHLD

int loop_client_fork(const Options &opt);

int client_reconnected(const Options &opt);
//function:
//      reconnect
// return:
//      0   reconnection failed 
//      1   reconnection succeed