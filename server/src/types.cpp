#include "../include/types.hpp"

using namespace std;
using namespace fly;

CircularQueue::CircularQueue(size_t init_size) {
    _size = init_size;
    data = new uint8_t[_size];
    front = 0;
    rear = 0;   // point to the next of the last
    _num_free_bytes = _size;
}

// CircularQueue::~CircularQueue() {
//     //delete[] data;
// }

bool CircularQueue::enqueue(const uint8_t *buf, const size_t size) {
    if (_num_free_bytes < size) {
        // LOG(Level::Debug) << "queue overflow" << endl;
        //cerr << "DEBUG: queue overflow" << endl;
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        data[rear] = buf[i];
        rear = (rear + 1) % _size;
        _num_free_bytes -= 1;
    }
    return true;
}

bool CircularQueue::dequeue(uint8_t *buf, const size_t size) {
    if (_num_free_bytes + size > _size) {
        // LOG(Level::Debug) << "queue underflow" << endl;
        return false;
    }
    for (size_t i = 0; i < size; i++) {
        buf[i] = data[front];
        front = (front + 1) % _size;
    }
    _num_free_bytes += size; // maintainance
    return true;
}

size_t CircularQueue::get_num_free_bytes() const {
    return _num_free_bytes;
}

size_t CircularQueue::size() const {
    return _size - _num_free_bytes; // _size is actually _capacity
}

bool CircularQueue::is_empty() {
    return _num_free_bytes == _size;
}

bool CircularQueue::is_full() {
    return _num_free_bytes == 0;
}

uint16_t CircularQueue::current_packet_size() {
    // Hacky magic number
    uint8_t buf[2];
    buf[0] = data[(front+2) % _size];
    buf[1] = data[(front+3) % _size];
    cout << "size: " << size() << endl;
    cout << "buf " << +buf[0] << " " << +buf[1] << endl;
    return (ntohs(*(uint16_t*)buf));
}

bool CircularQueue::has_complete_packet() {
    // the first condition checks for if even contain a complete packet header,
    // and the second varifies the whole packet length
    return (size() >= kHeaderSize) && (size() >= kHeaderSize + current_packet_size());
}

Packet CircularQueue::dequeue_packet(bool is_scr) {
    DynamicPayload payload;
    size_t packet_size = current_packet_size();

    PacketHeader header;
    size_t struct_size; // TODO
    uint8_t* payload_struct;
    // cout << "Bad 1" << endl;
    if(is_scr == true) {
        struct_size = sizeof(ScreenInfoPacket);
        dequeue(payload_struct, struct_size);
        payload.first = payload_struct;
        // cout << "Bad 2" << endl;

        size_t vector_size = header.packet_size - struct_size;
        payload.second.reserve(vector_size);
        dequeue(payload.second.data(), vector_size);
        // cout << "Bad 3" << endl;

        Packet packet { header, payload };
        return packet;
    }
    // cout << "Bad 4" << endl;

    // get rid of the header from the buffer
    dequeue((uint8_t*)&header, kHeaderSize); // the bit ordering of the 2-3 bytes doesn't really matter

    // cout << "Bad 4" << endl;

    switch((PacketType)header.packet_type) {
        case PacketType::AuthResponse: {
            struct_size = sizeof(AuthResponsePacket);
            payload_struct = (uint8_t*)(new AuthResponsePacket);
            break;
        }
        case PacketType::SysInfoResponse: {
            struct_size = sizeof(SysInfoResponsePacket);
            payload_struct = (uint8_t*)(new SysInfoResponsePacket);
            break;
        }
        case PacketType::ConfInfoResponse: {
            struct_size = sizeof(ConfInfoResponsePacket);
            payload_struct = (uint8_t*)(new ConfInfoResponsePacket);
            break;
        }
        case PacketType::ProcInfoResponse: {
            struct_size = sizeof(ProcInfoResponsePacket);
            payload_struct = (uint8_t*)(new ProcInfoResponsePacket);
            break;
        }
        case PacketType::EtherInfoResponse: {
            struct_size = sizeof(EtherInfoResponsePacket);
            payload_struct = (uint8_t*)(new EtherInfoResponsePacket);
            break;
        }
        case PacketType::TerInfoResponse: {
            struct_size = sizeof(TerInfoResponsePacket);
            payload_struct = (uint8_t*)(new TerInfoResponsePacket);
            break;
        }
        case PacketType::IPTermResponse: {
            struct_size = sizeof(IPTermResponsePacket);
            packet_size = sizeof(IPTermResponsePacket) + kHeaderSize;
            payload_struct = (uint8_t*)(new IPTermResponsePacket);
            break;
        }
        case PacketType::End: {
            struct_size = sizeof(EndPacket);
            payload_struct = (uint8_t*)(new EndPacket);
            break;
        }
    }
    // switch (header.packet_type) {
    //     c
    //     struct_size = sizeof(DemoPacket);
    //     // payload_struct = uint8_t(new DemoPacket);
    //     break;
    // default:
    //     // raise error
    //     break;
    // }
    dequeue(payload_struct, struct_size);

    // cout << "Bad 5" << endl;

    payload.first = payload_struct;

    size_t vector_size = packet_size - kHeaderSize - struct_size;

    cout << "packet_size: " << packet_size << " kHeaderSize: " << kHeaderSize << " struct_size " <<  struct_size << " vector_size " << vector_size << endl;
    payload.second.reserve(vector_size);

    // cout << "Bad 6" << endl;
    dequeue(payload.second.data(), vector_size);

    // cout << "Bad 7" << endl;

    Packet packet { header, payload };
    return packet;
    // (DemoPacket*)packet.payload.first
}
