#ifndef DATABASE_H
#define DATABASE_H

#include <unistd.h>
#include <iomanip>
#include <iostream>
#include <mysql/mysql.h>
#include <vector>
#include <sstream>
#include <string>
#include "log.hpp"
#include "parse_arguments.hpp"
#include "types.hpp"
using namespace std;

// use singleton design
// example: https://sourcemaking.com/design_patterns/singleton/cpp/1
class DatabaseConnection {
	public:
		void DatabaseInit();
	   	// bool check_account(string account_name);	 		 // check if an account is registered
		// bool check_password(string account_name, string password);
		// bool reset_password(string account_name, string password);
		bool OnRecvAuthResponse(Packet packet, Client* client);
		bool OnRecvSysInfoResponse(Packet packet, const Client &client);
		bool OnRecvConfInfoResponse(Packet packet, const Client &client);
		bool OnRecvProcInfoResponse(Packet packet, const Client &client);
		bool OnRecvEtherInfoResponse(Packet packet, const Client &client);
		bool OnRecvUsbStateResponse(Packet packet, const Client &client);
		bool OnRecvUsbInfoResponse(Packet packet, const Client &client);
		bool OnRecvPrintDevResponse(Packet packet, const Client &client);
		bool OnRecvPrintQueueResponse(Packet packet, const Client &client);
		bool OnRecvTermResponse(Packet packet, Client &client);
		bool OnRecvIPTermResponse(Packet packet, Client &client);
		bool OnRecvScreenInfoPacket(Packet packet, const Client &client);
		bool UpdateTTYConnected(const Client &client);
		string int32_t2ipaddr(int32_t addr);
		static DatabaseConnection *obj;
		static DatabaseConnection *get_instance(); 		  	 // return a class instance	
		MYSQL_RES *MysqlExecCommand(string command);
	private:
		MYSQL *MysqlHandler;
};

#endif
