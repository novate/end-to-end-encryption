#include "../include/DatabaseConnection.hpp"

const char* kDatabaseName = "yzmon_1652289";
const char* kDatabaseUserId= "dbuser_1652289";
const char* kDatabasePassword= "yzmond.1652289";

extern Options opt;
int global_number = 0;

DatabaseConnection *DatabaseConnection::get_instance() // return a class instance	
{
	if (!obj)
	{
		obj = new DatabaseConnection;
	}
	return obj;
}

// Initialize Mysql connection
// Connect to Database
// For Now provided running on my own virtual machine
void DatabaseConnection::DatabaseInit() 
{
	// initialize mysql handler
        while((this->MysqlHandler = mysql_init(NULL))==NULL) {
        	cout << "mysql_init failed" << endl;
		cout << "retry after 3 seconds" << endl;
		sleep(3);
        }
    
	// connect to mysql server
        while(mysql_real_connect(this->MysqlHandler, "localhost", kDatabaseUserId, kDatabasePassword,  kDatabaseName, 0, NULL, 0) == NULL) {
            	cout << "mysql_real_connect failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
		cout << "retry after 3 seconds" << endl;
		sleep(3);
        }

        // set up mysql decode to gbk
        mysql_set_character_set(this->MysqlHandler, "gbk");

}

// a tool function for mysql operation
MYSQL_RES* DatabaseConnection::MysqlExecCommand(string command)
{
	MYSQL_RES *result;
	while(mysql_query(this->MysqlHandler, command.c_str()) != 0) {
		if (string(mysql_error(this->MysqlHandler)).substr(3) == "Dup") {
			continue;
		}
		cout << "mysql_query failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
		cout << "reconnect after 3 seconds" << endl;
		sleep(3);
		DatabaseInit();
	}

	string empty = "";
	while((result = mysql_store_result(this->MysqlHandler)) == NULL) {
		if(mysql_error(this->MysqlHandler) == empty) {
			mysql_free_result(result);
			break;
		}
        	cout << "mysql_store_result failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
		cout << "reconnect after 3 seconds" << endl;
		sleep(3);
		DatabaseInit();
		// reconnect to mysql server
		mysql_query(this->MysqlHandler, command.c_str());
	}

	return result;
}

