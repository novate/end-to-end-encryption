#include "client_communicate.hpp"

// DevInfo Client::gene_dev_info() {
void Client::gene_dev_info(int devid) {
    dev.cpu = 2600;
    dev.ram = 1846;
    dev.flash = 3723;
    
    srand((unsigned)time(nullptr));

    dev.instID = devid;
    dev.instInnID = 1;

    dev.devInnerID = rand() % 256;
    dev.devInnerID = dev.devInnerID << 8;
    dev.devInnerID += rand() % 256;

    create_random_str(dev.groupSeq, 16);
    create_random_str(dev.type, 16);
    create_random_str(dev.version, 16);

    // for(int i = 0; i < 256; i++)
    //     dev.IPTermMap[i] = (uint8_t)0;
    // return dev;
}

void Client::push_back_array(vector<uint8_t> & message, uint8_t * array, int length) {
    for(int i=0; i<length; i++) {
        message.push_back(array[i]);
    }
    return;
}

void Client::push_back_uint16(vector<uint8_t> & message, uint16_t data) {
    // auto var16 = htons(data);
    // message.push_back((uint8_t)(var16>>8));
    // message.push_back((uint8_t)(var16));
    message.push_back((uint8_t)(data>>8));
    message.push_back((uint8_t)(data));
}

void Client::push_back_uint32(vector<uint8_t> & message, uint32_t data) {
    // auto var32 = htonl(data);
    // message.push_back((uint8_t)(var32>>24));
    // message.push_back((uint8_t)(var32>>16));
    // message.push_back((uint8_t)(var32>>8));
    // message.push_back((uint8_t)(var32));
    message.push_back((uint8_t)(data>>24));
    message.push_back((uint8_t)(data>>16));
    message.push_back((uint8_t)(data>>8));
    message.push_back((uint8_t)(data));
    return;
}

void Client::pop_first_array(vector<uint8_t> & message, uint8_t * array, int length) {
    for(int i=0; i<length; i++) {
        array[i] = message.front();
        message.erase(message.begin());
    }
    return;
}

void Client::pop_first_uint8(vector<uint8_t> & message, uint8_t& data) {
    data = message.front();
    message.erase(message.begin());
}

void Client::pop_first_uint16(vector<uint8_t> & message, uint16_t& data) {
    data = message.front();
    message.erase(message.begin());
    data = data << 8;
    data += message.front();
    message.erase(message.begin());
    // data = ntohs(data);
}

void Client::pop_first_uint32(vector<uint8_t> & message, uint32_t& data) {
    data = message.front();
    message.erase(message.begin());
    data = data << 8;
    data += message.front();
    message.erase(message.begin());
    data = data << 8;
    data += message.front();
    message.erase(message.begin());
    data = data << 8;
    data += message.front();
    message.erase(message.begin());
    // data = ntohl(data);
}

void Client::push_back_screen_info(vector<uint8_t> & message, const uint8_t screen_num) {
    //screen 
    message.push_back((uint8_t)screen_num);
    //pad
    message.push_back((uint8_t)0x00);
    //remote server port
    push_back_uint16(message, (uint8_t)rawIPTerFlags);
    //remote server ip
    uint32_t ip = (uint32_t)inet_aton("192.168.80.2", nullptr);
    message.push_back((uint8_t)ip >> 24);
    message.push_back((uint8_t)ip >> 16);
    message.push_back((uint8_t)ip >> 8);
    message.push_back((uint8_t)ip);

    //proto
    uint8_t tp1[12] = "专用SSH";
    push_back_array(message, (uint8_t *)tp1, 12);

    //screen state
    uint8_t tp2[8] = "已登录";
    push_back_array(message, (uint8_t *)tp2, 8);
  
    //screen prompt
    uint8_t tp3[24] = "储蓄系统";
    push_back_array(message, (uint8_t *)tp3, 24);

    //tty type
    uint8_t tp4[12] = "vt100";
    push_back_array(message, (uint8_t *)tp4, 12);

    //system time 
    push_back_uint32(message, (uint32_t)time(nullptr));

    //statics
    push_back_uint32(message, (uint32_t)rand() % 1000000);
    push_back_uint32(message, (uint32_t)rand() % 1000000);
    push_back_uint32(message, (uint32_t)rand() % 1000000);
    push_back_uint32(message, (uint32_t)rand() % 1000000);

    //Ping statics
    push_back_uint32(message, (uint32_t)rand() % 123456);
    push_back_uint32(message, (uint32_t)rand() % 123456);
    push_back_uint32(message, (uint32_t)rand() % 123456);
    
}

