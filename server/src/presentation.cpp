#include "../include/presentation.hpp"

extern TransferLayer TransLayerInstance;

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
    switch (client.RecvPacketType) {
        // first message
        case PacketType::NullPacket: {
            // Read config
            ifstream ifs(kFnConfServer);
            if (!ifs.is_open()) {
                cout << "找不到该配置文件：" << kFnConfServer << endl;
                cout << "请将该配置文件和本可执行文件置于同一个目录下" << endl;
                return -1;
            }
            Options opt = parse_arguments(ifs);

            bool log_env[4][4];
            string s_tp = opt.at("tmp_packet");
            string s_ts = opt.at("tmp_socket");
            string s_dp = opt.at("dev_packet");
            string s_ds = opt.at("dev_socket");
            for (u_int i = 0; i < 4; i++) {
                log_env[0][i] = (s_tp[i] == '1');
            }
            for (u_int i = 0; i < 4; i++) {
                log_env[1][i] = (s_ts[i] == '1');
            }
            for (u_int i = 0; i < 4; i++) {
                log_env[2][i] = (s_dp[i] == '1');
            }
            for (u_int i = 0; i < 4; i++) {
                log_env[3][i] = (s_ds[i] == '1');
            }

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
            u_int random_num=0, svr_time=0;
            uint8_t auth_str[33] = "yzmond:id*str&to!tongji@by#Auth^";
            encrypt_auth(random_num, svr_time, auth_str, 32);
            for (int i = 0; i < 32; i++) {
                pkt.auth_string[i] = auth_str[i];
            } 
            pkt.random_num = htonl(random_num);
            pkt.svr_time = htonl(svr_time);

            vector<pair<uint8_t*, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            client.send_msg(buffer);

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
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.required_version_main = ntohs(recved_pkt.required_version_main);
            if (recved_pkt.required_version_main > 0x03) {
                LERR << "客户端要求的服务端版本高于本服务器的版本，本机大版本=0x03，客户端要求大版本不小于0x" << hex <<  (u_int)recved_pkt.required_version_main << endl;
                return false;
            }

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
            DatabaseConnection::get_instance()->OnRecvAuthResponse(packet, &client);

            return true;
        }
        case PacketType::AuthResponse: {
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

            vector<pair<uint8_t*, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            client.send_msg(buffer);

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
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.user_cpu_time = ntohl(recved_pkt.user_cpu_time);
            recved_pkt.nice_cpu_time = ntohl(recved_pkt.nice_cpu_time);
            recved_pkt.system_cpu_time = ntohl(recved_pkt.system_cpu_time);
            recved_pkt.idle_cpu_time = ntohl(recved_pkt.idle_cpu_time);
            recved_pkt.freed_memory = ntohl(recved_pkt.freed_memory);

            // writeToDataBase(client, packet);

            return true;
        }
        case PacketType::SysInfoResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x03;
            header.packet_size = htons(8);

            // packet struct
            SysInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<pair<uint8_t*, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            client.send_msg(buffer);

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
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);

            // writeToDataBase(client, packet);

            return true;
        }
        case PacketType::ConfInfoResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x04;
            header.packet_size = htons(8);

            // packet struct
            SysInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<pair<uint8_t*, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            client.send_msg(buffer);

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
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            // writeToDataBase(client, packet);

            return true;
        }
        // Proc包后面可能是Ether包，Ether包后面也可能是Ether包。采用发1收1的方法，同时要看ether_last还剩下多少，ether_last收一个就减去一个。
        case PacketType::ProcInfoResponse:
        case PacketType::EtherInfoResponse: {
            if (client.ether_last == 0) {
                client.packet_type = PacketType::PrintQueueResponse;
                return true;
            }
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x05;
            header.packet_size = htons(8);

            // packet struct
            SysInfoRequestPacket pkt;
            pkt.port = htons(client.ether_last-1);
            pkt.payload_size = 0;

            vector<pair<uint8_t*, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            client.send_msg(buffer);

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
            recved_pkt.port = ntohs(recved_pkt.port);
            if (packet.header.port != client.ether_last-1) {
                LERR << "收到的以太口错误，理想=" << (u_int)client.ether_last-1 << "，实际=" << hex << (u_int)recved_pkt.port << endl;
                return false;
            }
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
            
            // writeToDataBase(client, packet);
            client.ether_last--;
            return true;
        }
        case PacketType::PrintQueueResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x09;
            header.packet_size = htons(8);

            // packet struct
            SysInfoRequestPacket pkt;
            pkt.payload_size = 0;

            vector<pair<*uint8_t, size_t>> buffer { 
                make_pair((uint8_t*)&header, sizeof(header)), 
                make_pair((uint8_t*)&pkt, sizeof(pkt)) 
            };
            send_msg(buffer);

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
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);
            recved_pkt.term_num = ntohs(recved_pkt.term_num);
            for (int i = 0; i < 16; i++) {
                client.dumb_term[i] = recved_pkt.dumb_term[i];
            }
            for (int i = 0; i < 254; i++) {
                client.ip_term[i] = recved_pkt.ip_term[i];
            }
            client.term_num = recved_pkt.term_num;
            writeToDataBase(client, packet);

            return true;
        }

        // TODO: 这里放哑终端的和IP终端的

        // 一切结束
        case PacketType::End_all: {
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

std::string logify_data(vector<uint8_t> & message) {
    std::stringstream ss, ss_word;
    u_int i;
    for (i = 0; i < message.size(); i++) {
        if (i % 16 == 0) {
            ss << ss_word.str() << std::endl;
            ss_word.clear();    //clear any bits set
            ss_word.str(std::string());
            ss << ' ' << setw(4) << setfill('0') << hex << i << ": ";
        }
        else if (i % 8 == 0) {
            ss << "- ";
        }
        ss << setw(2) << setfill('0') << hex << +message[i] << ' ';
        // print printable char.
        char ch = (message[i] > 31 && message[i] < 127) ? message[i] : '.';
        ss_word << ch;
    }  
    if (i%16==0){
        ss << setw(0) << ss_word.str();
    }
    else {
        auto interval = 3 * (16 - (i % 16)) + (i % 16 > 8 ? 0 : 2);
        ss << setw(interval) << setfill(' ') << ' ' << setw(0) << ss_word.str();
    }
    return ss.str();
}

std::string logify_data(const uint8_t* data, const int len) {
    std::stringstream ss, ss_word;
    // ss_word.str(std::string());
    int i;
    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            ss << ss_word.str() << std::endl;
            ss_word.clear();    //clear any bits set
            ss_word.str(std::string());
            ss << ' ' << setw(4) << setfill('0') << hex << uppercase << i << ": ";
        }
        else if (i % 8 == 0) {
            ss << "- ";
        }
        ss << setw(2) << setfill('0') << hex << uppercase << +data[i] << ' ';
        // print printable char.
        char ch = (data[i] > 31 && data[i] < 127) ? data[i] : '.';
        ss_word << ch;
        // ss_word << data[i];
    }  
    if (i%16==0){
        ss << setw(0) << ss_word.str();
    }
    else {
        auto interval = 3 * (16 - (i % 16)) + (i % 16 > 8 ? 0 : 2);
        // cout << "i: " << i << ", interval: " << interval << endl;
        ss << setw(interval) << setfill(' ') << ' ' << setw(0) << ss_word.str();
    }
    return ss.str();
}
