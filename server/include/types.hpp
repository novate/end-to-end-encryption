#ifndef TYPES_HPP
#define TYPES_HPP
#include <vector>
#include <arpa/inet.h>
#include <stdint.h>
#include <string>
#include "Log.h"

#include <algorithm>    // std::max
#include <queue>
#include <list>

#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>

using namespace fly;

#define graceful_return(s, x) {\
    perror((s));\
    return((x)); }

// Constants
const unsigned int kHeaderSize = 4; // network packet header size
const size_t kMaxPacketLength = 10240; // TODO: double check on this number
const size_t kRecvBufferSize = kMaxPacketLength;

enum class PacketType : uint8_t {
    DemoPacket = 1;
};

Struct DemoPacket {
    uint8_t padding[4];
};

// Convert first to a packet struct
using DynamicPayload = std::pair<uint8_t*, std::vector<uint8_t>>;

struct PacketHeader {
    uint8_t direction;
    uint8_t packet_type;
    uint16_t packet_size;
};

struct Packet {
    PacketHeader header;
    DynamicPayload payload;
};

enum class PacketType: uint8_t {
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

    End = 0xFF
};

struct VersionRequirePacket {
    uint8_t direction;
    uint8_t descriptor;
    uint16_t packet_size;
    uint8_t pad_1[2];
    uint16_t payload;
    uint16_t version_require;
    uint8_t version_sub1;
    uint8_t version_sub2;
    uint16_t time_gap_fail;
    uint16_t time_gap_succeed;
    uint8_t is_empty_tty;
    uint8_t pad[3];
    uint8_t auth_string[32];
    uint32_t random_num;
    uint32_t svr_time;
};

// Used as a buffer in transfer layer, instantiated in Clients
class CircularQueue {
public:
    CircularQueue(size_t init_size);
    ~CircularQueue();
    uint8_t *data; // debug

    bool enqueue(const uint8_t *buf, const size_t size);
    bool dequeue(uint8_t *buf, const size_t size);
    uint16_t current_packet_size(); // note: this is actually the payload size

    // Also requires a getter method for _num_free_bytes here.
    size_t get_num_free_bytes() const;
    size_t size () const;
    bool is_empty();
    bool is_full();
    bool has_complete_packet(); // has at least one complete packet
    DataPacket dequeue_packet(); // return a complete packet

private:
    size_t _size;
    size_t _num_free_bytes;
    size_t front, rear;
};

struct Client {
    Client(int socket_fd, size_t buffer_size) :
        socket_fd(socket_fd),
        recv_buffer(buffer_size)
    {}
    // ~Client(); // Should call the destructor of the underlying CircularQueue

    CircularQueue recv_buffer;
    std::queue< std::vector<uint8_t> > send_buffer;

    int socket_fd;



};

#endif