bool Client::client_pack_message(PacketType type, const Options & opt) {
    vector<uint8_t> message;
    
    //head
    message.push_back((uint8_t)0x91);
    //descriptor
    message.push_back((uint8_t)type);

    switch(type) {
        case PacketType::VersionRequire:
        {
            //packet length
            push_back_uint16(message, (uint16_t)12);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)4);
            //main version
            push_back_uint16(message, serverMainVersion);
            //sec 1 version
            message.push_back(serverSec1Version);
            //sec 2 version
            message.push_back(serverSec2Version);
        }
            break;
        case PacketType::AuthResponse:
        {
            //packet length
            push_back_uint16(message, (uint16_t)116);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)108);       
            //CPU
            push_back_uint16(message, dev.cpu);
            //RAM
            push_back_uint16(message, dev.ram);
            //FLASH
            push_back_uint16(message, dev.flash);
            //dev inner seq
            push_back_uint16(message, dev.devInnerID);
 
            //group seq
            push_back_array(message, dev.groupSeq, 16);
            //type
            push_back_array(message, dev.type, 16);
            //version
            push_back_array(message, dev.version,16);

            //ethernet, usb, printer..., 8 bytes in total
            for(int ip = 0; ip < 8; ip++) {
                message.push_back((uint8_t)0);
            }

            //instID
            push_back_uint32(message, dev.instID);
            //instInnID
            message.push_back(dev.instInnID);
            //3 bytes pad
            for(int ip = 0; ip < 3; ip++) {
                message.push_back((uint8_t)0);
            }

            //authstr
            vector<uint8_t> tmp;
            uint8_t tpdata[104];
            vector<uint8_t>::iterator it;
             
            it = message.begin() + 8;   //from CPU

            for(; it != message.end(); it++) {       //to the last uint8_t in current message
                tmp.push_back(*it);
            }

            int n = tmp.size();
            if(n != 72) {
                LOG(Level::ERR) << "Descriptor: 0x91" << endl
                    << "PacketType: 0x01" << endl << "data length from CPU to AuthStr is not 72!!" 
                        << "data length = "  << n << endl;
                    return false;
            }
            for(int i = 0; i < 72; i++) {
                tpdata[i] = tmp[i];
            }
            uint8_t authStr[33] = "yzmond:id*str&to!tongji@by#Auth^";
            for(int i = 0; i < 32; i++) {
                tpdata[72+i] = authStr[i];
            }
            
            //encrypt
            u_int random_num=0, svr_time=0;
            encrypt_auth(random_num, svr_time, tpdata, 104);

            it = message.begin() + 8;
            for(int i = 0; i < 72; i++) {   // replace old 72 bytes
                message[i+8] = tpdata[i];
            }

            for(int i = 72; i < 104; i++) {    //add new auth str (32 bytes)    
                message.push_back(tpdata[i]);
            }

            //random num
            push_back_uint32(message, (uint32_t)random_num);
        }
            break;

        case PacketType::SysInfoResponse:
        {
            //packet length
            push_back_uint16(message, (uint16_t)28);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)20); 
            //user CPU time
            push_back_uint32(message, (uint32_t)5797); 
            //nice CPU time
            push_back_uint32(message, (uint32_t)0); 
            //system CPU time
            push_back_uint32(message, (uint32_t)13013); 
            //idle CPU time
            push_back_uint32(message, (uint32_t)5101426);
            //freed memory
            push_back_uint32(message, (uint32_t)2696);
        }
            break;

        case PacketType::ConfInfoResponse:
        {
            //load config.dat
            char * read_file = new char[8192];
            string file_name = "config.dat";
            int size = read_dat(file_name, read_file, 8191);
            if (size < 0) {
                LOG(Level::ERR) << "No such file: " << file_name << endl;
                return -1;
            }
            LOG(Level::RDATA) << "read test:"  << logify_data(reinterpret_cast<uint8_t*>(read_file), size) << endl;
            LOG(Level::Debug) << "size of data: " << size << endl;
            
            // add '\0' to the end of string
            read_file[size++] = '\0';

            //packet length
            push_back_uint16(message, (uint16_t)size + 8);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)size); 

            //config info
            push_back_array(message, (uint8_t *)read_file, size);
        }
            break;

        case PacketType::ProcInfoResponse:
        {
            //load config.dat
            char * read_file = new char[8192];
            string file_name = "process.dat";
            int size = read_dat(file_name, read_file, 8191);
            if (size < 0) {
                LOG(Level::ERR) << "No such file: " << file_name << endl;
                return -1;
            }
            LOG(Level::RDATA) << "read test:"  << logify_data(reinterpret_cast<uint8_t*>(read_file), size) << endl;
            LOG(Level::Debug) << "size of data: " << size << endl;
            
            // add '\0' to the end of string
            read_file[size++] = '\0';

            //packet length
            push_back_uint16(message, (uint16_t)size + 8);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)size); 

            //config info
            push_back_array(message, (uint8_t *)read_file, size);      
        }
            break;
        
        case PacketType::USBfileResponse:
        {
            //load usefile.dat
            char * read_file = new char[8192];
            string file_name = "usefile.dat";
            int size = read_dat(file_name, read_file, 4095);
            if (size < 0) {
                LOG(Level::ERR) << "No such file: " << file_name << endl;
                return -1;
            }
            LOG(Level::RDATA) << "read test:"  << logify_data(reinterpret_cast<uint8_t*>(read_file), size) << endl;
            LOG(Level::Debug) << "size of data: " << size << endl;
            
            // add '\0' to the end of string
            read_file[size++] = '\0';

            //packet length
            push_back_uint16(message, (uint16_t)size + 8);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)size); 

            //config info
            push_back_array(message, (uint8_t *)read_file, size); 
        }
            break;
        
        case PacketType::PrintQueueResponse:
        {
            //packet length
            push_back_uint16(message, (uint16_t)9);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)1);

            message.push_back((uint8_t)0);    
        }             
            break;
        
        case PacketType::TerInfoResponse:
        {
            uint8_t ttyInfoMap[270] = {0};    //dumb+IP terminal map
            
            // for(int i = 0; i < 16; i++) {
            //     ttyInfoMap[i] = 0;
            // }   //no dumb terminal

            //TODO: get max&min tty amount from Options
            int maxTNum = stoi(opt.at("最大配置终端数量"));
            int minTNum = stoi(opt.at("最小配置终端数量"));
            LOG(Level::Debug) << "max tty amount = " << maxTNum << endl;
            LOG(Level::Debug) << "min tty amount = " << minTNum << endl;
            
            int total = minTNum + rand() % (maxTNum - minTNum + 1);
            int async_term_num = 0; 
            terCount = total - async_term_num;
            
            srand((unsigned)time(nullptr));
            int randPos;    //generate random ttyInfoMap
            for(u_int i = 0; i < terCount; i++) {
                randPos = rand() % 254;
                while(ttyInfoMap[16 + randPos] == 1) {
                    randPos = rand() % 254;
                }
                ttyInfoMap[16 + randPos] = 1;
                // dev.IPTermMap[randPos] = (uint8_t)1;
            }

            //16 dumb-terminal, 254 ip-terminal, 2 bytes tty num
            //packet length
            push_back_uint16(message, (uint16_t)(8+16+254+2));
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)(16+254+2));
            //tty map    
            push_back_array(message, ttyInfoMap, 270);
            //tty configed
            push_back_uint16(message, (uint16_t)(total + rand() % (270-total) ));
        }
            break;

        case PacketType::DumbTerResponse:
        {
            int maxSNum = stoi(opt.at("每个终端最大虚屏数量"));
            int minSNum = stoi(opt.at("每个终端最小虚屏数量"));
            LOG(Level::Debug) << "max screen num = " << maxSNum << endl;
            LOG(Level::Debug) << "min screen num = " << minSNum << endl;
            
            srand((unsigned)time(nullptr));
            uint8_t screenNum = minSNum + rand() % (maxSNum - minSNum + 1);
            screenCount += screenNum;
            uint8_t activeScreen = rand() % screenNum;

            //packet length
            push_back_uint16(message, (uint16_t)(8 + 28 + screenNum*96));
            //dumb terminal number
            push_back_uint16(message, (uint16_t)(1 + rand() % 16));
            //data length
            push_back_uint16(message, (uint16_t)(28 + screenNum*96));

            //port
            // message.push_back((uint8_t)(1 + rand() % 254) );
            message.push_back(rawIPTerFlags);

            //asigned port
            message.push_back(rawIPTerFlags);
            
            // message.push_back((uint8_t)(1 + rand() % 254) );
            //active screen
            message.push_back(activeScreen);
            //screen numv
            message.push_back(screenNum);

            //tty addr
            push_back_uint32(message, (uint32_t)0);

            //tty type
            uint8_t tp5[12] = "串口终端";
            push_back_array(message, (uint8_t *)tp5, 12);
            
            //tty state
            uint8_t tp6[8] = "正常";
            push_back_array(message, (uint8_t *)tp6, 8);

            //screen info
            for(uint8_t i = 1; i <= screenNum; i++) {
                push_back_screen_info(message, i);
            }
        }
            break;

        case PacketType::IPTermResponse:
        {
            int maxSNum = stoi(opt.at("每个终端最大虚屏数量"));
            int minSNum = stoi(opt.at("每个终端最小虚屏数量"));
            LOG(Level::Debug) << "max screen num = " << maxSNum << endl;
            LOG(Level::Debug) << "min screen num = " << minSNum << endl;
            srand((unsigned)time(nullptr));
            uint8_t screenNum = minSNum + rand() % (maxSNum - minSNum + 1);
            screenCount += screenNum;
            uint8_t activeScreen = rand() % screenNum;

            //packet length
            push_back_uint16(message, (uint16_t)(8 + 28 + screenNum*96));
            //rawIPTerFlags
            push_back_uint16(message, (uint16_t)rawIPTerFlags);
            //data length
            push_back_uint16(message, (uint16_t)(28 + screenNum*96));
            //port
            message.push_back((uint8_t)rawIPTerFlags);
            //asigned port
            message.push_back((uint8_t)rawIPTerFlags);
            //active screen
            message.push_back(activeScreen);
            //screen numv
            message.push_back(screenNum);

            //tty addr
            uint32_t ip = (uint32_t)inet_aton("192.168.80.2", nullptr);
            message.push_back((uint8_t)ip >> 24);
            message.push_back((uint8_t)ip >> 16);
            message.push_back((uint8_t)ip >> 8);
            message.push_back((uint8_t)ip);

            //tty type
            uint8_t tp7[12] = "IP终端";
            push_back_array(message, (uint8_t *)tp7, 12);
            
            //tty state
            uint8_t tp8[8] = "菜单";
            push_back_array(message, (uint8_t *)tp8, 8);

            //screen info
            for(uint8_t i = 1; i <= screenNum; i++) {
                push_back_screen_info(message, i);
            }
        }
            break;
        
        case PacketType::End:
        {
            //packet length
            push_back_uint16(message, (uint16_t)8);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)0);      
        }      
            break;
        default:
            break;
    }

    send_message = message;
    return true;
}

