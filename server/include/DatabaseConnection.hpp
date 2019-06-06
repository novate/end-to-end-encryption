#ifndef DATABASE_H
#define DATABASE_H

#include  <unistd.h>
#include  <iomanip>
#include  <iostream>
#include  <mysql/mysql.h>
#include  <vector>
#include  <string>
using namespace std;

// use singleton design
// example: https://sourcemaking.com/design_patterns/singleton/cpp/1
class DatabaseConnection {
	public:
		void DatabaseInit();
	   	bool check_account(string account_name);	 		 // check if an account is registered
		bool check_password(string account_name, string password);
		bool reset_password(string account_name, string password);
		static DatabaseConnection *obj;
		static DatabaseConnection *get_instance(); 		  	 // return a class instance	
		MYSQL_RES *MysqlExecCommand(string command);
	private:
		uint64_t id;
		MYSQL *MysqlHandler;
};

// // Initialize Mysql connection
// // Connect to Database
// // For Now provided running on my own virtual machine
// void DatabaseConnection::DatabaseInit() 
// {
// 	// initialize mysql handler
//         while((this->MysqlHandler = mysql_init(NULL))==NULL) {
//         	cout << "mysql_init failed" << endl;
// 		cout << "retry after 3 seconds" << endl;
// 		sleep(3);
//         }
    
// 	// connect to mysql server
//         while(mysql_real_connect(this->MysqlHandler, "localhost", kDatabaseUserId, kDatabasePassword,  kDatabaseName, 0, NULL, 0) == NULL) {
//             	cout << "mysql_real_connect failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
// 		cout << "retry after 3 seconds" << endl;
// 		sleep(3);
//         }

// 	this->id = 0;
//         // set up mysql decode to gbk
//         mysql_set_character_set(this->MysqlHandler, "gbk");

// }

// // a tool function for mysql operation
// MYSQL_RES* DatabaseConnection::MysqlExecCommand(string command)
// {
// 	MYSQL_RES *result;
// 	while(mysql_query(this->MysqlHandler, command.c_str()) != 0) {
// 		cout << "mysql_query failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
// 		cout << "reconnect after 3 seconds" << endl;
// 		sleep(3);
// 		DatabaseInit();
// 	}

// 	string empty = "";
// 	while((result = mysql_store_result(this->MysqlHandler)) == NULL) {
// 		if(mysql_error(this->MysqlHandler) == empty) {
// 			mysql_free_result(result);
// 			break;
// 		}
//         	cout << "mysql_store_result failed(" << mysql_error(this->MysqlHandler) << ")" << endl;
// 		cout << "reconnect after 3 seconds" << endl;
// 		sleep(3);
// 		DatabaseInit();
// 		// reconnect to mysql server
// 		mysql_query(this->MysqlHandler, command.c_str());
// 	}

// 	return result;
// }

// bool DatabaseConnection::check_account(string account_name)
// {
//         MYSQL_RES *result;
// 	MYSQL_ROW row;

// 	// form command	
// 	string command = "select count(*) from account where username = '" + account_name + "'";

// 	// exec command and store return value
// 	result = MysqlExecCommand(command);
// 	row = mysql_fetch_row(result);

// 	if(atoi(row[0]) == 1) {
// 		mysql_free_result(result);
// 		return true;
// 	}
// 	else {
// 		mysql_free_result(result);
// 		return false;
// 	} 
// }

// bool DatabaseConnection::check_password(string account_name, string password)
// {
// 	MYSQL_RES *result;
// 	MYSQL_ROW row;

// 	// form command
// 	string command = "select count(*) from account where username = '" + account_name + "' and password = '" + password + "';";

// 	// exec command and store return value
// 	result = MysqlExecCommand(command);
// 	row = mysql_fetch_row(result);

// 	if(atoi(row[0]) == 1) {
// 		mysql_free_result(result);
// 		return true;
// 	}
// 	else {
// 		mysql_free_result(result);
// 		return false;
// 	}
// }

// void DatabaseConnection::push_message(string account_main, string account_sub, string message)
// {
// 	// form command
// 	string command = "insert into history values('";
// 	command += boost::lexical_cast<std::string>(id);
// 	command += "', '" + account_main + "','" + account_sub + "','" + message + "')";

// 	MysqlExecCommand(command);
// 	id++;
// }

// vector<string> DatabaseConnection::retrive_message(string account_main)
// {
// 	MYSQL_RES *result;
// 	MYSQL_ROW row;
// 	vector<string> StringStack;

// 	// form command
// 	// get history max count
// 	string command_pre = "select count(*) from history where (username_main = '" + account_main + "') or (username_sub = '" + account_main + "')";
// 	result = MysqlExecCommand(command_pre);
// 	row = mysql_fetch_row(result);
// 	int history_max = atoi(row[0]);
// 	mysql_free_result(result);

// 	string command = "select * from history where (username_main = '" + account_main + "') or (username_sub = '" + account_main + "')";
// 	// exec command and store return value
// 	result = MysqlExecCommand(command);
// 	int count_limit = retrive_history_count(account_main);
// 	int k = 0;
// 	while((row = mysql_fetch_row(result)) != NULL && k < history_max) {
// 		k++;
// 		if(k <= history_max - count_limit) continue;
// 		StringStack.push_back(row[1]);
// 		StringStack.push_back(row[2]);
// 		StringStack.push_back(row[3]);
// 		// message_all += row[3];
//         }

// 	mysql_free_result(result);
// 	// return message_all;
// 	return StringStack;
// }

// int DatabaseConnection::retrive_history_count(string account_name)
// {
// 	MYSQL_RES *result;
// 	MYSQL_ROW row;
	
// 	// cout << "debug" << endl;
// 	int history_count = 100;
// 	string command = "select * from account where username = '" + account_name + "'";

// 	result = MysqlExecCommand(command);
// 	row = mysql_fetch_row(result);
// 	history_count = atoi(row[2]);

// 	mysql_free_result(result);
// 	return history_count;
// }

#endif
