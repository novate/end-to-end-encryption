#include "../include/DatabaseConnection.hpp"

const char* kDatabaseName = "db1551713";
const char* kDatabaseUserId= "u1551713";
const char* kDatabasePassword= "u1551713";

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

	this->id = 0;
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

bool DatabaseConnection::check_account(string account_name)
{
        MYSQL_RES *result;
	MYSQL_ROW row;

	// form command	
	string command = "select count(*) from account where username = '" + account_name + "'";

	// exec command and store return value
	result = MysqlExecCommand(command);
	row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) {
		mysql_free_result(result);
		return true;
	}
	else {
		mysql_free_result(result);
		return false;
	} 
}

bool DatabaseConnection::check_password(string account_name, string password)
{
	MYSQL_RES *result;
	MYSQL_ROW row;

	// form command
	string command = "select count(*) from account where username = '" + account_name + "' and password = MD5('" + password + "');";

	// exec command and store return value
	result = MysqlExecCommand(command);
	row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) {
		mysql_free_result(result);
		return true;
	}
	else {
		mysql_free_result(result);
		return false;
	}
}

bool DatabaseConnection::reset_password(string account_name, string password)
{
	string command = "update account set password = MD5('" + password + "') where username = '" + account_name + "'";
	MysqlExecCommand(command);

	return true;
}
