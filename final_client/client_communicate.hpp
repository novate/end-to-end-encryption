#ifndef CLIENT_COMMUNICATE_H
#define CLIENT_COMMUNICATE_H

#include <ctime>

#include "shared_library.hpp"

using namespace std;

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


struct ClientDevInfo {
    uint16_t cpu;  // MHz, from '/proc/cpuinfo/', 1st CPU
    uint16_t ram;      // (kB to)MB, from '/proc/meminfo/'
    uint16_t flash;   // MB

    uint16_t devInnerID;
    uint8_t groupSeq[16];
    uint8_t type[16];
    uint8_t version[16];
    
    uint32_t instID;    //'devid' in database
    uint8_t instInnID;

    uint8_t authstr[32];

    //IPterm info
    uint8_t IPtermMap[256];
};

class DevInfo {
private:
    // AuthResponse
    char devid[9];  // 设备机构号
    char devno[3];  //  机构内序号
    time_t time;    //  本次写入数据时间
    char ipaddr[15];    // Client端的IP地址
    char sid[32];   //  设备的组序列号+设备的内部序列号
    char type[16];  //  设备的型号
    char version[16];   // 设备的软件版本号
    int cpu[3]; // 设备的CPU主频
    int sdram[3];   //设备的SDRAM大小
    int flash[3];   // 设备的FLASH大小
    int ethnum; // 以太口数量
    int syncnum;    // 同步口数量
    int asyncnum[2];    // 异步口数量
    int switchnum[2];   // 交换机数量
    char usbnum[6]; // USB口(存在/不存在)
    char prnnum[6]; // 打印口(存在/不存在)
    // SysInfoResponse
    float cpu_used; // CPU占用率(float6.2)
    float sdram_used;   // 内存使用情况(float6.2)
    // ConfInfoResponse
    char config[8192];  // 系统配置
    // ProcInfoResponse
    char process[8192]; // 系统当前进程
    // EtherInfoResponse
    char eth0_ip[15];   // eth0的ip地址
    char eth0_mask[15]; // eth0的mask
    char eth0_mac[17];  // eth0的mac
    char eth0_state[4]; // eth0的状态(UP/Down)
    char eth0_speed[5]; // eth0的速度(100/10)
    char eth0_duplex[6];    // eth0的工作状态(半双工\全双工)
    char eth0_autonego[2];  // eth0的是否自动协商(是\否)
    int eth0_txbytes[11];   // eth0发送的字节数
    int eth0_txpackets[11]; // eth0发送的包数
    int eth0_rxbytes[11];   // eth0接受的字节数
    int eth0_rxpackets[11]; // eth0接受的包数
    char eth1_ip[15];   // eth1的ip地址
    char eth1_mask[15]; // eth1的mask
    char eth1_mac[17];  // eth1的mac
    char eth1_state[4]; // eth1的状态(UP/Down)
    char eth1_speed[5]; // eth1的速度(100/10)
    char eth1_duplex[6];    // eth1的工作状态(半双工\全双工)
    char eth1_autonego[2];  // eth1的是否自动协商(是\否)
    int eth1_txbytes[11];   // eth1发送的字节数
    int eth1_txpackets[11]; // eth1发送的包数
    int eth1_rxbytes[11];   // eth1接受的字节数
    int eth1_rxpackets[11]; // eth1接受的包数
    // USBInfoResponse
    char usbstate[6];   // 是否插入U盘
    // USBFileResponse
    char usbfiles[4096];    // U盘的根目录内容
    // PrintDevResponse
    char prnstate[6];   // 打印任务是否启动
    // PrintQueueResponse
    char prnfiles[4096];    // 打印队列中现有的内容
    // TerInfoResponse
    int tty_configed[3];    // 配置的终端数量
    // IPTermResponse DumbTerResponse
    char ttyinfo_devid[9];  // 网点机构号
    char ttyinfo_devno[3];  // 网点序号
    int ttyinfo_ttyno[3];   // 终端号
    time_t ttyinfo_time;    // 时间号
    int ttyinfo_readno[3];  // 从哪个终端号读配置
    char ttyinfo_type[12];  // 类型(串口终端/串口打印/IP 终端)
    char ttyinfo_state[12]; // 状态(菜单/正常)
    char ttyinfo_ttyip[15]; // IP 终端的地址(串口终端无)
    int ttyinfo_scrnum[2];  // 该终端对应的虚屏数
    char scrinfo_devid[9];  // 网点机构号
    char scrinfo_devno[3];  // 网点序号
    int scrinfo_ttyno[3];   // 终端号
    int scrinfo_scrno[2];   // 虚屏号
    time_t scrinfo_time;    // 时间号
    char scrinfo_is_current;    // 是否当前屏(*或空)
    char scrinfo_protocol[16];  // 虚屏协议(ccbtelnet、yzssh)
    char scrinfo_serverip[15];  // 服务器 IP
    int scrinfo_serverport[5];  // 服务器端口号
    char scrinfo_state[12]; // 状态(未登陆、已登陆)
    char scrinfo_ttytype[12];   // 终端类型(vt100,vt200)
    int scrinfo_tx_server[11];  // 发服务器的字节
    int scrinfo_rx_server[11];  // 收服务器的字节
    int scrinfo_tx_terminal[11];    // 发终端的字节
    int scrinfo_rx_terminal[11];    // 收终端的字节
    float scrinfo_ping_min; // ping 该虚屏所对应的前置服务器 IP 地址的延时的最小值
    float scrinfo_ping_avg; // ping 该虚屏所对应的前置服务器 IP 地址的延时的平均值
    float scrinfo_ping_max; // ping 该虚屏所对应的前置服务器 IP 地址的延时的最大值
    // Final
    int tty_connected[3];   // 已连接的终端数量
public:
    // provide function to transfer packet data to DevInfo Object Element
};

