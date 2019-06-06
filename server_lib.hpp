#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include "shared_library.hpp"

int get_listener(const Options &opt);
int loop_server(int listener, const Options &opt);

const size_t kRecvBufferSize = 10240;

Struct Demo { };
using VLDemo = std::pair< Demo, std::vector<uint8_t> >;


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


#endif // SERVER_LIB_H
