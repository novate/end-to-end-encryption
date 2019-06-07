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