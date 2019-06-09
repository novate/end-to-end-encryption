#include "../include/presentation.hpp"

extern TransferLayer TransLayerInstance;
extern Options opt;

using namespace std;
using namespace fly;

DatabaseConnection *DatabaseConnection::obj = NULL;
PresentationLayer::PresentationLayer()
{
        // initialize DatabaseConnection class
        DatabaseConnection::get_instance()->DatabaseInit();

        return;
}

bool PresentationLayer::fsm(Client &client) { 
    // send
    switch (client.state) {
        // first message
        case SessionState::Acceptance: {
            // first message
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x01;
            uint16_t packet_size = kHeaderSize + sizeof(AuthRequestPacket);
            header.packet_size = htons(packet_size);

            // packet struct
            AuthRequestPacket pkt;
            uint16_t payload_size = sizeof(AuthRequestPacket) - 4;
            pkt.payload_size = htons(payload_size);
            pkt.version_main = htons(0x3);
            pkt.version_sub1 = 0x4;
            pkt.version_sub2 = 0x5;
            pkt.time_gap_fail = htons((unsigned short)stoi(opt.at("设备连接间隔")));
            pkt.time_gap_succeed = htons((unsigned short)stoi(opt.at("设备采样间隔")));
            pkt.is_empty_tty = 0x1;

            // ------------------------------------------------Generateing Encryption String -----------------
            u_int random_num=0, svr_time=0;
            uint8_t auth_str[33] = "yzmond:id*str&to!tongji@by#Auth^";
            encrypt_auth(random_num, svr_time, auth_str, 32);
            for (int i = 0; i < 32; i++) {
                pkt.auth_string[i] = auth_str[i];
            } 
            pkt.random_num = htonl(random_num);
            pkt.svr_time = htonl(svr_time);

            vector<uint8_t> temp_vec;

            for(int i = 0; i < kHeaderSize; i++) {
                temp_vec.push_back(((uint8_t*)&header)[i]);
            }
            for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                temp_vec.push_back(((uint8_t*)&pkt)[i]);
            }

            client.send_buffer.push(temp_vec);

            client.state = SessionState::WaitAuth;
            // TODO Send Packet

            // vector<pair<uint8_t*, size_t>> buffer { 
            //     make_pair((uint8_t*)&header, sizeof(header)), 
            //     make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            // };
            // client.send_msg(buffer);
            return true;
            }
        case SessionState::WaitAuth : {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x00) {
                LERR << "收到的包类型错误，理想=0x00，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }
            VersionRequirePacket &recved_pkt = *((VersionRequirePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.required_version_main = ntohs(recved_pkt.required_version_main);
            if (recved_pkt.required_version_main > 0x03) {
                LERR << "客户端要求的服务端版本高于本服务器的版本，本机大版本=0x03，客户端要求大版本不小于0x" << hex <<  (u_int)recved_pkt.required_version_main << endl;
                return false;
            }

            std::vector<uint8_t> recvbuffer;
            recvbuffer.reserve(kHeaderSize + sizeof(recved_pkt));
            uint8_t *ptr = recvbuffer.data();
            memcpy(ptr, &packet, sizeof(uint8_t)*(kHeaderSize + sizeof(recved_pkt)));
            recvbuffer.insert(recvbuffer.end(), packet.payload.second.begin(), packet.payload.second.end());


            Packet packet2 = client.recv_buffer.dequeue_packet();
            if (packet2.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet2.header.direction << endl;
                return false;
            }
            if (packet2.header.packet_type != 0x01) {
                LERR << "收到的包类型错误，理想=0x01，实际=0x" << hex <<  (u_int)packet2.header.packet_type << endl;
                return false;
            }

            AuthResponsePacket &recved_pkt2 = *((AuthResponsePacket*)packet2.payload.first);
            packet2.header.packet_size = ntohs(packet2.header.packet_size);
            recved_pkt2.payload_size = ntohs(recved_pkt2.payload_size);
            recved_pkt2.random_num = ntohl(recved_pkt2.random_num);
            uint8_t* encrypted;
            encrypted = (uint8_t*)&recved_pkt2.cpu_frequence;
            if (decrypt_auth(recved_pkt2.random_num, encrypted, 104) == false) {
                LERR << "客户端未通过加密认证，强制下线" << endl;
                return false;
            }
            recved_pkt2.cpu_frequence = ntohs(recved_pkt2.cpu_frequence);
            recved_pkt2.ram = ntohs(recved_pkt2.ram);
            recved_pkt2.flash = ntohs(recved_pkt2.flash);
            recved_pkt2.internal_serial = ntohs(recved_pkt2.internal_serial);
            recved_pkt2.devid = ntohl(recved_pkt2.devid);
            client.ether_last = recved_pkt2.ethnum;
            if(DatabaseConnection::get_instance()->OnRecvAuthResponse(packet, &client) == false) return false;

            // ------------------------------------------Send Packet SysInfo----------------------------
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x02;
            header.packet_size = htons(8);

            // packet struct
            SysInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<uint8_t> temp_vec;
            for(int i = 0; i < kHeaderSize; i++) {
                temp_vec.push_back(((uint8_t*)&header)[i]);
            }
            for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                temp_vec.push_back(((uint8_t*)&pkt)[i]);
            }

            client.send_buffer.push(temp_vec);

            client.state = SessionState::WaitSysInfo;

            // vector<pair<uint8_t*, size_t>> buffer { 
            //     make_pair((uint8_t*)&header, sizeof(header)), 
            //     make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            // };
            // client.send_msg(buffer);

            return true;
        }
        case SessionState::WaitSysInfo: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x02) {
                LERR << "收到的包类型错误，理想=0x02，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }
            SysInfoResponsePacket &recved_pkt = *((SysInfoResponsePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.user_cpu_time = ntohl(recved_pkt.user_cpu_time);
            recved_pkt.nice_cpu_time = ntohl(recved_pkt.nice_cpu_time);
            recved_pkt.system_cpu_time = ntohl(recved_pkt.system_cpu_time);
            recved_pkt.idle_cpu_time = ntohl(recved_pkt.idle_cpu_time);
            recved_pkt.freed_memory = ntohl(recved_pkt.freed_memory);

            DatabaseConnection::get_instance()->OnRecvSysInfoResponse(packet, client);

            // -------------------------------------------Send Conf Packet ----------------------------------
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x03;
            header.packet_size = htons(8);

            // packet struct
            ConfInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<uint8_t> temp_vec;
            for(int i = 0; i < kHeaderSize; i++) {
                temp_vec.push_back(((uint8_t*)&header)[i]);
            }
            for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                temp_vec.push_back(((uint8_t*)&pkt)[i]);
            }

            client.send_buffer.push(temp_vec);

            client.state = SessionState::WaitConfigInfo;

            return true;
        }
        case SessionState::WaitConfigInfo: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x03) {
                LERR << "收到的包类型错误，理想=0x03，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }

            ConfInfoResponsePacket &recved_pkt = *((ConfInfoResponsePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);

            if(DatabaseConnection::get_instance()->OnRecvConfInfoResponse(packet, client) == false) return false;

            // --------------------------------------------------- send Proc Packet ---------------------------------
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x04;
            header.packet_size = htons(8);

            // packet struct
            ProcInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<uint8_t> temp_vec;
            for(int i = 0; i < kHeaderSize; i++) {
                temp_vec.push_back(((uint8_t*)&header)[i]);
            }
            for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                temp_vec.push_back(((uint8_t*)&pkt)[i]);
            }

            client.send_buffer.push(temp_vec);

            client.state = SessionState::WaitProcInfo;

            return true;
        }
        case SessionState::WaitProcInfo: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x04) {
                LERR << "收到的包类型错误，理想=0x04，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }

            ProcInfoResponsePacket &recved_pkt = *((ProcInfoResponsePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);

            if(DatabaseConnection::get_instance()->OnRecvProcInfoResponse(packet, client) == false) return false;

            switch(client.ethnum) {
                case 0x00: {
                    // TODO
                    // ------------------------------------------- send TTYInfo Packet ---------------------------------
                    // construct message
                    // header
                    PacketHeader header;
                    // packet_type: hton
                    header.direction = 0x11;
                    header.packet_type = 0x09;
                    header.packet_size = htons(8);
        
                    // packet struct
                    TerInfoRequestPacket pkt;
                    pkt.payload_size = 0;

                    vector<uint8_t> temp_vec;
                    for(int i = 0; i < kHeaderSize; i++) {
                        temp_vec.push_back(((uint8_t*)&header)[i]);
                    }
                    for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                        temp_vec.push_back(((uint8_t*)&pkt)[i]);
                    }

                    client.send_buffer.push(temp_vec);

                    client.state = SessionState::WaitTermInfo;
                    break;
                }
                case 0x02:{
                    // construct message
                    // header
                    PacketHeader header;
                    // packet_type: hton
                    header.direction = 0x11;
                    header.packet_type = 0x05;
                    header.packet_size = htons(8);

                    // packet struct
                    EtherInfoRequestPacket pkt;
                    pkt.port = htons(1);
                    pkt.payload_size = 0;

                    vector<uint8_t> temp_vec;
                    for(int i = 0; i < kHeaderSize; i++) {
                        temp_vec.push_back(((uint8_t*)&header)[i]);
                    }
                    for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                        temp_vec.push_back(((uint8_t*)&pkt)[i]);
                    }

                    client.send_buffer.push(temp_vec);

                    // No Break
                }
                case 0x01:{
                    // construct message
                    // header
                    PacketHeader header;
                    // packet_type: hton
                    header.direction = 0x11;
                    header.packet_type = 0x05;
                    header.packet_size = htons(8);

                    // packet struct
                    EtherInfoRequestPacket pkt;
                    pkt.port = htons(0);
                    pkt.payload_size = 0;

                    vector<uint8_t> temp_vec;
                    for(int i = 0; i < kHeaderSize; i++) {
                        temp_vec.push_back(((uint8_t*)&header)[i]);
                    }
                    for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                        temp_vec.push_back(((uint8_t*)&pkt)[i]);
                    }

                    client.send_buffer.push(temp_vec);

                    client.state = SessionState::WaitEtheInfo;

                    break;
                }

            }

            return true;
        }
        case SessionState::WaitEtheInfo: {
            switch(client.ethnum) {
                case 0x02: {
                    // recv
                    Packet packet = client.recv_buffer.dequeue_packet();
                    if (packet.header.direction != 0x91) {
                        LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                        return false;
                    }
                    if (packet.header.packet_type != 0x05) {
                        LERR << "收到的包类型错误，理想=0x05，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                        return false;
                    }
                    EtherInfoResponsePacket &recved_pkt = *((EtherInfoResponsePacket*)packet.payload.first);
                    packet.header.packet_size = ntohs(packet.header.packet_size);
                    recved_pkt.port = ntohs(recved_pkt.port);
                    recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
                    recved_pkt.options = ntohs(recved_pkt.options);
                    recved_pkt.addr = ntohl(recved_pkt.addr);
                    recved_pkt.mask = ntohl(recved_pkt.mask);
                    recved_pkt.addr_port_1 = ntohl(recved_pkt.addr_port_1);
                    recved_pkt.mask_port_1 = ntohl(recved_pkt.mask_port_1);
                    recved_pkt.addr_port_2 = ntohl(recved_pkt.addr_port_2);
                    recved_pkt.mask_port_2 = ntohl(recved_pkt.mask_port_2);
                    recved_pkt.addr_port_3 = ntohl(recved_pkt.addr_port_3);
                    recved_pkt.mask_port_3 = ntohl(recved_pkt.mask_port_3);
                    recved_pkt.addr_port_4 = ntohl(recved_pkt.addr_port_4);
                    recved_pkt.mask_port_4 = ntohl(recved_pkt.mask_port_4);
                    recved_pkt.addr_port_5 = ntohl(recved_pkt.addr_port_5);
                    recved_pkt.mask_port_5 = ntohl(recved_pkt.mask_port_5);
                    recved_pkt.send_bytes = ntohl(recved_pkt.send_bytes);
                    recved_pkt.send_packets = ntohl(recved_pkt.send_packets);
                    recved_pkt.send_errs = ntohl(recved_pkt.send_errs);
                    recved_pkt.send_drop = ntohl(recved_pkt.send_drop);
                    recved_pkt.send_fifo = ntohl(recved_pkt.send_fifo);
                    recved_pkt.send_frame = ntohl(recved_pkt.send_frame);
                    recved_pkt.send_compressed = ntohl(recved_pkt.send_compressed);
                    recved_pkt.send_multicast = ntohl(recved_pkt.send_multicast);
                    recved_pkt.recv_bytes = ntohl(recved_pkt.recv_bytes);
                    recved_pkt.recv_packets = ntohl(recved_pkt.recv_packets);
                    recved_pkt.recv_errs = ntohl(recved_pkt.recv_errs);
                    recved_pkt.recv_drop = ntohl(recved_pkt.recv_drop);
                    recved_pkt.recv_fifo = ntohl(recved_pkt.recv_fifo);
                    recved_pkt.recv_frame = ntohl(recved_pkt.recv_frame);
                    recved_pkt.recv_compressed = ntohl(recved_pkt.recv_compressed);
                    recved_pkt.recv_multicast = ntohl(recved_pkt.recv_multicast);

                    if(DatabaseConnection::get_instance()->OnRecvEtherInfoResponse(packet, client) == false) return false;
                    // No break
                }
                case 0x01: {
                    // recv
                    Packet packet = client.recv_buffer.dequeue_packet();
                    if (packet.header.direction != 0x91) {
                        LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                        return false;
                    }
                    if (packet.header.packet_type != 0x05) {
                        LERR << "收到的包类型错误，理想=0x05，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                        return false;
                    }
                    EtherInfoResponsePacket &recved_pkt = *((EtherInfoResponsePacket*)packet.payload.first);
                    packet.header.packet_size = ntohs(packet.header.packet_size);
                    recved_pkt.port = ntohs(recved_pkt.port);
                    recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
                    recved_pkt.options = ntohs(recved_pkt.options);
                    recved_pkt.addr = ntohl(recved_pkt.addr);
                    recved_pkt.mask = ntohl(recved_pkt.mask);
                    recved_pkt.addr_port_1 = ntohl(recved_pkt.addr_port_1);
                    recved_pkt.mask_port_1 = ntohl(recved_pkt.mask_port_1);
                    recved_pkt.addr_port_2 = ntohl(recved_pkt.addr_port_2);
                    recved_pkt.mask_port_2 = ntohl(recved_pkt.mask_port_2);
                    recved_pkt.addr_port_3 = ntohl(recved_pkt.addr_port_3);
                    recved_pkt.mask_port_3 = ntohl(recved_pkt.mask_port_3);
                    recved_pkt.addr_port_4 = ntohl(recved_pkt.addr_port_4);
                    recved_pkt.mask_port_4 = ntohl(recved_pkt.mask_port_4);
                    recved_pkt.addr_port_5 = ntohl(recved_pkt.addr_port_5);
                    recved_pkt.mask_port_5 = ntohl(recved_pkt.mask_port_5);
                    recved_pkt.send_bytes = ntohl(recved_pkt.send_bytes);
                    recved_pkt.send_packets = ntohl(recved_pkt.send_packets);
                    recved_pkt.send_errs = ntohl(recved_pkt.send_errs);
                    recved_pkt.send_drop = ntohl(recved_pkt.send_drop);
                    recved_pkt.send_fifo = ntohl(recved_pkt.send_fifo);
                    recved_pkt.send_frame = ntohl(recved_pkt.send_frame);
                    recved_pkt.send_compressed = ntohl(recved_pkt.send_compressed);
                    recved_pkt.send_multicast = ntohl(recved_pkt.send_multicast);
                    recved_pkt.recv_bytes = ntohl(recved_pkt.recv_bytes);
                    recved_pkt.recv_packets = ntohl(recved_pkt.recv_packets);
                    recved_pkt.recv_errs = ntohl(recved_pkt.recv_errs);
                    recved_pkt.recv_drop = ntohl(recved_pkt.recv_drop);
                    recved_pkt.recv_fifo = ntohl(recved_pkt.recv_fifo);
                    recved_pkt.recv_frame = ntohl(recved_pkt.recv_frame);
                    recved_pkt.recv_compressed = ntohl(recved_pkt.recv_compressed);
                    recved_pkt.recv_multicast = ntohl(recved_pkt.recv_multicast);

                    if(DatabaseConnection::get_instance()->OnRecvEtherInfoResponse(packet, client) == false) return false;

                    // ------------------------------------------- send TTYInfo Packet ---------------------------------
                    // construct message
                    // header
                    PacketHeader header;
                    // packet_type: hton
                    header.direction = 0x11;
                    header.packet_type = 0x09;
                    header.packet_size = htons(8);
        
                    // packet struct
                    TerInfoRequestPacket pkt;
                    pkt.payload_size = 0;

                    vector<uint8_t> temp_vec;
                    for(int i = 0; i < kHeaderSize; i++) {
                        temp_vec.push_back(((uint8_t*)&header)[i]);
                    }
                    for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                        temp_vec.push_back(((uint8_t*)&pkt)[i]);
                    }

                    client.send_buffer.push(temp_vec);

                    client.state = SessionState::WaitTermInfo;

                    break;
                }
            }
        }
        case SessionState::WaitTermInfo: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x09) {
                LERR << "收到的包类型错误，理想=0x09，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }
            TerInfoResponsePacket &recved_pkt = *((TerInfoResponsePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.term_num = ntohs(recved_pkt.term_num);
            if(DatabaseConnection::get_instance()->OnRecvTermResponse(packet, client) == false) return false;

            // ------------------------------------------- Send IP/Dumb Packet ----------------------------------
            for (int i = 0; i < 16; i++) {
                client.dumb_term[i] = recved_pkt.dumb_term[i];
                if(client.dumb_term[i] == 0x00) continue;
                // construct message
                // header
                PacketHeader header;
                // packet_type: hton
                header.direction = 0x11;
                header.packet_type = 0x0a;
                header.packet_size = htons(8);

                // packet struct
                IPTermRequestPacket pkt;
                pkt.ttyno = i + 1;
                pkt.payload_size = 0;

                vector<uint8_t> temp_vec;
                for(int i = 0; i < kHeaderSize; i++) {
                    temp_vec.push_back(((uint8_t*)&header)[i]);
                }
                for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                    temp_vec.push_back(((uint8_t*)&pkt)[i]);
                }

                client.send_buffer.push(temp_vec);
            }
            for (int i = 0; i < 254; i++) {
                client.ip_term[i] = recved_pkt.ip_term[i];
                if(client.ip_term[i] == 0x00) continue;
                // construct message
                // header
                PacketHeader header;
                // packet_type: hton
                header.direction = 0x11;
                header.packet_type = 0x0b;
                header.packet_size = htons(8);

                // packet struct
                IPTermRequestPacket pkt;
                pkt.ttyno = i + 1;
                pkt.payload_size = 0;

                vector<uint8_t> temp_vec;
                for(int i = 0; i < kHeaderSize; i++) {
                    temp_vec.push_back(((uint8_t*)&header)[i]);
                }
                for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                    temp_vec.push_back(((uint8_t*)&pkt)[i]);
                }

                client.send_buffer.push(temp_vec);
            }

            client.state = SessionState::WaitIPTermInfo;

            return true;
        }

        case SessionState::WaitIPTermInfo: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0x0a || packet.header.packet_type != 0x0b) {
                LERR << "收到的包类型错误，理想=0x09，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }
            IPTermResponsePacket &recved_pkt = *((IPTermResponsePacket*)packet.payload.first);
            packet.header.packet_size = ntohs(packet.header.packet_size);
            recved_pkt.ttyno = ntohs(recved_pkt.ttyno);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.ttyip = ntohs(recved_pkt.ttyip);
            if(DatabaseConnection::get_instance()->OnRecvIPTermResponse(packet, client) == false) return false;
            client.tty_cnt++;
            client.scr_num = (int)recved_pkt.screen_num;
            if(recved_pkt.screen_num != 0x00) client.state = SessionState::WaitScrInfo;

            return true;
        }

        case SessionState::WaitScrInfo: {
            for(int i = 0; i < client.scr_num; i++) {
                // recv
                Packet packet = client.recv_buffer.dequeue_packet(true);

                ScreenInfoPacket &recved_pkt = *((ScreenInfoPacket*)packet.payload.first);
                recved_pkt.server_port = ntohs(recved_pkt.server_port);
                recved_pkt.server_ip = ntohl(recved_pkt.server_ip);
                recved_pkt.time = ntohl(recved_pkt.time);
                recved_pkt.send_term_byte = ntohl(recved_pkt.send_term_byte);
                recved_pkt.recv_term_byte = ntohl(recved_pkt.recv_term_byte);
                recved_pkt.send_server_byte = ntohl(recved_pkt.send_server_byte);
                recved_pkt.recv_server_byte = ntohl(recved_pkt.recv_server_byte);
                recved_pkt.ping_min = ntohl(recved_pkt.ping_min);
                recved_pkt.ping_avg = ntohl(recved_pkt.ping_avg);
                recved_pkt.ping_max = ntohl(recved_pkt.ping_max);

                if(DatabaseConnection::get_instance()->OnRecvScreenInfoPacket(packet, client) == false) return false;

            }
            if(client.tty_cnt == client.tty_connected)  {
                // construct message
                // header
                PacketHeader header;
                // packet_type: hton
                header.direction = 0x11;
                header.packet_type = 0xff;
                header.packet_size = htons(8);

                // packet struct
                EndPacket pkt;
                pkt.payload_size = 0;

                vector<uint8_t> temp_vec;
                for(int i = 0; i < kHeaderSize; i++) {
                    temp_vec.push_back(((uint8_t*)&header)[i]);
                }
                for(int i = 0; i < sizeof(AuthRequestPacket); i++) {
                    temp_vec.push_back(((uint8_t*)&pkt)[i]);
                }

                client.send_buffer.push(temp_vec);

                client.state = SessionState::End;
            }
            else client.state = SessionState::WaitIPTermInfo;

            return true;
        }
        case SessionState::End: {
            // recv
            Packet packet = client.recv_buffer.dequeue_packet();
            if (packet.header.direction != 0x91) {
                LERR << "收到的文件头错误，理想=0x91，实际=0x" << hex <<  (u_int)packet.header.direction << endl;
                return false;
            }
            if (packet.header.packet_type != 0xff) {
                LERR << "收到的包类型错误，理想=0xff，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }

            LENV << "服务器收到客户端的应答报文，关闭TCP连接" << endl;

            if(DatabaseConnection::get_instance()->UpdateTTYConnected(client) == false) return false;

            return true;
        }
        default:
            // error
            LERR << "收到了不该收到的包" << endl;
            break;
    }
}

void encrypt_auth(u_int& random_num, u_int& svr_time, uint8_t* auth, const int length) {
    svr_time = (u_int)time(0);
    svr_time = svr_time ^ (u_int)0xFFFFFFFF;
    srand(svr_time);
    random_num = (u_int)rand();
    int pos = random_num % 4093;
    for (int i = 0; i < length; i++) {
        auth[i] = auth[i] ^ kSecret[pos];
        pos = (pos+1)%4093;
    }
}

bool decrypt_auth(const u_int random_num, uint8_t* auth, const int length) {
	const uint8_t c_auth[33] = "yzmond:id*str&to!tongji@by#Auth^";
    int pos = random_num % 4093;
    for (int i = 0; i < length; i++) {
        auth[i] = auth[i] ^ kSecret[pos];
		if (i > length-32 && auth[i] != c_auth[i-length+32]) {
			return false;
		}
        pos = (pos+1)%4093;
    }
	return true;
}
