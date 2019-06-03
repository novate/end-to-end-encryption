#include "shared_library.hpp"

using namespace std;

int get_listener(const Options &opt) {
    int listen_port = stoi(opt.port);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener < 0)
        graceful("socket", 1);

    // lose the pesky "address already in use" error message
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // set nonblocking listener
    if (!opt.block) {
        fcntl(listener, F_SETFL, O_NONBLOCK);
    }

    // bind
    if (server_bind_port(listener, listen_port) == -1)
        graceful("bind", 2);

    std::cout << "Listening on port " << listen_port << " on all interfaces...\n";

    // set listening
    listen(listener, 1000);

    return listener;
}

int server_bind_port(int listener, int listen_port) {
    struct sockaddr_in listen_addr;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(listen_port);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    return bind(listener, (sockaddr*) &listen_addr, sizeof(sockaddr));
}

void loop_server_nofork(int listener, const Options &opt) {
    queue<Socket> socket_q;
    set<Socket> set_data_socket;

    fd_set master, readfds, writefds;      // master file descriptor list
    FD_ZERO(&master);
    FD_SET(listener, &master);
    Socket listener_socket (listener);
    listener_socket.has_been_active = true;
    set_data_socket.insert(listener_socket);
    int fdmax = listener; // maximum file descriptor number 
    timeval tv {timeout_seconds, timeout_microseconds}; // set a 2 second client timeout

    for (;;) {
        //sleep(1);
        cout << "begin of loop\n";
        cout << "set_data_socket.size(): " << set_data_socket.size() << endl;
        readfds = master; // copy at the last minutes
        writefds = master; // copy at the last minutes
        FD_CLR(listener, &writefds); // avoid selecting writable listener
        cout << "before select\n";
        int rv = select(fdmax+1, &readfds, &writefds, NULL, &tv);
        cout << "DEBUG: rv\t" << rv << endl;
        switch (rv) {
            case -1:
                graceful("select in main loop", 5);
                break;
            case 0:
                // timeout, close sockets that haven't responded in an interval, exept for listener
                remove_dead_connections(master, fdmax, set_data_socket, &socket_q);
                tv = {timeout_seconds, timeout_microseconds}; // set a 2 second client timeout
                for (auto socket_it = set_data_socket.begin(); socket_it != set_data_socket.end(); socket_it++) {
                    // reset has been active
                    auto s = *socket_it;
                    if (s.socketfd != listener) {
                        s.has_been_active = false;
                        set_data_socket.erase(socket_it); 
                        set_data_socket.insert(s); 
                    }
                }
                break;
            default:
                for (auto socket_it = set_data_socket.begin(); socket_it != set_data_socket.end(); socket_it++) {
                    // won't touch the variables that are used for the sorting
                    // cast to mutable
                    auto socket = *socket_it;
                    int i = socket.socketfd;
                    cout << "begin of set loop\n";
                    cout << "DEBUG: i = " << i << endl;;
                    if (FD_ISSET(i, &readfds) && i == listener) { // we got a new connection
                        server_accept_client(listener, opt.block, &master, &fdmax, &set_data_socket, &socket_q);

                        // TODO: check if this possible workaround works
                        tv = {timeout_seconds, timeout_microseconds}; // should the timer be reset?
                    } else if ((FD_ISSET(i, &writefds) && socket.stage % 2 == 1)|| 
                            (FD_ISSET(i, &readfds) && socket.stage % 2 == 0))  { // we got a readable or writable socket
                        int comm_rv = server_communicate_new(socket);
                        cout << "server_communicate_new returned with " << comm_rv << endl;
                        if (comm_rv < 0) {
                            // only close socket if an error is encountered
                            cout << "server_communicate_new returned with < 0\n";
                            close(i); 
                            // remove the socket from the sets
                            FD_CLR(i, &master);
                            //set_data_socket.erase(find_socketfd(i, set_data_socket));
                            set_data_socket.erase(socket_it);
                            fill_up_sets(master, fdmax, set_data_socket, socket_q);
                            break;
                        } else {
                            // after a successful communication
                            // remove from the set if done
                            set_data_socket.erase(socket_it);

                            if (comm_rv == 1 && socket.stage == 10) {
                                cout << "removing a finished socket...\n";
                                FD_CLR(i, &master);
                                fill_up_sets(master, fdmax, set_data_socket, socket_q);
                                break;
                            } else {
                                // re-insert socket into the set
                                Socket newsocket = socket;
                                newsocket.has_been_active = true;
                                set_data_socket.insert(newsocket);
                                break;
                            }
                        }
                    }
                    cout << "end of set loop\n";
                }
                break;
        }
        cout << "end of loop\n\n";
    }

}

