#include "communicate.hpp"


DevInfo gene_dev_info() {
    DevInfo dev;
    return dev;
}

vector<uint8_t> client_pack_message(PacketType type) {
    vector<uint8_t> message;
    //descriptor
    message.push_back((uint8_t)type);

    switch(type) {
        case PacketType::VersionRequire:
            break;
        case PacketType::AuthResponse:

            break;
        default:
            break;
    }

    return message;
}

bool client_unpack_message(PacketType type) {

}

int client_communicate(int socketfd, Options opt) {
    return 0;
}