class Client {
public:
    Client(int devid){
        isConnected = false;
        isVerified = false;
        gene_dev_info(devid);
        terCount = 0;
        screenCount = 0;
    }
    //recv & send message
    int recv_msg(int socketfd);
    int send_msg(int socketfd);

    //TODO
    int client_communicate(int socketfd, const Options & opt, std::ofstream& xls_stream);

    //pack & unpack
    bool client_pack_message(PacketType type, const Options & opt);
    bool client_unpack_message(const int socketfd, const Options & opt);

    void push_back_uint16(vector<uint8_t> & message, uint16_t data);
    void push_back_uint32(vector<uint8_t> & message, uint32_t data);
    void push_back_array(vector<uint8_t> & message, uint8_t * array, int length);
    void push_back_screen_info(vector<uint8_t> & message, const uint8_t screen_num);

    void pop_first_uint8(vector<uint8_t> & message, uint8_t& data);
    void pop_first_uint16(vector<uint8_t> & message, uint16_t& data);
    void pop_first_uint32(vector<uint8_t> & message, uint32_t& data);
    void pop_first_array(vector<uint8_t> & message, uint8_t * array, int length);
	void gene_dev_info(int devid);
    
private:
    // TCP
    int socketfd;
    vector<uint8_t> send_message;
    vector<uint8_t> recv_message;

    uint8_t recvPacketType;
    uint8_t sendPacketType;

    char send_buffer[4096];
    char recv_buffer[4096];

    //status
    bool isConnected;
    bool isVerified;
    uint8_t seqNum;
    
    // required verson
    uint16_t serverMainVersion;
    uint8_t serverSec1Version;
    uint8_t serverSec2Version;

    //raw server version
    uint16_t rawServerMainVersion;
    uint8_t  rawServerSec1Version;
    uint8_t  rawServerSec2Version;
    uint16_t rawFailInterval;
    uint16_t rawAnotherInterval;
    uint8_t  rawPermitEmptyTerminal;
    uint16_t rawDumbTerFlags;
    uint16_t rawIPTerFlags;
    
    u_int terCount;
    u_int screenCount;

    // dev info
    ClientDevInfo dev;

};

#endif // CLIENT_COMMUNICATE_H