bool DatabaseConnection::OnRecvAuthResponse(Packet packet, Client* client) {
	// sql result handler
	MYSQL_RES *result;

	AuthResponsePacket &packet_struct = *((AuthResponsePacket *)packet.payload.first);
	MysqlExecCommand("delete from devstate_base;");
	MysqlExecCommand("delete from devstate_ttyinfo;");
	MysqlExecCommand("delete from devstate_scrinfo;");
	std::stringstream command;
	command << "insert into devstate_base (devstate_base_devid, devstate_base_devno, devstate_base_time, devstate_base_ipaddr, devstate_base_sid, "
	 << "devstate_base_type , devstate_base_version, devstate_base_cpu, devstate_base_sdram, devstate_base_flash, devstate_base_ethnum, devstate_base_syncnum, "
	 << "devstate_base_asyncnum, devstate_base_switchnum, devstate_base_usbnum, devstate_base_prnnum, devstate_base_cpu_used, devstate_base_sdram_used, devstate_base_tty_configed, devstate_base_tty_connected) values ("; 

	// devid
	command << "'" << to_string(packet_struct.devid) << "', ";
	client->devid = to_string(packet_struct.devid);
	// devno
	command << "'" << to_string(packet_struct.devno) << "', ";
	client->devno = to_string(packet_struct.devno);
	// time
	command << "now(), ";
	// ipaddr
	command << "'" << client->ipaddr << "', ";
	// sid
	// std::string group_serial(packet_struct.group_serial, packet_struct.group_serial + 16);
	command << "'" << packet_struct.group_serial << to_string(packet_struct.internal_serial) << "', ";
	// type
	// std::string device_type(packet_struct.device_type, packet_struct.device_type + 16);
	command << "'" << packet_struct.device_type << "', ";
	// version
	// std::string software_version(packet_struct.software_verison, packet_struct.software_verison + 16);
	command << "'" << packet_struct.software_verison << "', ";
	// cpu
	command << to_string(packet_struct.cpu_frequence) << ", ";
	// sdram
	command << to_string(packet_struct.ram) << ", ";
	client->ram = packet_struct.ram;
	// flash
	command << to_string(packet_struct.flash) << ", ";
	// ethnum
	command << to_string(packet_struct.ethnum) << ", ";
	client->ethnum = packet_struct.ethnum;
	// syncnum
	command << to_string(packet_struct.syncnum) << ", ";
	// asyncnum
	command << to_string(packet_struct.asyncnum) << ", ";
	// switchnum
	command << to_string(packet_struct.switchnum) << ", ";
	// usbnum
	command << "'" << to_string(packet_struct.usbnum) << "', ";
	client->usbnum = packet_struct.usbnum;
	// prnnum
	command << "'" << to_string(packet_struct.prnnum) << "', 0, 0, 0, 0);\n";
	client->prnnum = packet_struct.prnnum;

	string temp = command.str();
	cout << temp << endl;
	result = MysqlExecCommand(command.str());

	delete (AuthResponsePacket*)packet.payload.first;
	if(*mysql_error(this->MysqlHandler))
	{
	    // an error occurred
		LERR << "验证包写入数据库失败" << endl;
		LDB << "验证包写入数据库失败" << endl;
		return false;	  
	}
	else {
		LDB << "收到验证包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvSysInfoResponse(Packet packet, const Client &client) {
	double cpu_used, sdram_used;
	// sql result handler
	MYSQL_RES *result;

	SysInfoResponsePacket &packet_struct = *((SysInfoResponsePacket*)packet.payload.first);

	cpu_used = ((double)(packet_struct.user_cpu_time + packet_struct.system_cpu_time)) / (packet_struct.user_cpu_time + packet_struct.nice_cpu_time +packet_struct.idle_cpu_time);
	sdram_used = ((double)packet_struct.freed_memory) / (client.ram);

	std::stringstream command;
	command << "update devstate_base set devstate_base_cpu_used = ";
	command << fixed << setprecision(2) << cpu_used;
	command << ", devstate_base_sdram_used = ";
	command << fixed << setprecision(2) << sdram_used;
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	delete (SysInfoResponsePacket*)packet.payload.first;
	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "系统信息包写入数据库失败" << endl;
		LDB << "系统信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到系统信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvConfInfoResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	// ConfInfoResponsePacket &packet_struct = *((ConfInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_config = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;


	delete (ConfInfoResponsePacket*)packet.payload.first;
	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "配置信息包写入数据库失败" << endl;
		LDB << "配置信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到配置信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvProcInfoResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	// ProcInfoResponsePacket &packet_struct = *((ProcInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_process = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	delete (ProcInfoResponsePacket*)packet.payload.first;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "进程信息包写入数据库失败" << endl;
		LDB << "进程信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到进程信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

string DatabaseConnection::int32_t2ipaddr(int32_t addr) {
	unsigned char bytes[4];
    bytes[0] = addr & 0xFF;
    bytes[1] = (addr >> 8) & 0xFF;
    bytes[2] = (addr >> 16) & 0xFF;
    bytes[3] = (addr >> 24) & 0xFF;

	stringstream ss;
	ss << int(bytes[3]) << "." << int(bytes[2]) << "." << int(bytes[1]) << "." << int(bytes[0]);

	return ss.str();
}

bool DatabaseConnection::OnRecvEtherInfoResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	EtherInfoResponsePacket &packet_struct = *((EtherInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	char ethernet_num;
	if(packet_struct.port == 0x0000)  ethernet_num = '0';
	else ethernet_num = '1';

	command << "update devstate_base set devstate_base_eth" << ethernet_num << "_ip = '";
	command << int32_t2ipaddr(packet_struct.addr) << "', ";
	command << "devstate_base_eth" << ethernet_num << "_mask = '";
	command << int32_t2ipaddr(packet_struct.mask) << "', ";
	command << "devstate_base_eth" << ethernet_num << "_mac = '";
	command << std::hex << (int)packet_struct.mac[0] << (int)packet_struct.mac[1] << ":";
	command << (int)packet_struct.mac[2] << (int)packet_struct.mac[3] << ":";
	command << (int)packet_struct.mac[4] << (int)packet_struct.mac[5] << "', ";
	// restore to decimal
	command << std::dec;
	command << "devstate_base_eth" << ethernet_num << "_state = " << (packet_struct.state == 0x0001 ? "'UP'" : "'DOWN'") << ", ";
	command << "devstate_base_eth" << ethernet_num << "_speed = " << ((packet_struct.options & 0x0001) == 0x0001 ? "'100MB'" : "'10MB'") << ", ";
	command << "devstate_base_eth" << ethernet_num << "_duplex = " << ((packet_struct.options & 0x0002) == 0x0002 ? "'全双工'" : "'半双工'") << ", ";
	command << "devstate_base_eth" << ethernet_num << "_autonego = " << ((packet_struct.options & 0x0004) == 0x0004 ? "'是'" : "'否'") << ", ";
	command << "devstate_base_eth" << ethernet_num << "_txbytes = " << to_string(packet_struct.send_bytes) << ", ";
	command << "devstate_base_eth" << ethernet_num << "_txpackets = " << to_string(packet_struct.send_packets) << ", ";
	command << "devstate_base_eth" << ethernet_num << "_rxbytes = " << to_string(packet_struct.recv_bytes) << ", ";
	command << "devstate_base_eth" << ethernet_num << "_txpackets = " << to_string(packet_struct.recv_packets);
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	delete (EtherInfoResponsePacket*)packet.payload.first;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "以太网包写入数据库失败" << endl;
		LDB << "以太网包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到以太网包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvUsbStateResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	USBStateResponsePacket &packet_struct = *((USBStateResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_usbstate = '";
	command << (packet_struct.usb_is_inserted == 1 ? "已插入" : "未插入") << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	delete (USBStateResponsePacket*)packet.payload.first;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "USB口信息包写入数据库失败" << endl;
		LDB << "USB口信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到USB口信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvUsbInfoResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	// ProcInfoResponsePacket &packet_struct = *((ProcInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_usbfiles = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "U盘文件列表信息写入数据库失败" << endl;
		LDB << "U盘文件列表信息写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到U盘文件列表信息，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvPrintDevResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	PrintDevResponsePacket &packet_struct = *((PrintDevResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_prnname = '";
	std::string s(packet_struct.printer_name, packet_struct.printer_name+32);
	command << s << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;
	command << "update devstate_base set devstate_base_prnstate = '";
	command << (packet_struct.print_is_usable == 1 ? "已启动" : "未启动") << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "打印口信息写入数据库失败" << endl;
		LDB << "打印口信息写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到打印口信息，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvPrintQueueResponse(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	// ProcInfoResponsePacket &packet_struct = *((ProcInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_prnfiles = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "打印队列写入数据库失败" << endl;
		LDB << "打印队列写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到打印队列，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvTermResponse(Packet packet, Client &client) {
	// sql result handler
	MYSQL_RES *result;

	TerInfoResponsePacket &packet_struct = *((TerInfoResponsePacket*)packet.payload.first);

	std::stringstream command;

	command << "update devstate_base set devstate_base_tty_configed = " << to_string(packet_struct.term_num);
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;
	client.tty_connected = packet_struct.term_num;
	delete (TerInfoResponsePacket*)packet.payload.first;
	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "终端服务信息包写入数据库失败" << endl;
		LDB << "终端服务信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到终端服务信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvIPTermResponse(Packet packet, Client &client) {
	// sql result handler
	MYSQL_RES *result;

	IPTermResponsePacket &packet_struct = *((IPTermResponsePacket *)packet.payload.first);
	
	bool is_dump = (packet.header.packet_type == 0x0a);

	std:stringstream command;
	command << "insert into devstate_ttyinfo (devstate_ttyinfo_devid, devstate_ttyinfo_devno, devstate_ttyinfo_ttyno, ";
	command << "devstate_ttyinfo_time, devstate_ttyinfo_readno, devstate_ttyinfo_type, devstate_ttyinfo_state, ";
	command << "devstate_ttyinfo_ttyip, devstate_ttyinfo_scrnum) values (";
	command << "'" << client.devid << "', ";
	command << "'" << client.devno << "', ";
	command << (is_dump ? "9" + to_string(packet_struct.ttyno) : to_string(packet_struct.ttyno)) << ", ";
	client.current_tty = (is_dump ? 900 + packet_struct.ttyno : packet_struct.ttyno);
	client.current_scr = packet_struct.active_screen;
	command << "now(), ";
	command << to_string(packet_struct.readno) << ", ";
	// std::string tty_type(packet_struct.type, packet_struct.type + 12);
	command << "'" << packet_struct.type << "', ";
	// std::string tty_state(packet_struct.state, packet_struct.state + 12);
	command << "'" << packet_struct.state << "', ";
	command << "'" << int32_t2ipaddr(packet_struct.ttyip) << "', ";
	command << to_string(packet_struct.screen_num) << ")";
	// delete (IPTermResponsePacket*)packet.payload.first;
	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "终端信息包写入数据库失败" << endl;
		LDB << "终端信息包写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到终端信息包，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::OnRecvScreenInfoPacket(Packet packet, const Client &client) {
	// sql result handler
	MYSQL_RES *result;

	ScreenInfoPacket &packet_struct = *((ScreenInfoPacket*)packet.payload.first);

	std::stringstream command;
	command << "insert into devstate_scrinfo (devstate_scrinfo_devid, devstate_scrinfo_devno, devstate_scrinfo_ttyno, ";
	command << "devstate_scrinfo_scrno, devstate_scrinfo_time, devstate_scrinfo_is_current, devstate_scrinfo_protocol, ";
	command << "devstate_scrinfo_serverip, devstate_scrinfo_serverport, devstate_scrinfo_state, devstate_scrinfo_ttytype, ";
	command << "devstate_scrinfo_tx_server, devstate_scrinfo_rx_server, devstate_scrinfo_tx_terminal, devstate_scrinfo_rx_terminal, ";
	command << "devstate_scrinfo_ping_min, devstate_scrinfo_ping_avg, devstate_scrinfo_ping_max) values (";
	command << "'" << client.devid << "', ";
	command << "'" << client.devno << "', ";
	command << to_string(client.current_tty) << ", ";
	// command << to_string(packet_struct.screen_no) << ", ";
	command << to_string(global_number++%127) << ", ";
	command << "now(), ";
	command << "'*' , ";
	// std::string proto(packet_struct.proto, packet_struct.proto + 12);
	command << "'" << packet_struct.proto << "', ";
	command << "'" << int32_t2ipaddr(packet_struct.server_ip) << "', ";
	command << to_string(packet_struct.server_port) << ", ";
	// std::string scr_state(packet_struct.state, packet_struct.state + 8);
	command << "'" << packet_struct.state << "', ";
	// std::string tty_type(packet_struct.tty_type, packet_struct.tty_type + 12);
	command << "'" << packet_struct.tty_type << "', ";
	command << to_string(packet_struct.send_server_byte) << ", ";
	command << to_string(packet_struct.recv_server_byte) << ", ";
	command << to_string(packet_struct.send_term_byte) << ", ";
	command << to_string(packet_struct.recv_term_byte) << ", ";
	float ping_ming = packet_struct.ping_min / 10.0;
	float ping_max = packet_struct.ping_max / 10.0;
	float ping_avg = packet_struct.ping_avg / 10.0;
	command << ping_ming << ", ";
	command << ping_avg << ", ";
	command << ping_max << ")";
	delete (ScreenInfoPacket*)packet.payload.first;
	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "虚屏信息写入数据库失败" << endl;
		LDB << "虚屏信息写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到虚屏信息，已验证并写入数据库" << std::endl;
		return true;
	}
}

bool DatabaseConnection::UpdateTTYConnected(const Client &client) {
	// sql result handler
	MYSQL_RES *result;
	
	std::stringstream command;

	command << "update devstate_base set devstate_base_tty_connected = " << to_string(client.tty_connected);
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(*mysql_error(this->MysqlHandler)) {
		LERR << "终端总数写入数据库失败" << endl;
		LDB << "终端总数写入数据库失败" << endl;
		return false;
	}
	else {
		LDB << "收到终端总数，已验证并写入数据库" << std::endl;
		return true;
	}
}