bool Client::client_unpack_message(const int socketfd, const Options & opt) {
    vector<uint8_t> message = recv_message;

    uint8_t front;
    pop_first_uint8(message, front);
    if(front != 0x11) {
        LOG(Level::ERR) << "从服务器收到的包的包头错误，收到的包头为： " << front << endl;
        return false;
    }

    pop_first_uint8(message, recvPacketType);
    sendPacketType = recvPacketType;

    switch(static_cast<PacketType>(recvPacketType)) {
        case PacketType::AuthRequest:
        {
            uint16_t total_length;
            pop_first_uint16(message, total_length);
            uint16_t padding16;
            pop_first_uint16(message, padding16);
            uint16_t data_length;
            pop_first_uint16(message, data_length);
            pop_first_uint16(message, rawServerMainVersion);
            pop_first_uint8(message, rawServerSec1Version);
            pop_first_uint8(message, rawServerSec2Version);
            if (rawServerMainVersion < 0x02) {
                LOG(Level::ERR) << "服务器版本号不符合要求，服务器版本为：" 
                    << rawServerMainVersion << "." << rawServerSec1Version << "." << rawServerSec2Version << "."<< endl;
                //reply: versionRequire
                client_pack_message(PacketType::VersionRequire, opt);
                send_msg(socketfd);
                //wait for server to close TCP connection
                return true;
            }
            pop_first_uint16(message, rawFailInterval);
            pop_first_uint16(message, rawAnotherInterval);
            pop_first_uint8(message, rawPermitEmptyTerminal);
            uint8_t padding8;
            pop_first_uint8(message, padding8);
            pop_first_uint16(message, padding16);
            uint8_t server_auth[32];
            pop_first_array(message, server_auth, 32);
            uint32_t random_num, svr_time;
            pop_first_uint32(message, random_num);
            pop_first_uint32(message, svr_time);
            // LOG(Level::Debug) << "server_auth: \n" << logify_data(server_auth, 32) << endl;
            if (decrypt_auth(random_num, server_auth, 32) == false) {
                LOG(Level::ERR) << "服务器认证错误" << endl;
                LOG(Level::ERR) << hex << random_num << endl;
                return false;
            }
            else {
                LOG(Level::ENV) << "服务器认证通过" << endl;
                return true;
            }
        }
        case PacketType::SysInfoRequest:
        {
            LOG(Level::ENV) << "服务器请求系统信息" << endl;
            return true;
        }
        case PacketType::ConfInfoRequest:
        {
            LOG(Level::ENV) << "服务器请求配置信息" << endl;
            return true;
        }
        case PacketType::ProcInfoRequest:
        {
            LOG(Level::ENV) << "服务器请求进程信息" << endl;
            return true;
        }
        case PacketType::EtherInfoRequest:
            LOG(Level::ENV) << "服务器请求以太网口信息" << endl;
            return true;
        case PacketType::USBInfoRequest:
        {
            LOG(Level::ENV) << "服务器请求USB口信息" << endl;
            return true;
        }
        case PacketType::USBfileRequest:
        {
            LOG(Level::ENV) << "服务器请求U盘信息" << endl;
            return true;
        }
        case PacketType::PrintDevRequest:
        {
            LOG(Level::ENV) << "服务器请求打印口信息" << endl;
            return true;
        }
        case PacketType::PrintQueueRequest:
        {
            LOG(Level::ENV) << "服务器请求打印队列信息" << endl;
            return true;
        }
        case PacketType::TerInfoRequest:
        {
            LOG(Level::ENV) << "服务器请求终端服务信息" << endl;
            return true;
        }
        case PacketType::DumbTerRequest:
        {
            LOG(Level::ENV) << "服务器请求哑终端信息" << endl;
            uint16_t padding;
            pop_first_uint16(message, padding);
            pop_first_uint16(message, rawDumbTerFlags);
            return true;
        }
        case PacketType::IPTermRequest:
        {
            LOG(Level::ENV) << "服务器请求IP终端信息" << endl;
            uint16_t padding;
            pop_first_uint16(message, padding);
            pop_first_uint16(message, rawIPTerFlags);
            return true;
        }
        case PacketType::End:
        {
            LOG(Level::ENV) << "服务器提示已收到全部包" << endl;
            return true;
        }
        default:
        {
            LOG(Level::ERR) << "不可被识别的包类型" << endl;
            return false;
        }
    }
}

