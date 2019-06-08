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

            return true;
        }
        case PacketType::AuthResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x02;
            uint16_t packet_size = 8;
            header.packet_size = htons(packet_size);

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
            recved_pkt.freed_cpu_time = ntohl(recved_pkt.freed_cpu_time);

            writeToDataBase(client, packet);

            return true;
        }
        case PacketType::SysInfoResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x03;
            uint16_t packet_size = 8;
            header.packet_size = htons(packet_size);

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
            if (packet.header.packet_type != 0x03) {
                LERR << "收到的包类型错误，理想=0x03，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }

            writeToDataBase(client, packet);

            return true;
        }
        case PacketType::ConfInfoResponse: {
            // construct message
            // header
            PacketHeader header;
            // packet_type: hton
            header.direction = 0x11;
            header.packet_type = 0x04;
            uint16_t packet_size = 8;
            header.packet_size = htons(packet_size);

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
            if (packet.header.packet_type != 0x04) {
                LERR << "收到的包类型错误，理想=0x04，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }

            writeToDataBase(client, packet);

            return true;
        }
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
            uint16_t packet_size = 8;
            header.packet_size = htons(packet_size);

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
            if (packet.header.packet_type != 0x05) {
                LERR << "收到的包类型错误，理想=0x05，实际=0x" << hex <<  (u_int)packet.header.packet_type << endl;
                return false;
            }
            EtherInfoResponsePacket &recved_pkt = *((EtherInfoResponsePacket*)packet.payload.first);
            recved_pkt.payload_size = ntohs(recved_pkt.payload_size);

            writeToDataBase(client, packet);
            client.ether_last--;
            return true;
        }

        default:
            // error
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
