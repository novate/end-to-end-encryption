#include "server_lib.hpp"
#include "communicate.hpp"

int get_listener(const Options &opt) {
    int listen_port = stoi(opt.at("监听端口号"));

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener < 0)
        graceful("socket", 1);

    // lose the pesky "address already in use" error message
    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // bind
    if (server_bind_port(listener, listen_port) == -1)
        graceful("bind", 2);

    // TODO: use the new log library
    std::cout << "Listening on port " << listen_port << " on all interfaces...\n";

    // set listening
    // TODO: determine the number based on the number of concurrent connections
    listen(listener, 1000);
    return listener;
}

int loop_server(int listener, const Options &opt) {
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
                    cout << "DEBUG: i = " << i << endl;
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