int remove_dead_connections(fd_set &master, int &fdmax, set<Socket> &set_data_socket, queue<Socket> *socket_q) {
    // TODO: remove from the STL set also
    set<Socket> newset;
    int num_removed = 0;
    for (auto s: set_data_socket) {
        if (!s.has_been_active) {
            FD_CLR(s.socketfd, &master);
	    num_removed++;
        } else {
            newset.insert(s);
        }
    }
    set_data_socket = newset;

    if (socket_q != nullptr) { // optionally fill up sets
        fill_up_sets(master, fdmax, set_data_socket, *socket_q);
    }
    return num_removed;
}

int server_accept_client(int listener, bool block, fd_set *master, int *fdmax, set<Socket> *set_data_socket, queue<Socket> *socket_q) {
    // Accept connections from listener.
    // For select()ing, new socketfds are inserted into the socket queue, 
    // then fill up the sets from the queue if the set size is 
    // less than max_active_connections.
    // For non-select()ing, the new socketfd is returned directely.

    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen = sizeof(remoteaddr);

    int newfd = accept(listener, (sockaddr *) &remoteaddr, &addrlen);
    cout << "DEBUG: accept " << newfd << endl;

    if (newfd == -1) {
        graceful("server_accept_new_client", 7);
    } else {
        // set non-blocking connection
        if (!block) {
            int val = fcntl(newfd, F_GETFL, 0);
            if (val < 0) {
                close(newfd);
                graceful_return("fcntl, GETFL", -2);
            }
            if (fcntl(newfd, F_SETFL, val|O_NONBLOCK) < 0) {
                close(newfd);
                graceful_return("fcntl, SETFL", -3);
            }            
        }

        if (master != NULL && fdmax != NULL && set_data_socket != NULL && socket_q != NULL) { // if using select
            (*socket_q).emplace(newfd);
            fill_up_sets(*master, *fdmax, *set_data_socket, *socket_q);
        }

        char remoteIP[INET6_ADDRSTRLEN];
        std::cout << "New connection from " << inet_ntop(remoteaddr.ss_family,
                                            get_in_addr((struct sockaddr*) &remoteaddr),
                                            remoteIP, INET6_ADDRSTRLEN)
                << " on socket " << newfd << std::endl;
    }
    return newfd;
}

