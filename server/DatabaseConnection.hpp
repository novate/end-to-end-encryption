#ifndef DATABASE_H
#define DATABASE_H

#include  <unistd.h>
#include  <iomanip>
#include  <iostream>
#include  <mysql/mysql.h>
#include  <vector>
#include  <string>
#include  "communicate.hpp"
using namespace std;

// #define DatabaseName "Simple_chat_room"
// #define DatabaseUserId "cyanic"
// #define DatabasePassword "1985727yyhstc"


// const string kDatabaseName = "Simple_chat_room";


// use singleton design
// example: https://sourcemaking.com/design_patterns/singleton/cpp/1
class DatabaseConnection {
	public:
		void DatabaseInit();
	   	// bool check_account(string account_name);	 		 // check if an account is registered
		// bool check_password(string account_name, string password);
		// bool reset_password(string account_name, string password);
		bool store_devinfo(DevInfo devinfo, PacketType type);
		bool store_packet_auth(DevInfo devinfo);
		static DatabaseConnection *obj;
		static DatabaseConnection *get_instance(); 		  	 // return a class instance	
		MYSQL_RES *MysqlExecCommand(string command);
	private:
		MYSQL *MysqlHandler;
};

#endif
