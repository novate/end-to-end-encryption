#ifndef PRESENTATION_LAYER_H
#define PRESENTATION_LAYER_H
#include "types.hpp"
#include "transfer.hpp"
// #include "instances.hpp"
#include "parse_arguments.hpp"
#include "DatabaseConnection.hpp"

void encrypt_auth(u_int& random_num, u_int& svr_time, uint8_t* auth, const int length);
bool decrypt_auth(const u_int random_num, uint8_t* auth, const int length);

class PresentationLayer {
private:

public:
    PresentationLayer();
    // if return false, kill the connection of client.
    bool fsm(Client &client);
};

#endif