int server_communicate_new(Socket &socket) {
    std::cout << "server_communicate\t" << socket.stage  << std::endl;
    switch (socket.stage) {
        // 1. server send a string "StuNo"
        case 1: {
            int val_send_thing = send_thing_new(socket, STR_1, strlen(STR_1));
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server send: " << STR_1 << std::endl;
            stage_done(socket);

        }
        // 2. server recv an int as student number, network byte order
        case 2: {
            int last_bytes_processed = socket.bytes_processed;
            uint32_t n_stuNo = 0;
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, (int)sizeof(uint32_t));
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(&n_stuNo+last_bytes_processed, buffer_in, sizeof(uint32_t)-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            socket.stuNo = ntohl(n_stuNo);
            std::cout << "server recv: " << socket.stuNo << std::endl;
            stage_done(socket);
        }
        // 3. server send a string "pid"
        case 3: {
            int val_send_thing = send_thing_new(socket, STR_2, strlen(STR_2));
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server send: " << STR_2 << std::endl;
            stage_done(socket);
        }
        // 4. server recv an int as client's pid, network byte order
        case 4: {
            int last_bytes_processed = socket.bytes_processed;
            uint32_t n_pid = 0;
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, (int)sizeof(uint32_t));
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(&n_pid+last_bytes_processed, buffer_in, sizeof(uint32_t)-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            socket.pid = ntohl(n_pid);
            std::cout << "server recv: " << socket.pid << std::endl;
            stage_done(socket);
        }
        // 5. server send a string "TIME"
        case 5: {
            int val_send_thing = send_thing_new(socket, STR_3, strlen(STR_3)+1);
            cout << "DEBUG TIME:\t" << val_send_thing << endl;
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server send: " << STR_3 << std::endl;
            stage_done(socket);
        }
        // 6. server recv client's time as a string with a fixed length of 19 bytes
        case 6: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, 19);
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(socket.time_str+last_bytes_processed, buffer_in, 19-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server recv: " << socket.time_str << std::endl;
            stage_done(socket);
        }     
        // 7. server send a string "str*****", where ***** is a 5-digit random number ranging from 32768-99999, inclusively
        case 7: {
            srand((unsigned)time(NULL)); 
            //int random = rand() % 67232 + 32768;
            socket.random = rand() % 67232 + 32768;
            std::stringstream ss;
            // ss << "str" << random;
            ss << "str" << socket.random;
            std::string str = ss.str();
            int val_send_thing = send_thing_new(socket, str.c_str(), str.length()+1);
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server send: " << str << std::endl;
            stage_done(socket);
        }
        // 8. server recv a random string with length *****, and each character is in ASCII 0~255
        case 8: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, socket.random);
            if (val_recv_thing < 0) {
                return val_recv_thing;
            }
            memcpy(socket.client_string+last_bytes_processed, buffer_in, socket.random-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server recv client string ok." << std::endl;
            stage_done(socket);
        }
        // 9. server send a string "end"
        case 9: {
            int val_send_thing = send_thing_new(socket, STR_4, strlen(STR_4));
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "server send: " << STR_4 << std::endl;
            std::cout << "server begin write file." << std::endl;
            std::stringstream ss_filename;
            ss_filename << "./server_txt/" << socket.stuNo << '.' << socket.pid << ".pid.txt";
            std::string str_filename = ss_filename.str();
            if (write_file_new(str_filename.c_str(), socket) == -1) {
                graceful_return("write_file", -11);
            }
            std::cout << "server end write file." << std::endl;
            stage_done(socket);
        }        
        default: {
            graceful_return("stage number beyond index", -13);
        }         
    }
}

