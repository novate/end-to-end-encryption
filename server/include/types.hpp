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

const int MaxHistoryLen = 300;
const int MaxFileLen = 1021;    // 1 KB    

// Constants
const size_t kSessionSetSize = 5; // max number of active sessions
const unsigned int kHeaderSize = 3; // network packet header size
const size_t kMaxPacketLength = 1024; // TODO: double check on this number
const size_t kRecvBufferSize = kMaxPacketLength;

// used as the first byte of data packets
enum class PacketType : uint8_t {
    Info = 0x00,
    InfoResponse = 0x01,
    Password = 0x02,
    PasswordResponse = 0x03,
    Refuse = 0x04,
    OnlineList = 0x05,
    UserName = 0x06,
    OnlineUser = 0x07,
    SyncEnd = 0x08,
    SendInvit = 0x09,
    RecvInvit = 0x0A,
    InvitResponse = 0x0B,
    Board = 0x0C,
    SingleCoord = 0x0D,
    DoubleCoord = 0x0E,
    GameOver = 0x0F,
    OfflineUser = 0x10,
};

struct DataPacketHeader {
    PacketType type;
    uint16_t payload_size;
};
    
struct DataPacket {
    PacketType type;
    std::vector<uint8_t> data;
};

// status codes
enum class StatusCode : int {
    OK = 0,
    OpenFile = -1,
    LogInit = -2,
    RecvError = -3,
    RecvPartial = -4,
    RecvComplete = -5,
    SendError = -6,
    SendPartial = -7,
    SendComplete = -8,
    Accept = -9,
    CreateSocket = -10,
    Setsockopt = -11,
    Bind = -12,
    Listen = -13,

    //present layer error code : start from -20
    NoCompletePacket = -20
};

// Server response type
enum class ResponseType : uint8_t {
    UserNotExist = 0,
    OK = 1,
    RefuseInvit = 2,
    WrongPassword = 3,
    ErrorOccurs = 4,
    AlreadyLoggedIn = 5,
    Busy = 6,
};

// State machine definition
// Defined almost sequentially. Actions corresponding to a state are in comments.
enum class SessionState : unsigned int {
    Acceptance,         // On acceptance, create a new client instance
    Error,
    WaitForPasswd,
    ServerWaiting,
    WaitInvitResponse,
    Responding,      
    WaitForBoard,
    InGame,
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


struct Message_To_App{
    PacketType type_;
    ResponseType respond_;
    std::string user_name_;
    std::string password_; 
    std::string user_name_b_;
    int board_[10][10];
    uint8_t plane_coord_[12];
    int x, y;
    int head_x, head_y;
    int tail_x, tail_y;
    // std::string media_text_;
    // std::vector<std::string> user_name_list_;
    // std::string file_name_;
    // std::string media_file_;
    // unsigned short config_; // 2 bytes in TransLayer
};

struct Message_To_Pre{
    PacketType type_;
    ResponseType respond_; 
    // int config_;
    std::vector<std::string> onlineuser_;
    std::string user_name_a_;
    std::string user_change_;
    int x,y;
    int head_x, head_y;
    int tail_x, tail_y;
};

// not sure if struct group_text should be kept or just use text[] instead ?
struct group_text{
    std::vector<std::string> user_list;
    std::string data;
};

struct file{
    std::string filePath;
};


// #endif

// #ifndef CLIENT_H
// #define CLIENT_H

struct Client;

struct GameInfo {
    Client* opponent_;
    int win_board_[10][10];
    uint8_t plane_coord_[12];
};

struct Client {

    Client(int socket_fd, size_t buffer_size) : 
        socket_fd(socket_fd),
        recv_buffer(buffer_size) 
    {}

    int client_id;

    CircularQueue recv_buffer;
    std::queue< std::vector<uint8_t> > send_buffer;

    int socket_fd;
    SessionState state = SessionState::Acceptance;
    std::string host_username_;
    
    Message_To_App message_ptoa;
    Message_To_Pre message_atop;
    GameInfo game_info_;

    // should be always greater than kHeaderSize (reset to this)
    // updated when packet is received and on state change


    // ~Client(); // Should call the destructor of the underlying CircularQueue


};

#endif