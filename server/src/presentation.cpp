#include "../include/presentation.hpp"

extern ApplicationLayer AppLayerInstance;
extern TransferLayer TransLayerInstance;

using namespace std;

void fsm(Client &client) {    
    // send
    switch (client.packet_type) {
        case -1: {
            // first message
            // construct message
            pack_message(message);
            send_msg(socketfd, message);


            // recv
            recv_msg();
            unpack_message();
            return;
        }

        case ...

        default:
            // error
            break;
    }
}