void fill_up_sets(fd_set &master, int &fdmax, set<Socket> &set_data_socket, queue<Socket> &socket_q) {
    // fill up the sets from the queue
    while (!socket_q.empty() && set_data_socket.size() < max_active_connections) {
        // pop a new socket from the queue
        int socketfd = socket_q.front().socketfd;
        socket_q.pop();

        // insert it to the sets
        FD_SET(socketfd, &master); // add to master set
        set_data_socket.emplace(socketfd);
        cout << "Filling up sets from the queue with socketfd = " << socketfd << ". Size after: " << set_data_socket.size() << endl;
        if (socketfd > fdmax)      // keep track of the max
            fdmax = socketfd;
    }
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int send_thing_new(Socket &socket, const char *str, const int send_len) {
    int val_send = send(socket.socketfd, str+socket.bytes_processed, min(send_len-socket.bytes_processed, max_sendlen), MSG_NOSIGNAL);
    if (errno == EPIPE) {
        graceful_return("peer offline", -3);
    }
    else if (val_send == -1) {
        graceful_return("send", -6);
    }
    else if (socket.bytes_processed > send_len) {
        graceful_return("message sent is of wrong quantity of byte", -7);
    }
    else {
        socket.bytes_processed += val_send;
    }

    if (socket.bytes_processed == send_len) {
        socket.bytes_processed = 0;
        return 1;   // processed all bytes
    }
    return 0;       // no error, but not all bytes processed
}

int recv_thing_new(Socket &socket, char *buffer, const int recv_len) {
    memset(buffer, 0, sizeof(char)*buffer_len);
    // int val_recv = recv(socket.socketfd, buffer+socket.bytes_processed, max_recvlen, 0);
    int val_recv = recv(socket.socketfd, buffer, max_recvlen, 0);
    if (val_recv < 0) {
        cout << "DEBUG: bytes_processed: " << socket.bytes_processed << ", val_recv: " << val_recv << endl;
        graceful_return("recv", -10);
    }
    else if (val_recv == 0) {
        graceful_return("peer offline", -3);
    }
    else if (socket.bytes_processed > recv_len) {
        graceful_return("not received exact designated quantity of bytes", -10);
    }
    else {
        socket.bytes_processed += val_recv;
    }

    if (socket.bytes_processed == recv_len) {
        socket.bytes_processed = 0;
        return 1;   // processed all bytes
    }
    return 0;       // no error, but not all bytes processed
}

int write_file_new(const char *str_filename, Socket &socket) {
    // return 0: all good
    // return -1: file open error
    std::ofstream myfile;
    myfile.open(str_filename, std::ofstream::binary|std::ios::out|std::ios::trunc);
    if (!myfile.is_open()) {
        graceful_return("file open", -1);
    }
    myfile << socket.stuNo << '\n';
    myfile << socket.pid << '\n';
    myfile << socket.time_str << '\n';
    myfile.write(reinterpret_cast<const char*>(socket.client_string), socket.random);
    myfile.close();
    return 0;
}

int client_nofork(const Options &opt) {
    set<Socket> set_data_socket;
    fd_set master, readfds, writefds;      // master file descriptor list
    FD_ZERO(&master);
    int fdmax = 0; // maximum file descriptor number 
    timeval tv {timeout_seconds, timeout_microseconds}; // set a 2 second client timeout

    unsigned int num_success = 0;
    unsigned int num_current_conn = 0;
    while (num_success < opt.num) {
        //sleep(1);
        cout << "begin of main loop\n";
        cout << "set size: " << set_data_socket.size() << endl;
        if (int(num_current_conn) < min(max_active_connections, int(opt.num - num_success - num_current_conn))) {
            // create new connections
            int newfd = create_connection(opt);
            if (newfd < 0) { // error
                continue;
            } else {
                num_current_conn++;
                FD_SET(newfd, &master); // add to master set
                set_data_socket.emplace(newfd);
                if (newfd > fdmax) {      // keep track of the max
                    fdmax = newfd;
                }
            }
        } else {
            // communicate
            readfds = master; // copy at the last minutes
            writefds = master; // copy at the last minutes
            int rv = select(fdmax+1, &readfds, &writefds, NULL, &tv);
            cout << "select returned with " << rv << endl;
            switch (rv) {
                case -1:
                    graceful("select in main loop", 5);
                    break;
                case 0:
                    // timeout, close sockets that haven't responded in an interval, exept for listener
                    num_current_conn -= remove_dead_connections(master, fdmax, set_data_socket, nullptr);
                    tv = {timeout_seconds, timeout_microseconds}; // set a 2 second client timeout
                    for (auto socket_it = set_data_socket.begin(); socket_it != set_data_socket.end(); socket_it++) {
                        // reset has been active
                        auto s = *socket_it;
                        s.has_been_active = false;
                        set_data_socket.erase(socket_it); 
                        set_data_socket.insert(s); 
                    }
                    break;
                default:
                    for (auto socket_it = set_data_socket.begin(); socket_it != set_data_socket.end(); socket_it++) {
                        // won't touch the variables that are used for the sorting
                        // cast to mutable
                        auto socket = *socket_it;
                        int i = socket.socketfd;
                        cout << "DEBUG: i = " << i << endl;
                        cout << "DEBUG: stage = " << socket.stage << endl;
                        if ((FD_ISSET(i, &writefds) && socket.stage % 2 == 0) || 
                                (FD_ISSET(i, &readfds) && socket.stage % 2 == 1))  { // we got a readable or writable socket
                            int comm_rv = client_communicate_new(socket, opt);
                            cout << "client_communicate_new returned with " << comm_rv << endl;
                            if (comm_rv < 0) {
                                // only close socket if an error is encountered
                                close(i); 
                                // remove the socket from the sets
                                FD_CLR(i, &master);
                                set_data_socket.erase(socket_it);
				num_current_conn--;
                                break;
                            } else {
                                // after a successful communication
                                // remove from the set if done
                                set_data_socket.erase(socket_it); // remove for update or deletion

                                if (comm_rv == 1 && socket.stage == 10) {
                                    cout << "removing a finished socket...\n";
                                    FD_CLR(i, &master);
                                    num_success++;
                                    num_current_conn--;
                                    break;
                                } else {
                                    // re-insert socket into the set if not done
                                    socket.has_been_active = true;
                                    set_data_socket.insert(socket);
                                    break;
                                }
                            }
                        }
                    }
                    break;
            } // switch select 
        } // communicate
        cout << "end of main loop\n\n";
    }
    return 0;
}

int create_connection(const Options &opt) {
    // create a connection from opt
    // return negative code on error, socketfd on success

    struct sockaddr_in   servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(opt.port));
    if(inet_pton(AF_INET, opt.ip.c_str(), &servaddr.sin_addr) < 0)
        graceful_return("Invalid ip address", -1);

    // get a socket
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        graceful_return("socket", -2);

    // connect
    if (!opt.block) { //non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); 
        errno = 0;
        if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
            // EINPROGRESS means connection is in progress
            // then select on it
            fd_set fds;      
            if(errno != EINPROGRESS)
                graceful_return("connect", -3);
            
            FD_ZERO(&fds);      
            FD_SET(sockfd, &fds);       
            int select_rtn;

            if((select_rtn = select(sockfd+1, NULL, &fds, NULL, NULL)) > 0) {
                int error = -1, slen = sizeof(int);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&slen);
                //error == 0 means connect succeeded
                if(error != 0) graceful_return("connect", -3);
            }
        }
        //connect succeed   
    } else { // blocking
        if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
            graceful_return("connect", -3);
    }
    return sockfd;
}

