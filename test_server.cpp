#include "communicate.hpp"

int log_init(std::ofstream &log_stream, const std::string log_name, const Level level, const bool* const log_env, const bool on_screen, const bool is_trunc) {
    // log_stream must not be opened before getting into this function.
    if (log_stream.is_open()) {
        return -1;
    }
	if (is_trunc) {
		log_stream.open(log_name, ios::out|ios::trunc);
	}
    else {
		log_stream.open(log_name, ios::out|ios::app);
	}
    if (!log_stream.is_open()) {
        return -2;
    }
    Log::get().setLogStream(log_stream);
    Log::get().setLevel(level);
    Log::get().setEnv(log_env);
    Log::get().setOnScreen(on_screen);
    return 0;
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

void create_random_str(uint8_t* random_string, const int length) {
    uint8_t *p = new uint8_t[length];
    srand((unsigned)time(NULL));
    for(int i = 0; i < length; i++) {
        p[i] = rand() % 256;
    }
    memcpy(random_string, p, length);
    delete[] p;
    return;
}

// return size of the file, if open file error, return 0.
// must read no more than maxLength(8191/4095) bytes.
int read_dat(const std::string dat_name, char* result, int maxLength) {
	ifstream ifs;
	ifs.open(dat_name, std::ifstream::in);
	if (!ifs.is_open()) {
		return -1;
	}
	// read no more than 8191 bytes.
	ifs.seekg(0, std::ios::end);
	int length = ifs.tellg();
	length = length > maxLength ? maxLength : length;
	ifs.seekg(0, std::ios::beg);
	ifs.read(result, length);
	ifs.close();
	return length;
}

// TODO: Is this necessary?
DevInfo Server::gene_dev_info() {
    dev.cpu = 2600;
    dev.ram = 1846;
    dev.flash = 3723;
    
    srand((unsigned)time(NULL));
    //'devid' in database
    //TODO: need unique instID(devid)! and I'm not doing this.
    dev.instID = rand() % 512;
    dev.instInnID = 1;

    dev.devInnerID = rand() % 256;
    dev.devInnerID = dev.devInnerID << 8;
    dev.devInnerID += rand() % 256;

    create_random_str(dev.groupSeq, 16);
    create_random_str(dev.type, 16);
    create_random_str(dev.version, 16);
    return dev;
}

void Server::push_back_array(vector<uint8_t> & message, uint8_t * array, int length) {
    for(int i=0; i<length; i++) {
        message.push_back(array[i]);
    }
    return;
}

void Server::push_back_uint16(vector<uint8_t> & message, uint16_t data) {
    auto var16 = inet_htons(data);
    message.push_back((uint8_t)(var16>>8));
    message.push_back((uint8_t)(var16));
}

void Server::push_back_uint32(vector<uint8_t> & message, uint32_t data) {
    auto var32 = inet_htonl(data);
    message.push_back((uint8_t)(var32>>24));
    message.push_back((uint8_t)(var32>>16));
    message.push_back((uint8_t)(var32>>8));
    message.push_back((uint8_t)(var16));
    return;
}

void Server::pop_first_array(vector<uint8_t> & message, uint8_t * array, int length) {
    for(int i=0; i<length; i++) {
        array[i] = message.front();
        message.erase(message.begin());
    }
    return;
}

void Server::pop_first_uint8(vector<uint8_t> & message, uint8_t& data) {
    data = message.front();
    message.erase(message.begin());
}

void Server::pop_first_uint16(vector<uint8_t> & message, uint16_t& data) {
    uint8_t raw[2];
    raw[1] = message.front();
    message.erase(message.begin());
    raw[2] = message.front();
    message.erase(message.begin());
    memcpy(&data, sizeof(data), raw, 2);
    data = inet_ntohs(data);
}

void Server::pop_first_uint32(vector<uint8_t> & message, uint32_t& data) {
    uint8_t raw[4];
    raw[1] = message.front();
    message.erase(message.begin());
    raw[2] = message.front();
    message.erase(message.begin());
    raw[3] =message.front();
    message.erase(message.begin());
    raw[4] = message.front();
    message.erase(message.begin());
    memcpy(&data, sizeof(data), raw, 4);
    data = inet_ntohl(data);
}

// TODO: Is this necessary?
void Server::push_back_screen_info(vector<uint8_t> & message) {
    //screen 
    message.push_back((uint8_t)(1 + rand() % 16));
    //pad
    message.push_back((uint8_t)0x00);
    //remote server port
    //TODO: HOW TO GET REMOTE PORT??
    push_back_uint16(message, (uint16_t)12350);
    //remote server ip
    //TODO: HOW TO GET REMOTE IP??
    uint32_t ip = (uint32_t)inet_aton("192.168.0.0");
    message.push_back((uint8_t)ip >> 24);
    message.push_back((uint8_t)ip >> 16);
    message.push_back((uint8_t)ip >> 8);
    message.push_back((uint8_t)ip);

    //proto
    string prot = "专用SSH";
    const char tp[12] = {0};
    tp = prot.c_str();
    push_back_array(message, (uint8_t *)tp, 12);

    //screen state
    string state = "已登录";
    const char tp[8] = {0};
    tp = prot.c_str();
    push_back_array(message, (uint8_t *)tp, 8);
  
    //screen prompt
    string promp = "储蓄系统";
    const char tp[24] = {0};
    tp = promp.c_str();
    push_back_array(message, (uint8_t *)tp, 24);

    //tty type
    string ttyType = "vt100";
    const char tp[12] = {0};
    tp = ttyType.c_str();
    push_back_array(message, (uint8_t *)tp, 12);

    //system time 
    push_back_uint32(message, (uint32_t)time(NULL));

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

// TODO: Rewrite it symmetrically as client_pack message.
void Server::server_unpack_message(Options opt) {
    vector<uint8_t> message;
    
    //head
    message.push_back((uint8_t)0x91);
    //descriptor
    message.push_back((uint8_t)type);

    switch(type) {
        case PacketType::VersionRequire:
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
            break;

        case PacketType::AuthResponse:
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
            //2 bytes pad
            for(int ip = 0; ip < 2; ip++) {
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
                LOG(Level::Error) << "Descriptor: 0x91" << endl
                    << "PacketType: 0x01" << endl << "the bit num from CPU to AuthStr is not 72!!" << endl;
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
            break;

        case PacketType::SysInfoResponse:
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
            break;

        case PacketType::ConfInfoResponse:
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
            break;

        case PacketType::ProcInfoResponse:
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
            break;
        
        case PacketType::USBfileResponse:
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
            break;
        
        case PacketType::PrintQueueResponse:
            //packet length
            push_back_uint16(message, (uint16_t)9);
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)1);

            message.push_back((uint8_t)0);                 
            break;
        
        case PacketType::TerInfoResponse:
            uint8_t ttyInfoMap[270] = {0};    //dumb+IP terminal map
            
            // for(int i = 0; i < 16; i++) {
            //     ttyInfoMap[i] = 0;
            // }   //no dumb terminal

            //TODO: get max&min tty amount from Options
            int maxTNum = stoi(opt.at["最大配置终端数量"]);
            int minTNum = stoi(opt.at["最小配置终端数量"]);
            LOG(Level::Debug) << "max tty amount = " << maxSNum << endl;
            LOG(Level::Debug) << "min tty amount = " << minSNum << endl;
            
            int total = minTNum + rand() % (maxTNum - minTNum + 1);
            int async_term_num = 0; 
            int ipterm_num = total - async_term_num;

            int randPos;    //generate random ttyInfoMap
            for(int i = 0; i < ipterm_num; i++) {
                randPos = rand() % 254;
                while(ttyInfoMap[16 + randPos] == 1) {
                    randPos = rand() % 254;
                }
                ttyInfoMap[16 + randPos] = 1;
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
            break;

        case PacketType::DumbTerResponse:
            int maxSNum = stoi(opt.at["每个终端最大虚屏数量"]);
            int minSNum = stoi(opt.at["每个终端最小虚屏数量"]);
            LOG(Level::Debug) << "max screen num = " << maxSNum << endl;
            LOG(Level::Debug) << "min screen num = " << minSNum << endl;

            uint8_t screenNum = minSNum + rand() % (maxSNum - minSNum + 1);
            uint8_t activeScreen = rand() % screenNum;

            //packet length
            push_back_uint16(message, (uint16_t)(8 + 28 + screenNum*96));
            //dumb terminal number
            push_back_uint16(message, (uint16_t)(1 + rand() % 16));
            //data length
            push_back_uint16(message, (uint16_t)(28 + screenNum*96));
            //port
            message.push_back((uint8_t)(1 + rand() % 254) );
            //asigned port
            message.push_back((uint8_t)(1 + rand() % 254) );
            //active screen
            message.push_back(activeScreen);
            //screen numv
            message.push_back(screenNum);

            //tty addr
            push_back_uint32(message, (uint32_t)0);

            //tty type
            string ttyType = "串口终端";
            const char tp[12] = {0};
            tp = ttyType.c_str();
            push_back_array(message, (uint8_t *)tp, 12);
            
            //tty state
            string ttyState = "正常";
            memset(tp, 0, 12);
            tp = ttyState.c_str();
            push_back_array(message, (uint8_t *)tp, 8);

            //screen info
            for(int i = 0; i < screenNum; i++) {
                push_back_screen_info(message);
            }

            break;

        case PacketType::IPTermResponse:
            int maxSNum = stoi(opt.at["每个终端最大虚屏数量"]);
            int minSNum = stoi(opt.at["每个终端最小虚屏数量"]);
            LOG(Level::Debug) << "max screen num = " << maxSNum << endl;
            LOG(Level::Debug) << "min screen num = " << minSNum << endl;

            uint8_t screenNum = minSNum + rand() % (maxSNum - minSNum + 1);
            uint8_t activeScreen = rand() % screenNum;

            //packet length
            push_back_uint16(message, (uint16_t)(8 + 28 + screenNum*96));
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)(28 + screenNum*96));
            //port
            message.push_back((uint8_t)(1 + rand() % 254) );
            //asigned port
            message.push_back((uint8_t)(1 + rand() % 254) );
            //active screen
            message.push_back(activeScreen);
            //screen numv
            message.push_back(screenNum);

            //tty addr
            uint32_t ip = (uint32_t)inet_aton("192.168.80.2");
            message.push_back((uint8_t)ip >> 24);
            message.push_back((uint8_t)ip >> 16);
            message.push_back((uint8_t)ip >> 8);
            message.push_back((uint8_t)ip);

            //tty type
            string ttyType = "IP终端";
            const char tp[12] = {0};
            tp = ttyType.c_str();
            push_back_array(message, (uint8_t *)tp, 12);
            
            //tty state
            string ttyState = "菜单";
            memset(tp, 0, 12);
            tp = ttyState.c_str();
            push_back_array(message, (uint8_t *)tp, 8);

            //screen info
            for(int i = 0; i < screenNum; i++) {
                push_back_screen_info(message);
            }

            break;
        
        case PacketType::End:
            //packet length
            push_back_uint16(message, (uint16_t)8;
            //0x0000
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)0);            
            break;
        default:
            break;
    }

    send_message = message;
    return true;
}

