#ifndef APPLICA_H
#define APPLICA_H
#include "DatabaseConnection.hpp"
// #include "client.hpp"
#include "types.hpp"
#include "../include/presentation.hpp"
#include "../include/transfer.hpp"
#include "Log.h"
// #include <iostream>


class ApplicationLayer{
        private:
                // DatabaseConnection *DatabaseConnection::obj;
                bool CheckUser(std::string user_name_);
                bool CheckPasswd(std::string user_name_, std::string password_);
                bool ResetPasswd(std::string user_name_, std::string password_);
        public:
                void CopyBoard(Client *client_A, Client *client_B);
                ApplicationLayer();
                void MessageToApp(Client *client);
                void BroadcastOffline(Client* client);
};



#endif