int client_communicate_new(Socket &socket, const Options &opt) {
    std::cout << "client_communicate" << std::endl;
    switch (socket.stage) {
        // 1. recv "StuNo" from server
        case 1: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer[buffer_len] = {0};
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, strlen(STR_1));
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(buffer+last_bytes_processed, buffer_in, strlen(STR_1)-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client recv: " << buffer << std::endl;
            if (!same_string(buffer, STR_1, strlen(STR_1))) {
                graceful_return("not received correct string", -12);
            }
            stage_done(socket);
        }
        // 2. send client student number
        case 2: {
            socket.stuNo = stu_no;
            uint32_t n_stuNo = htonl(socket.stuNo);
            char buffer[buffer_len] = {0};
            memcpy(buffer, &n_stuNo, sizeof(uint32_t));
            int val_send_thing = send_thing_new(socket, buffer, sizeof(uint32_t));
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client send: " << buffer << std::endl;
            stage_done(socket);
        }
        // 3. recv "pid" from server
        case 3: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer[buffer_len] = {0};
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, strlen(STR_2));
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(buffer+last_bytes_processed, buffer_in, strlen(STR_2)-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client recv: " << buffer << std::endl;
            if (!same_string(buffer, STR_2, strlen(STR_2))) {
                graceful_return("not received correct string", -12);
            }
            stage_done(socket);
        }
        // 4. send client pid
        case 4: {
            uint32_t n_pid;
            pid_t pid = getpid();
            if(opt.fork) {
                n_pid = htonl((uint32_t)pid);
            }
            else {
                //if nofork, send: pid<<16 + socket_id
                n_pid = htonl((uint32_t)((((int)pid)<<16)+socket.socketfd));
            }           
            socket.pid = ntohl(n_pid);
            char buffer[buffer_len] = {0};
            memcpy(buffer, &n_pid, sizeof(uint32_t));
            int val_send_thing = send_thing_new(socket, buffer, sizeof(uint32_t));
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client send: " << buffer << std::endl;
            stage_done(socket);
        }
        // 5. recv "TIME" from server
        case 5: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer[buffer_len] = {0};
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, strlen(STR_3)+1);
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(buffer+last_bytes_processed, buffer_in, strlen(STR_3)+1-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client recv: " << buffer << std::endl;
            if (!same_string(buffer, STR_3, strlen(STR_3))) {
                graceful_return("not received correct string", -12);
            }
            stage_done(socket);
        }
        // 6. send client current time(yyyy-mm-dd hh:mm:ss, 19 bytes)
        case 6: {
            char buffer[buffer_len] = {0};
            str_current_time(socket.time_str);
            strncpy(buffer, socket.time_str, 19);
            int val_send_thing = send_thing_new(socket, buffer, 19);
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client send: " << buffer << std::endl;
            stage_done(socket);
        }
        // 7. recv "str*****" from server and parse
        case 7: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer[buffer_len] = {0};
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, 9);
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(buffer+last_bytes_processed, buffer_in, 9-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client recv: " << buffer << std::endl;
            if (!same_string(buffer, "str", 3)) {
                graceful_return("not received correct string", -12);
            }
            socket.random = parse_str(buffer);
            if (socket.random < 32768) {
                graceful_return("not received correct string", -12);
            }
            std::cout << "rand number: " << socket.random << std::endl;
            stage_done(socket);
        }
        // 8. send random string in designated length
        case 8: {
            char buffer[buffer_len] = {0};
            create_random_str(socket.random, (unsigned char*)buffer);
            memcpy(buffer, socket.client_string, socket.random);
            int val_send_thing = send_thing_new(socket, buffer, socket.random);
            if (val_send_thing < 0) {
                return(val_send_thing);
            }
            else if (val_send_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client send client string ok." << std::endl;
            stage_done(socket);
        }
        // 9. recv "end" from server
        case 9: {
            int last_bytes_processed = socket.bytes_processed;
            char buffer[buffer_len] = {0};
            char buffer_in[buffer_len] = {0};
            int val_recv_thing = recv_thing_new(socket, buffer_in, strlen(STR_4));
            if (val_recv_thing < 0) {
                return(val_recv_thing);
            }
            memcpy(buffer+last_bytes_processed, buffer_in, strlen(STR_4)-last_bytes_processed);
            if (val_recv_thing != 1) {
                return 0;       // not all processed
            }
            std::cout << "client recv: " << buffer << std::endl;
            if (!same_string(buffer, STR_4, strlen(STR_4))) {
                graceful_return("not received correct string", -12);
            }
            std::cout << "client begin write file." << std::endl;
            std::stringstream ss_filename;
            ss_filename << "./client_txt/" << socket.stuNo << '.' << socket.pid << ".pid.txt";
            std::string str_filename = ss_filename.str();
            if (write_file_new(str_filename.c_str(), socket) == -1) {
                graceful_return("write_file", -11);
            }
            std::cout << "client end write file." << std::endl;
            stage_done(socket);
        }                        
        default: {
            graceful_return("stage number beyond index", -13);
        }
    }
}

