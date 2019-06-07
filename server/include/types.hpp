#ifndef TYPES_HPP
#define TYPES_HPP
#include <vector>
#include <arpa/inet.h>
#include <stdint.h>
#include <string>

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

#define graceful_return(s, x) {\
    perror((s));\
    return((x)); }

// Constants
const unsigned int kHeaderSize = 4; // network packet header size
const size_t kMaxPacketLength = 10240; // TODO: double check on this number
const size_t kRecvBufferSize = kMaxPacketLength;

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
    uint8_t pad_1[2];
    uint16_t payload_size;
    uint16_t version_require;
    uint8_t version_sub1;
    uint8_t version_sub2;
    uint16_t time_gap_fail;
    uint16_t time_gap_succeed;
    uint8_t is_empty_tty;
    uint8_t pad_2[3];
    uint8_t auth_string[32];
    uint32_t random_num;
    uint32_t svr_time;
};

struct AuthRequestPacket {

};

struct AuthResponsePacket {
    uint8_t pad_1[2];
    uint16_t payload_size;
    uint16_t cpu_frequence;
    uint16_t ram;
    uint16_t flash;
    uint16_t internal_serial;
    uint8_t group_serial[16];
    uint8_t device_type[16];
    uint8_t software_verison[16];
    uint8_t ethnum;
    uint8_t syncnum;
    uint8_t asyncnum;
    uint8_t switchnum;
    uint8_t usbnum;
    uint8_t prnnum;
    uint8_t pad_2[2];
    uint32_t devid;
    uint8_t devno;
    uint8_t pad_3[3];
    uint8_t auth_string[32];
    uint32_t random_num;
};

struct SysInfoResponsePacket {
    uint8_t pad_1[2];   
    uint16_t payload_size;
    uint32_t user_cpu_time;
    uint32_t nice_cpu_time;
    uint32_t system_cpu_time;
    uint32_t idle_cpu_time; 
    uint32_t freed_memory;
};

struct ConfInfoResponsePacket {
    uint8_t pad_1[2];   
    uint16_t payload_size;
};

struct ProcInfoResponsePacket {
    uint8_t pad_1[2];   
    uint16_t payload_size;
}

struct EtherInfoResponsePacket {
    uint16_t port;  // Ethernet 0/1 port   
    uint16_t payload_size;
    uint8_t if_exist;
    uint8_t if_set;
    uint8_t state;  // UP or DOWN
    uint8_t pad_1;
    uint8_t mac[6];
    uint16_t options;
    uint32_t addr;
    uint32_t mask;
    uint32_t addr_port_1;
    uint32_t mask_port_1;
    uint32_t addr_port_2;
    uint32_t mask_port_2;
    uint32_t addr_port_3;
    uint32_t mask_port_3;
    uint32_t addr_port_4;
    uint32_t mask_port_4;
    uint32_t addr_port_5;
    uint32_t mask_port_5;
    uint32_t send_bytes;
    uint32_t send_packets;
    uint32_t send_errs;
    uint32_t send_drop;
    uint32_t send_fifo;
    uint32_t send_frame;
    uint32_t send_compressed;
    uint32_t send_multicast;
    uint32_t recv_bytes;
    uint32_t recv_packets;
    uint32_t recv_errs;
    uint32_t recv_drop;
    uint32_t recv_fifo;
    uint32_t recv_frame;
    uint32_t recv_compressed;
    uint32_t recv_multicast;
};

struct TerInfoResponsePacket {
    uint16_t port;
    uint16_t payload_size;
    uint8_t dumb_term[16];
    uint8_t ip_term[254];
    uint16_t term_num;
};

struct IPTermResponsePacket {
    uint16_t ttyno;
    uint16_t payload_size;
    uint8_t port;
    uint8_t readno;
    uint8_t active_screen;
    uint8_t screen_num;
    uint32_t ttyip;
    uint8_t type[12];
    uint8_t state[8]; 
};

struct ScreenInfoPacket {
    uint8_t screen_no;
    uint8_t pad_1;
    uint16_t server_port;
    uint32_t server_ip;
    uint8_t proto[12];
    uint8_t state[8];
    uint8_t promp[24];
    uint8_t tty_type[12];
    uint32_t time;
    uint32_t send_term_byte;
    uint32_t recv_term_byte;
    uint32_t send_server_byte;
    uint32_t recv_server_byte;
    uint32_t ping_min;
    uint32_t ping_avg;
    uint32_t ping_max;
};

struct EndPacket {
    uint8_t pad_1[2];
    uint16_t payload_size;
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
    Packet dequeue_packet(); // return a complete packet

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