#ifndef TRANSFER_H
#define TRANSFER_H
#include "types.hpp"
#include "conf.hpp"
#include "../include/presentation.hpp"



class TransferLayer {

public:
    // TransferLayer() = default;
    // ~TransferLayer();

    // main loop. May never return.
    void select_loop(int listener);

    // send a packet to client
    StatusCode send_to_client(int target_socket_fd, const DataPacket packet);

    // kick client if password error
    StatusCode remove_client(Client &client);

    // Try to read as much as possible from the client's socket_fd and write to the 
    // underlying circular queue buffer.
    //
    // Precondition: client's socket_fd is readable 
    // Postcondition: the buffer is possibly left with packet data only partial received
    StatusCode try_recv(Client &client);

    StatusCode try_send(Client &client);

    //find client by user name
    Client* find_by_username(const std::string &username);

    Client* find_by_username_cnt(Client *client);

    std::vector<std::string> find_all_user(Client* host_client);
    std::vector<Client*> find_all_client(Client* host_client);

    // If false, return -1, else return socket code.
    int get_listener(const short port);

private:

    std::list<Client> session_set;

    // loop accept client
    StatusCode loop_accept_client();

    int accept_new_client(int listener);

    int reset_rw_fd_sets(fd_set &read_fds, fd_set &write_fds);


};

#endif