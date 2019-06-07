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

bool DatabaseConnection::store_packet_auth(DevInfo devinfo) 
{
	// form command
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
	string command = "insert into devstate_base (devstate_base_devid, devstate_base_devno, devstate_base_time, devstate_base_ipaddr, devstate_base_sid, devstate_base_type, devstate_base_version, devstate_base_cpu, devstate_base_sdram, devstate_base_flash, devstate_base_ethnum, devstate_base_syncnum, devstate_base_asyncnum, devstate_base_switchnum, devstate_base_usbnum, devstate_base_prnnum) values('";
	string devid(&devinfo.devid, &devinfo.devid + 9);
	command += devid + ", "	;
	string 

	MysqlExecCommand(command);
}

bool DatabaseConnection::store_devinfo(DevInfo devinfo, PacketType type)
{
	switch (type)
	{
		case PacketType::AuthResponse: {
			store_packet_auth(devinfo);
		}
		case PacketType::SysInfoResponse: {

		}
		case PacketType::ConfInfoResponse: {

		}
		case PacketType::ProcInfoResponse: {

		}
		case PacketType::EtherInfoResponse: {

		}
		case PacketType::USBInfoResponse: {

		}
		case PacketType::USBfileResponse: {

		}
		case PacketType::PrintDevResponse: {

		}
		case PacketType::PrintQueueResponse: {

		}
		case PacketType::TerInfoResponse: {

		}
		case PacketType::DumbTerResponse: {

		}
		case PacketType::IPTermRequest: {

		}
	}

	return true;
}
