#include "communicate.hpp"


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

DevInfo Client::gene_dev_info() {
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

void Client::push_back_array(vector<uint8_t> & message, uint8_t * array, int length) {
    for(int i=0; i<length; i++) {
        message.push_back(array[i]);
    }
    return;
}

void Client::push_back_uint16(vector<uint8_t> & message, uint16_t data) {
    var16 = inet_htons(data);
    message.push_back((uint8_t)(var16>>8));
    message.push_back((uint8_t)(var16));
}

void Client::push_back_uint32(vector<uint8_t> & message, uint32_t data) {
    var16 = inet_htonl(data);
    message.push_back((uint8_t)(var16>>24));
    message.push_back((uint8_t)(var16>>16));
    message.push_back((uint8_t)(var16>>8));
    message.push_back((uint8_t)(var16));
    return;
}

void Client::push_back_screen_info(vector<uint8_t> & message) {
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

void Client::client_pack_message(PacketType type, Options opt) {
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

bool Client::client_unpack_message(PacketType type) {

}

int client_communicate(int socketfd, Options opt) {
    Client client;
    srand((unsigned)time(NULL));
    return 0;
}
