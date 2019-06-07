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
    char devid[9];  // �豸������
    char devno[3];  //  ���������
    time_t time;    //  ����д������ʱ��
    char ipaddr[15];    // Client�˵�IP��ַ
    char sid[32];   //  �豸�������к�+�豸���ڲ����к�
    char type[16];  //  �豸���ͺ�
    char version[16];   // �豸������汾��
    int cpu[3]; // �豸��CPU��Ƶ
    int sdram[3];   //�豸��SDRAM��С
    int flash[3];   // �豸��FLASH��С
    int ethnum; // ��̫������
    int syncnum;    // ͬ��������
    int asyncnum[2];    // �첽������
    int switchnum[2];   // ����������
    char usbnum[6]; // USB��(����/������)
    char prnnum[6]; // ��ӡ��(����/������)
    // SysInfoResponse
    float cpu_used; // CPUռ����(float6.2)
    float sdram_used;   // �ڴ�ʹ�����(float6.2)
    // ConfInfoResponse
    char config[8192];  // ϵͳ����
    // ProcInfoResponse
    char process[8192]; // ϵͳ��ǰ����
    // EtherInfoResponse
    char eth0_ip[15];   // eth0��ip��ַ
    char eth0_mask[15]; // eth0��mask
    char eth0_mac[17];  // eth0��mac
    char eth0_state[4]; // eth0��״̬(UP/Down)
    char eth0_speed[5]; // eth0���ٶ�(100/10)
    char eth0_duplex[6];    // eth0�Ĺ���״̬(��˫��\ȫ˫��)
    char eth0_autonego[2];  // eth0���Ƿ��Զ�Э��(��\��)
    int eth0_txbytes[11];   // eth0���͵��ֽ���
    int eth0_txpackets[11]; // eth0���͵İ���
    int eth0_rxbytes[11];   // eth0���ܵ��ֽ���
    int eth0_rxpackets[11]; // eth0���ܵİ���
    char eth1_ip[15];   // eth1��ip��ַ
    char eth1_mask[15]; // eth1��mask
    char eth1_mac[17];  // eth1��mac
    char eth1_state[4]; // eth1��״̬(UP/Down)
    char eth1_speed[5]; // eth1���ٶ�(100/10)
    char eth1_duplex[6];    // eth1�Ĺ���״̬(��˫��\ȫ˫��)
    char eth1_autonego[2];  // eth1���Ƿ��Զ�Э��(��\��)
    int eth1_txbytes[11];   // eth1���͵��ֽ���
    int eth1_txpackets[11]; // eth1���͵İ���
    int eth1_rxbytes[11];   // eth1���ܵ��ֽ���
    int eth1_rxpackets[11]; // eth1���ܵİ���
    // USBInfoResponse
    char usbstate[6];   // �Ƿ����U��
    // USBFileResponse
    char usbfiles[4096];    // U�̵ĸ�Ŀ¼����
    // PrintDevResponse
    char prnstate[6];   // ��ӡ�����Ƿ�����
    // PrintQueueResponse
    char prnfiles[4096];    // ��ӡ���������е�����
    // TerInfoResponse
    int tty_configed[3];    // ���õ��ն�����
    // IPTermResponse DumbTerResponse
    char ttyinfo_devid[9];  // ���������
    char ttyinfo_devno[3];  // �������
    int ttyinfo_ttyno[3];   // �ն˺�
    time_t ttyinfo_time;    // ʱ���
    int ttyinfo_readno[3];  // ���ĸ��ն˺Ŷ�����
    char ttyinfo_type[12];  // ����(�����ն�/���ڴ�ӡ/IP �ն�)
    char ttyinfo_state[12]; // ״̬(�˵�/����)
    char ttyinfo_ttyip[15]; // IP �ն˵ĵ�ַ(�����ն���)
    int ttyinfo_scrnum[2];  // ���ն˶�Ӧ��������
    char scrinfo_devid[9];  // ���������
    char scrinfo_devno[3];  // �������
    int scrinfo_ttyno[3];   // �ն˺�
    int scrinfo_scrno[2];   // ������
    time_t scrinfo_time;    // ʱ���
    char scrinfo_is_current;    // �Ƿ�ǰ��(*���)
    char scrinfo_protocol[16];  // ����Э��(ccbtelnet��yzssh)
    char scrinfo_serverip[15];  // ������ IP
    int scrinfo_serverport[5];  // �������˿ں�
    char scrinfo_state[12]; // ״̬(δ��½���ѵ�½)
    char scrinfo_ttytype[12];   // �ն�����(vt100,vt200)
    int scrinfo_tx_server[11];  // �����������ֽ�
    int scrinfo_rx_server[11];  // �շ��������ֽ�
    int scrinfo_tx_terminal[11];    // ���ն˵��ֽ�
    int scrinfo_rx_terminal[11];    // ���ն˵��ֽ�
    float scrinfo_ping_min; // ping ����������Ӧ��ǰ�÷����� IP ��ַ����ʱ����Сֵ
    float scrinfo_ping_avg; // ping ����������Ӧ��ǰ�÷����� IP ��ַ����ʱ��ƽ��ֵ
    float scrinfo_ping_max; // ping ����������Ӧ��ǰ�÷����� IP ��ַ����ʱ�����ֵ
    // Final
    int tty_connected[3];   // �����ӵ��ն�����
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
