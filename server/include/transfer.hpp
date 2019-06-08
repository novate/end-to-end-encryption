#ifndef TRANSFER_H
#define TRANSFER_H
#include "types.hpp"
#include "log.hpp"
#include "./presentation.hpp"

enum class StatusCode : int {
    OK = 0,
    OpenFile = -1,
    LogInit = -2,
    RecvError = -3,
    RecvPartial = -4,
    RecvComplete = -5,
    SendError = -6,
    SendPartial = -7,
    SendComplete = -8,
    Accept = -9,
    CreateSocket = -10,
    Setsockopt = -11,
    Bind = -12,
    Listen = -13,

    //present layer error code : start from -20
    NoCompletePacket = -20
};

class TransferLayer {

public:
    // TransferLayer() = default;
    // ~TransferLayer();

    // main loop. May never return.
    void select_loop(int listener);

    // send a packet to client
    StatusCode send_to_client(int target_socket_fd, const Packet packet);

    // kick client if password error
    StatusCode remove_client(Client &client);

    // Try to read as much as possible from the client's socket_fd and write to the 
    // underlying circular queue buffer.
    //
    // Precondition: client's socket_fd is readable 
    // Postcondition: the buffer is possibly left with packet data only partial received
    StatusCode try_recv(Client &client);

    StatusCode try_send(Client &client);

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