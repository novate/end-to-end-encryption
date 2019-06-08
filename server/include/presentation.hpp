#ifndef PRESENTATION_LAYER_H
#define PRESENTATION_LAYER_H
#include "types.hpp"
#include "transfer.hpp"
#include "parse_arguments.hpp"
#include "DatabaseConnection.hpp"

void encrypt_auth(u_int& random_num, u_int& svr_time, uint8_t* auth, const int length);
bool decrypt_auth(const u_int random_num, uint8_t* auth, const int length);
std::string logify_data(vector<uint8_t> & message);
std::string logify_data(const uint8_t* data, const int len);

class PresentationLayer {
private:

public:
    PresentationLayer();
    // if return false, kill the connection of client.
    bool fsm(Client &client);
};

#endif