// TODO: Rewrite it symmetrically as client_unpack message.
bool Server::server_pack_message(PacketType type, Options opt) {
    vector<uint8_t> message;

    //head
    message.push_back((uint8_t)0x91);
    //descriptor
    message.push_back((uint8_t)type);

    switch(type) {
        case PacketType::AuthRequest:
            //packet length
            push_back_uint16(message, (uint16_t)60);
            //padding
            push_back_uint16(message, (uint16_t)0x0000);
            //data length
            push_back_uint16(message, (uint16_t)52);
            //main version
            push_back_uint16(message, (uint16_t)3);
            //sec 1 version
            message.push_back((uint8_t)0);
            //sec 2 version
            message.push_back((uint8_t)0);
            break;
            // 设备连接间隔
            push_back_uint16(message, static_cast<uint16_t>(stoi(opt.at["设备连接间隔"]));
            // 设备连接间隔
            push_back_uint16(message, static_cast<uint16_t>(stoi(opt.at["设备连接间隔"]));


            uint16_t data_length;
            pop_first_uint16(message, data_length);
            pop_first_uint16(message, rawServerMainVersion);
            pop_first_uint8(message, rawServerSec1Version);
            pop_first_uint8(message, rawServerSec2Version);
            if (rawServerMainVersion < 0x02) {
                LOG(Level::ERR) << "服务器版本号不符合要求，服务器版本为：" << rawServerMainVersion << "." << rawServerSec1Version << "." << rawServerSec2Version << "."<< endl;
                return false;
            }
            pop_first_uint8(message, rawPermitEmptyTerminal);
            uint8_t padding8;
            pop_first_uint8(message, padding8);
            pop_first_uint16(message, padding16);
            uint8_t server_auth[32];
            pop_first_array(message, server_auth, 32);
            uint32_t random_num, svr_time;
            pop_first_uint32(message, random_num);
            pop_first_uint32(message, svr_time);
            if (decrypt_auth(random_num, server_auth, 32) == false) {
                LOG(Level::ERR) << "服务器认证错误" << endl;
                return false;
            }
            else {
                LOG(Level::ENV) << "服务器认证通过" << endl;
                return true;
            }
        case PacketType::SysInfoRequest:
            LOG(Level::ENV) << "服务器请求系统信息" << endl;
            return true;
        case PacketType::ConfInfoRequest:
            LOG(Level::ENV) << "服务器请求配置信息" << endl;
            return true;
        case PacketType::ProcInfoRequest:
            LOG(Level::ENV) << "服务器请求进程信息" << endl;
            return true;
        case PacketType::EtherInfoRequest:
            LOG(Level::ENV) << "服务器请求以太网口信息" << endl;
            return true;
        case PacketType::USBInfoRequest:
            LOG(Level::ENV) << "服务器请求USB口信息" << endl;
            return true;
        case PacketType::USBfileRequest:
            LOG(Level::ENV) << "服务器请求U盘信息" << endl;
            return true;
        case PacketType::PrintDevRequest:
            LOG(Level::ENV) << "服务器请求打印口信息" << endl;
            return true;
        case PacketType::PrintQueueRequest:
            LOG(Level::ENV) << "服务器请求打印队列信息" << endl;
            return true;
        case PacketType::TerInfoRequest:
            LOG(Level::ENV) << "服务器请求终端服务信息" << endl;
            return true;
        case PacketType::DumbTerRequest:
            LOG(Level::ENV) << "服务器请求哑终端信息" << endl;
            pop_first_uint16(message, rawDumbTerFlags);
            return true;
        case PacketType::IPTermRequest:
            LOG(Level::ENV) << "服务器请求IP终端信息" << endl;
            pop_first_uint16(message, rawIPTerFlags);
            return true;
        case PacketType::End:
            LOG(Level::ENV) << "服务器提示已收到全部包" << endl;
            return true;
        default:
            LOG(Level::ERR) << "不可被识别的包类型" << endl;
            return false;
    }
}

// TODO: now just pseudo.
int Server::server_communicate(int socketfd, Options opt) {
    srand((unsigned)time(NULL));
    
    //block
    recv();
    client_unpack_message();
    while(recvPakcet != PacketType::End) {
        //recv & send message
        client_pack_message();
        send();
        recv();
        client_unpack_message();
    }

    return 0;
}