int Client::recv_msg(int socketfd) {
    int n;
    uint8_t head[8];
    //recv head
    if((n = recv(socketfd, head, 8, 0)) == -1) {
        LOG(Level::ERR) << "recv head return error!" << endl;
        return false;
    }
    if(n == 0) {
        LOG(Level::ENV) << "remote server has closed connection." << endl;
        return false;
    }
    if(n != 8) {
        LOG(Level::ERR) << "recved incorrect head length! Expected: 8 "
            << "recved: " << n << endl;
        return false;       
    }
    
    recvPacketType = head[1];

    //recv data
    uint16_t * pt;
    pt = (uint16_t*)&head[6];
    int dataLength = (int)ntohs(*pt);
    if(dataLength > 0) {
        if((n = recv(socketfd, recv_buffer, dataLength, 0)) == -1) {
            LOG(Level::ERR) << "recv packet data return error!" << endl;
            return false;
        }
        if(n == 0) {
            LOG(Level::ENV) << "remote server has closed connection." << endl;
            return false;
        }       
        if(n != dataLength) {
            LOG(Level::ERR) << "recved incorrect data length! Expected: " << dataLength
                << "recved: " << n << endl;
            return false;       
        }
    } //end of if

    vector<uint8_t>().swap(recv_message);
    if(recv_message.size() != 0) {
        LOG(Level::Debug) << "clear vector failed." << endl;
        return false;
    }

    for(int i = 0; i < 8; i++) {
        recv_message.push_back(head[i]);
    }
    for(int i = 0; i < dataLength; i++) {
        recv_message.push_back(recv_buffer[i]);
    }

    return true;
}

