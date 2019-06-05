#include "shared_library.hpp"

using namespace std;

class Socket {
//     public:
//         Socket(int socketfd)
//         : socketfd(socketfd)
//         {
//         }

//         int socketfd;
//         bool has_been_active = false;
//         int stage = 1;
//         int bytes_processed = 0;

//         int stuNo = 0;
//         int pid = 0;
//         int random = 0;
//         char time_str[time_str_length] = {0};
//         unsigned char client_string[buffer_len] = {0};
//         //char client_string[buffer_len] = {0};

//         bool operator< (const Socket &s) const { return socketfd < s.socketfd; }
};

enum class PakcetType: uint8_t {
    VersionRequire = 0x00,
    
    AuthRequest = 0x01,
    AuthResponse = 0x01,
    
    SysInfoRequest = 0x02,
    SysInfoResponse = 0x02,
    
    ConfInfoRequest = 0x03,
    ConfInfoResponse = 0x03,
    
    ProcInfoRequest = 0x04,
    ProcInfoResponse = 0x04,
    
    EtherInfoRequest = 0x05,
    EtherInfoResponse = 0x05,
    
    USBInfoRequest = 0x07,
    USBInfoResponse = 0x07,
    USBfileRequest = 0x0C,
    USBfileResponse = 0x0C,
    
    PrintDevRequest = 0x08,
    PrintDevResponse = 0x08,
    PrintQueueRequest = 0x0D,
    PrintQueueResponse = 0x0D,

    TerInfoRequest = 0x09,
    TerInfoResponse = 0x09,
    DumbTerRequest = 0x0A,
    DumbTerResponse = 0x0A,
    IPTermRequest = 0x0B,
    IPTermResponse = 0x0B,

    End = 0xFF,
};


struct DevInfo {
    uint16_t cpu;  // MHz, from '/proc/cpuinfo/', 1st CPU
    uint16_t ram;      // (kB to)MB, from '/proc/meminfo/'
    uint16_t flash;   // MB
    uint16_t innID = 1;

    uint8_t * groupSeq;
    uint8_t * model;
    uint8_t * softVersion;
    
    uint32_t instID;    //'devid' in database
    uint8_t instInnID;

    uint8_t * authstr;
};


class Client {
public:
    Client(){
        isConnected = false;
        isVerified = false;
    }

    int client_communicate(int socketfd, Options opt);
    vector<uint8_t> client_pack_message(PacketType type);
    void client_unpack_message(PakcetType type);

    void push_back_uint16(vector<uint8_t> & message, uint16_t data);
    void push_back_uint32(vector<uint8_t> & message, uint32_t data);
    // dev info
    DevInfo dev;

private:
    // TCP
    int socketfd;

    //status
    bool isConnected;
    bool isVerified;
    uint8_t seqNum;
    
    // server info
    uint16_t serverMainVersion;
    uint8_t serverSec1Version;
    uint8_t serverSec2Version;
};


int server_communicate(int socketfd, Options opt);