bool same_string(const char *str1, const char *str2, const int cmp_len) {
    char cmp_1[100], cmp_2[100];
    memcpy(cmp_1, str1, cmp_len);
    memcpy(cmp_2, str2, cmp_len);
    cmp_1[cmp_len] = '\0';
    cmp_2[cmp_len] = '\0';
    //std::cout << "cmp_1: " << cmp_1 << ", cmp_2: " << cmp_2 << ", strcmp: " << strcmp(cmp_1, cmp_2) << std::endl;
    if(strcmp(cmp_1, cmp_2) != 0) {
        return false;	//unexpected data        
    }
    else {
        return true;
    }
}

int str_current_time(char *time_str) {
    timespec time;
	clock_gettime(CLOCK_REALTIME, &time); 
	tm nowTime;
	localtime_r(&time.tv_sec, &nowTime);
	char current[1024];
	sprintf(current, "%04d-%02d-%02d %02d:%02d:%02d", 
			nowTime.tm_year + 1900, nowTime.tm_mon, nowTime.tm_mday, 
			nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    memcpy(time_str, current, 19);
	return 0;
}

int create_random_str(const int length, unsigned char *random_string) {
    unsigned char p[buffer_len] = {0};
    srand((unsigned)time(NULL));
    for(int i = 0; i < length; i++) {
        p[i] = rand() % 256;
    }
    memcpy(random_string, p, length);
    return 0;
}

int parse_str(const char *str) {
    char num[10] = {0};
    strncpy(num, str+3, 5);
    int parsed = atoi(num);
    if (parsed < 32768 || parsed > 99999) {
        return -1;      // unexpected
    }
    else {
        return parsed;
    }
}

int loop_server_fork(int listener, const Options &opt) {
    pid_t pid;
    //curnum: current children process num
    int curnum = 0;
    int rtr;    //return value
    fd_set readfds, rfds, wfds;

    //server won't naturally end
    while(1) {
        //curnum-1 when any child exit
        while((rtr = check_child()) == 1) {
            curnum--;
        }
        if (curnum <= max_active_connections) {
            //nonblock
            if(!opt.block) {
                FD_ZERO(&readfds);
                FD_SET(listener, &readfds);
                if((rtr = select(listener + 1, &readfds, NULL, NULL, NULL)) == -1) {
                    graceful("loop_server_fork select", -20);
                }
            }
            Socket newfd(-1);
            newfd.socketfd = server_accept_client(listener, opt.block, 
                                (fd_set*)NULL, (int*)NULL, (set<Socket> *)NULL, (queue<Socket> *)NULL);

            pid = fork();

            if(pid == -1) {
                graceful("loop_server_fork fork", -20);
            }
            //child
            else if(pid == 0) {
                //nonblock
                if(!opt.block) {
                    rtr = 0;
                    while(rtr >= 0) {
                        if(newfd.stage > 9) {
                            break;
                        }
                        //newfd.stage:
                        //  odd:    send
                        //  even:   recv 
                        if(!(newfd.stage % 2)) {
                            FD_ZERO(&rfds);
                            FD_SET(newfd.socketfd, &rfds);
                            if((rtr = select(newfd.socketfd+1, &rfds, NULL, NULL, NULL)) == -1)
                                graceful("loop_server_fork select", -1);
                            else if(FD_ISSET(newfd.socketfd, &rfds))
                                FD_CLR(newfd.socketfd, &rfds);  
                        }
                        else {
                            FD_ZERO(&wfds);
                            FD_SET(newfd.socketfd, &wfds);
                            if((rtr = select(newfd.socketfd+1, NULL, &wfds, NULL, NULL)) == -1)
                                graceful("loop_server_fork select", -1);
                            else if(FD_ISSET(newfd.socketfd, &wfds))
                                FD_CLR(newfd.socketfd, &wfds);  
                        }

                        rtr = server_communicate_new(newfd);
                    }     
                }
                else
                {
                    while((rtr = server_communicate_new(newfd)) >= 0)
                        if(newfd.stage > 9)
                            break;
                }
                
                if(rtr < 0) //error handling, close socketfd
                    close(newfd.socketfd);
                //rtr = 1, stage done
                exit(0); 
            }
            else    //father
                curnum ++;
        }
        //else, wait until socket_q.size() < max_active_connections
        else    
            continue;
    }
}

int check_child() {   
    errno = 0;
    pid_t pid = waitpid(-1, NULL, WNOHANG);   
    //set option = WNOHANG to avoid blocking in waitpid()
    //waitpid would return immediately if no exit child 
    if(pid == -1) {
        //ECHILD: no child process
        if(errno == ECHILD) {
            return 0;
        }
        graceful("check_SIGCHLD waitpid", -1);
    }
    //no exit child
    else if(pid == 0) {
        return 0;
    }
    //found exit child
    else if(pid > 0) {
        return 1;
    }
    return 2; 
}

int loop_client_fork(const Options &opt) {
    pid_t pid;
    //cur_num: current children process num
    unsigned int cur_num = 0, conn_num = 0;
    int rtr;    //return value
    fd_set rfds, wfds;

    while(1) {
        while((rtr = check_child()) == 1) {
            cur_num--;
        }
        if(cur_num < max_active_connections && conn_num < opt.num) {
            Socket newfd(-1);
            newfd.socketfd = create_connection(opt); 
            
            pid = fork();
            if(pid == -1) {
                graceful("loop_client_fork fork", -20);
            }
            //child
            else if(pid == 0) {
                //nonblock
                if(!opt.block) {
                    rtr = 0;
                    while(rtr >= 0) {
                        if(newfd.stage > 9) {
                            break;
                        }
                        //newfd.stage:
                        //  odd:    recv
                        //  even:   send 
                        if(newfd.stage % 2) {
                            FD_ZERO(&rfds);
                            FD_SET(newfd.socketfd, &rfds);
                            if((rtr = select(newfd.socketfd+1, &rfds, NULL, NULL, NULL)) == -1) {
                                graceful("loop_client_fork select", -1);
                            }
                            else if(FD_ISSET(newfd.socketfd, &rfds)) {
                                FD_CLR(newfd.socketfd, &rfds);
                            }
                        }
                        else {
                            FD_ZERO(&wfds);
                            FD_SET(newfd.socketfd, &wfds);
                            if((rtr = select(newfd.socketfd+1, NULL, &wfds, NULL, NULL)) == -1) {
                                graceful("loop_client_fork select", -1);
                            }
                            else if(FD_ISSET(newfd.socketfd, &wfds)) {
                                FD_CLR(newfd.socketfd, &wfds);
                            }      
                        }

                        rtr = client_communicate_new(newfd, opt);
                    }
                }
                else {
                    while((rtr = client_communicate_new(newfd, opt)) >= 0) {
                        if(newfd.stage > 9) {
                            break;
                        }
                    }                            
                }
                //error handling: close socketfd and reconnect
                if(rtr < 0) {
                    close(newfd.socketfd);
                    //reconnecting until succeed
                    while(!client_reconnected(opt));
                }
                //rtr = 1, stage done
                exit(0); 
            }
            //father
            else {
                cur_num ++;
                conn_num ++;
            }
        }
        //all connection finished
        else if(conn_num >= opt.num) {
            break;
        }
        //reach max_active_connections
        else {
            continue;
        }
    }
    return 0;
}

int client_reconnected(const Options &opt) {
    //sleep 20ms
    usleep(200000);
    int rtr;    //return value
    fd_set rfds, wfds;

    Socket newfd(-1);
    newfd.socketfd = create_connection(opt); 
    //nonblock
    if(!opt.block) {
        rtr = 0;
        while(rtr >= 0) {
            if(newfd.stage > 9) {
                break;
            }
            //newfd.stage:
            //  odd:    recv
            //  even:   send 
            if(newfd.stage % 2) {
                FD_ZERO(&rfds);
                FD_SET(newfd.socketfd, &rfds);
                if((rtr = select(newfd.socketfd+1, &rfds, NULL, NULL, NULL)) == -1) {
                    graceful("loop_client_fork select", -1);
                }
                else if(FD_ISSET(newfd.socketfd, &rfds)) {
                    FD_CLR(newfd.socketfd, &rfds);
                }
            }
            else {
                FD_ZERO(&wfds);
                FD_SET(newfd.socketfd, &wfds);
                if((rtr = select(newfd.socketfd+1, NULL, &wfds, NULL, NULL)) == -1) {
                    graceful("loop_client_fork select", -1);
                }
                else if(FD_ISSET(newfd.socketfd, &wfds)) {
                    FD_CLR(newfd.socketfd, &wfds); 
                }
            }
            rtr = client_communicate_new(newfd, opt);
        }
    }
    else {
        while((rtr = client_communicate_new(newfd, opt)) >= 0) {
            if(newfd.stage > 9) {
                break;
            }
        }         
    }
    //error handling: close socketfd and reconnect
    if(rtr < 0) {
        close(newfd.socketfd);
        return 0;
    }
    else {
        return 1;
    }
}