int Client::send_msg(int socketfd) {
    //send head
    int n;
    uint8_t head[8];
    for(int i = 0; i < 8; i++) {
        head[i] = send_message[i];
    }
    send_message.erase(send_message.begin(), send_message.begin()+8);
    
    if((n = send(socketfd, head, 8, 0)) == -1) {
        LOG(Level::ERR) << "send head return error!" << endl;
        return false;
    }
    if(n == 0) {
        LOG(Level::ENV) << "remote server has closed connection." << endl;
        return false;
    }
    if(n != 8) {
        LOG(Level::ERR) << "sent incorrect head length! Expected: 8 "
            << "sent: " << n << endl;
        return false;       
    }

    //send data
    vector<uint8_t>::iterator it;
    int pos = 0;
    for(it = send_message.begin(); it != send_message.end(); it++) {
        send_buffer[pos++] = *it;
    }

    send_message.erase(send_message.begin(), send_message.end());
    if(send_message.size() != 0) {
        LOG(Level::Debug) << "clear vector failed." << endl;
        return false;
    }

    uint16_t * pt;
    pt = (uint16_t*)&head[6];
    int dataLength = (int)htons(*pt);
    if(dataLength > 0) {
        if((n = send(socketfd, send_buffer, dataLength, 0)) == -1) {
            LOG(Level::ERR) << "send packet data return error!" << endl;
            return false;
        }
        if(n == 0) {
            LOG(Level::ENV) << "remote server has closed connection." << endl;
            return false;
        }       
        if(n != dataLength) {
            LOG(Level::ERR) << "sent incorrect data length! Expected: " << dataLength
                << "sent: " << n << endl;
            return false;       
        }
    } //end of if

    return true;
}

