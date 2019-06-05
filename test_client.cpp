#include "communicate.hpp"


DevInfo gene_dev_info() {
    DevInfo dev;
    return dev;
}

void Client::push_back_uint16(vector<uint8_t> & message, uint16_t data) {
    var16 = inet_htons(data);
    message.push_back((uint8_t)(var16>>8));
    message.push_back((uint8_t)(var16));
}

void Client::push_back_uint32(vector<uint8_t> & message, uint32_t data) {

}

vector<uint8_t> Client::client_pack_message(PacketType type) {
    vector<uint8_t> message;
    uint16_t var16, var32;
    //head
    message.push_back((uint8_t)0x91);
    //descriptor
    message.push_back((uint8_t)type);

    switch(type) {
        case PacketType::VersionRequire:
            //packet length
            
            //0x0000
            message.push_back((uint8_t)0x00);
            message.push_back((uint8_t)0x00);

            //data length
            var16 = inet_htons((uint16_t)11);
            message.push_back((uint8_t)(var16>>8));
            message.push_back((uint8_t)(var16));
            
            //main version
            var16 = inet_htons(serverMainVersion);
            message.push_back((uint8_t)(var16>>8));
            message.push_back((uint8_t)(var16));
            //sec 1 version
            message.push_back(serverSec1Version);
            //sec 2 version
            message.push_back(serverSec2Version);
            break;

        case PacketType::AuthResponse:

            break;
        default:
            break;
    }

    return message;
}

bool Client::client_unpack_message(PacketType type) {

}

int client_communicate(int socketfd, Options opt) {
    return 0;
}
