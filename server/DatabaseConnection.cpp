#include "./DatabaseConnection.hpp"

const char* kDatabaseName = "db1652289";
const char* kDatabaseUserId= "u1652289";
const char* kDatabasePassword= "u1652289";

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

bool DatabaseConnection::OnRecvAuthResponse(Packet packet, Client* client)
{
	// sql result handler
	MYSQL_RES *result;

	AuthResponsePacket &packet_struct = *((AuthResponsePacket *)packet.payload.first);
	std::stringstream command;
	command << "insert into devstate_base (devstate_base_devid, devstate_base_devno, devstate_base_time, devstate_base_ipaddr, devstate_base_sid, "
	 << "devstate_base_type , devstate_base_version, devstate_base_cpu, devstate_base_sdram, devstate_base_flash, devstate_base_ethnum, devstate_base_syncnum, "
	 << "devstate_base_asyncnum, devstate_base_switchnum, devstate_base_usbnum, devstate_base_prnnum) values ("; 

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
	std::string group_serial(packet_struct.group_serial, packet_struct.group_serial + 16);
	command << "'" << group_serial << to_string(packet_struct.internal_serial) << "', ";
	// type
	std::string internal_serial(packet_struct.device_type, packet_struct.device_type + 16);
	command << "'" << internal_serial << "', ";
	// version
	std::string software_version(packet_struct.software_verison, packet_struct.software_verison + 16);
	command << "'" << software_version << "', ";
	// cpu
	command << "'" << to_string(packet_struct.cpu_frequence) << "', ";
	// sdram
	command << "'" << to_string(packet_struct.ram) << "', ";
	client->ram = packet_struct.ram;
	// flash
	command << "'" << to_string(packet_struct.flash) << ", ";
	// ethnum
	command << "'" << to_string(packet_struct.ethnum) << ", ";
	// syncnum
	command << "'" << to_string(packet_struct.syncnum) << ", ";
	// asyncnum
	command << "'" << to_string(packet_struct.asyncnum) << ", ";
	// switchnum
	command << "'" << to_string(packet_struct.switchnum) << ", ";
	// usbnum
	command << "'" << to_string(packet_struct.usbnum) << ", ";
	// prnnum
	command << "'" << to_string(packet_struct.prnnum) << ")";

	result = MysqlExecCommand(command.str());
	if(result == NULL) return false;
	else return true;
}

bool DatabaseConnection::OnRecvSysInfoResponse(Packet packet, Client client) {
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

	result = MysqlExecCommand(command.str());
	if(result == NULL) return false;
	else return true;
}

bool DatabaseConnection::OnRecvConfInfoResponse(Packet packet, Client client) {
	// sql result handler
	MYSQL_RES *result;

	// ConfInfoResponsePacket &packet_struct = *((ConfInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_config = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(result == NULL) return false;
	else return true;
}

bool DatabaseConnection::OnRecvProcInfoResponse(Packet packet, Client client) {
	// sql result handler
	MYSQL_RES *result;

	// ProcInfoResponsePacket &packet_struct = *((ProcInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	command << "update devstate_base set devstate_base_process = '";
	command << packet.payload.second.data() << "'";
	command << " where devstate_base_devid = " << client.devid << " and devstate_base_devno = " << client.devno;

	result = MysqlExecCommand(command.str());
	if(result == NULL) return false;
	else return true;
}

bool DatabaseConnection::OnRecvEtherInfoResponse(Packet packet, Client client) {
	// sql result handler
	MYSQL_RES *result;

	EtherInfoResponsePacket &packet_struct = *((EtherInfoResponsePacket*)packet.payload.first);

	std::stringstream command;
	char ethernet_num;
	if(packet_struct.port == 0x0000)  ethernet_num = '0';
	else ethernet_num = '1';

	command << "update devstate_base set devstate_base_eth" << ethernet_num << "_ip = ";
	 
}