int Client::client_communicate(int socketfd, const Options & opt, std::ofstream& xls_stream) {
    srand((unsigned)time(nullptr));

    if(recv_msg(socketfd) == true) {
        LOG(Level::SPACK) << "从服务器收到数据\n" << endl;
    }
    else {
        //TODO: disconnect
    }

    u_int raw_unpack_size = recv_message.size();
    uint8_t *raw_unpack = new uint8_t[raw_unpack_size];
    for (u_int i = 0; i < raw_unpack_size; i++) {
        raw_unpack[i] = recv_message[i];
    }
    LOG(Level::RDATA) << "收到数据为：\n" << logify_data(raw_unpack, raw_unpack_size) << endl;
    client_unpack_message(socketfd, opt);

    while(static_cast<PacketType>(recvPacketType) != PacketType::End) {
        client_pack_message(static_cast<PacketType>(sendPacketType), opt);
        u_int raw_pack_size = send_message.size();
        uint8_t *raw_pack = new uint8_t[raw_pack_size];
        for (u_int i = 0; i < raw_pack_size; i++) {
            raw_pack[i] = send_message[i];
        }
        LOG(Level::SDATA) << "发送数据为：\n" << logify_data(raw_pack, raw_pack_size) << endl;
        
        if(send_msg(socketfd) == true) {
            LOG(Level::ENV) << "将缓冲区内容发送给服务器" << endl;
        }
        else {
            // disconnect and reconnect if the return value is not 0.
            return 1;
        }
        
        if(recv_msg(socketfd) == true) {
            LOG(Level::ENV) << "从服务器收到数据" << endl;
        }
        else {
            // disconnect and reconnect if the return value is not 0.
            return 1;
        }
        
        client_unpack_message(socketfd, opt);
    }
    client_pack_message(static_cast<PacketType>(sendPacketType), opt);
    u_int raw_pack_size = send_message.size();
    uint8_t *raw_pack = new uint8_t[raw_pack_size];
    for (u_int i = 0; i < raw_pack_size; i++) {
        raw_pack[i] = send_message[i];
        }
    LOG(Level::SDATA) << "发送数据为：\n" << logify_data(raw_pack, raw_pack_size) << endl;
        
    if(send_msg(socketfd) == true) {
        LOG(Level::ENV) << "完成所有发包，客户端退出" << endl;
        time_t now = time(0);
        char timestamp[100] = "";
        strftime(timestamp, 100, "%T", localtime(&now));
        std::string cur_time(timestamp);
        xls_stream << cur_time << '\t' << dev.instID << '\t' << "1\t" << terCount << '\t' << screenCount << std::endl;
        return 0;
    }
    else {
        // disconnect and reconnect if the return value is not 0.
        return 1;
    }
    return 1;
}
