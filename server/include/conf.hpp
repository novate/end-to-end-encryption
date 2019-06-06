#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdint.h>

#define kServerConfFile "conf/server.conf"
#define kClientConfFile "conf/client.conf"

struct ServerConf {
    uint16_t port;
};

struct ClientConf {
    char server_ip[16] = "0.0.0.0";
    uint16_t port;
};

ServerConf read_conf_server();
ClientConf read_conf_client();