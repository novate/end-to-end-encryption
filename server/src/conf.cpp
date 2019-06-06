#include "../include/conf.hpp"
using namespace std;

int main() {
    ServerConf conf_s = read_conf_server();
    cout << "server conf, port:\t" << conf_s.port << endl;
    ClientConf conf_c = read_conf_client();
    cout << "client conf, server_ip:\t" << conf_c.server_ip << endl;
    cout << "client conf, port:\t" << conf_c.port << endl;
    return 0;
}

ServerConf read_conf_server() {
    ServerConf conf;
    char *buf = new char[100];

    ifstream ifs(kServerConfFile, ifstream::in);

    string line;
    // get port
    getline(ifs, line);
    stringstream ss(line);
    uint16_t port;
    ss >> buf >> buf >> port;
    conf.port = port;

    ifs.close();

    delete[] buf;
    return conf;
}

ClientConf read_conf_client() {
    ClientConf conf;
    char *buf = new char[100];

    ifstream ifs(kClientConfFile, ifstream::in);
    
    string line;
    // get ip
    getline(ifs, line);
    stringstream ss1(line);
    ss1 >> buf >> buf >> buf;
    strcpy(conf.server_ip, buf);

    // get port
    getline(ifs, line);
    stringstream ss2(line);
    uint16_t port;
    ss2 >> buf >> buf >> port;
    conf.port = port;

    delete[] buf;
    return conf;
}
