#include "../include/application.hpp"

using namespace fly;
using namespace std;
const string InitPassword = "123456";

extern PresentationLayer PreLayerInstance;
extern TransferLayer TransLayerInstance;

DatabaseConnection *DatabaseConnection::obj = NULL;
ApplicationLayer::ApplicationLayer()
{
        // initialize DatabaseConnection class
        DatabaseConnection::get_instance()->DatabaseInit();

        return;
}

bool ApplicationLayer::CheckUser(std::string user_name_)
{
        return DatabaseConnection::get_instance()->check_account(user_name_);
}

bool ApplicationLayer::CheckPasswd(std::string user_name_, std::string password_)
{
        return DatabaseConnection::get_instance()->check_password(user_name_, password_);
}

bool ApplicationLayer::ResetPasswd(std::string user_name_, std::string password_) 
{
        return DatabaseConnection::get_instance()->reset_password(user_name_, password_);
}

void ApplicationLayer::BroadcastOffline(Client* client)
{
        std::vector<Client*> client_list_;
        client_list_ = TransLayerInstance.find_all_client(client);
        std::vector<Client*>::iterator it = client_list_.begin();
        for(; it != client_list_.end(); it++) {
                (*it)->message_atop.type_ = PacketType::OfflineUser;
                (*it)->message_atop.user_change_ = client->host_username_;
                PreLayerInstance.pack_Message(*it);
        }

        return;
}

void ApplicationLayer::MessageToApp(Client *client_name_)
{
        // main process here
        // finite state machine
        Message_To_App *message_ = &client_name_->message_ptoa;
        Message_To_Pre *respond_ = &client_name_->message_atop;
        switch(client_name_->state)
        {
                case SessionState::Acceptance: {
                        // if(message_->type_ != PacketType::Info){
                        //         // error occurs
                        //         // client_name_.message_atop.respond_ = ResponseType::ErrorOccurs;
                        //         respond_->respond_ = ResponseType::ErrorOccurs;
                        //         PreLayerInstance.pack_Message(client_name_);
                        //         LOG(Error) << "Error receive info packet: " << unsigned(uint8_t(message_->type_)) << endl 
                        //             << "Username: " << message_->user_name_ << std::endl;
                        //         return;
                        //         // stop the connection 
                        //         // client_name_->state = SessionState::Error;
                        //         // respond_.type_ = PacketType::InfoResponse;
                        // }        
                        // do recv info packet
                        switch(CheckUser(message_->user_name_)){
                               case true: {
                                       // account exists
                                        respond_->type_ = PacketType::InfoResponse;
                                        respond_->respond_ = ResponseType::OK;
                                        client_name_->host_username_ = message_->user_name_;
                                        LOG(Info) << "Check User Exists" << std::endl;
                                        PreLayerInstance.pack_Message(client_name_);
                                        client_name_->state = SessionState::WaitForPasswd;
                                       break;
                               }
                               case false: {
                                       // account not exists
                                        respond_->type_ = PacketType::InfoResponse;
                                        respond_->respond_ = ResponseType::UserNotExist;
                                        LOG(Error) << "User not Exists" << std::endl;
                                        PreLayerInstance.pack_Message(client_name_);
                                        break;
                               }
                        }
                        break;
                }
                case SessionState::WaitForPasswd: {
                //         if(message_->type_ != PacketType::Password) {
                //                 // error occurs
                //                 LOG(Error) << "Error receive password packet" << std::endl;
                //                 // stop the connection
                //                 client_name_->state = SessionState::Error;
                //                 respond_->type_ = PacketType::PasswordResponse;
                //                 respond_->respond_ = ResponseType::ErrorOccurs;
                //                 PreLayerInstance.pack_Message(client_name_);
                //                 return;
                //         }
                        // do recv password packet
                        switch(CheckPasswd(client_name_->host_username_, message_->password_)) {
                                case true: {
                                        // password correct
                                        Client * client_temp;
                                        if((client_temp = TransLayerInstance.find_by_username_cnt(client_name_)) !=NULL) {
                                                client_temp->message_atop.type_ = PacketType::Refuse;
                                                client_temp->message_atop.respond_ = ResponseType::ErrorOccurs;
                                                client_temp->state = SessionState::Error;
                                                PreLayerInstance.pack_Message(client_temp);
                                                respond_->type_ = PacketType::Refuse;
                                                respond_->respond_ = ResponseType::AlreadyLoggedIn;
                                                PreLayerInstance.pack_Message(client_name_);
                                        }
                                        respond_->type_ = PacketType::PasswordResponse;
                                        respond_->respond_ = ResponseType::OK;
                                        PreLayerInstance.pack_Message(client_name_);
                                        client_name_->state = SessionState::ServerWaiting;
                                        respond_->type_ = PacketType::OnlineList;
                                        // TODO FIND ALL THE ONLINE USERS
                                        respond_->onlineuser_ = TransLayerInstance.find_all_user(client_name_);
                                        // respond_->config_ = DatabaseConnection::get_instance()->retrive_history_count(client_name_->host_username_);
                                        PreLayerInstance.pack_Message(client_name_);
                                        
                                        // Find all client & broadcast new online user to others
                                        std::vector<Client*> client_list_;
                                        client_list_ = TransLayerInstance.find_all_client(client_name_);
                                        std::vector<Client*>::iterator it = client_list_.begin();
                                        for(; it != client_list_.end(); it++) {
                                                (*it)->message_atop.type_ = PacketType::OnlineUser;
                                                (*it)->message_atop.user_change_ = client_name_->host_username_;
                                                PreLayerInstance.pack_Message(*it);
                                        }
                                        break;
                                }
                                case false: {
                                        // password error
                                        LOG(Info) << "Recv Wrong Password" << endl;
                                        respond_->type_ = PacketType::PasswordResponse;
                                        respond_->respond_ = ResponseType::WrongPassword;
                                        PreLayerInstance.pack_Message(client_name_);
                                        client_name_->state = SessionState::Acceptance;
                                        break;
                                }
                                break;
                        }
                        break;
                }
                case SessionState::WaitInvitResponse: {
                        // switch(message_->type_) {
                        //         case PacketType::InvitResponse: {
                        //                 LOG(Info) << "Client A recv invitation response from CLinet B" << endl;
                        //                 LOG(Info) << "Debug Enter WaitInvitResponse." << endl;
                        //                 if(message_->respond_ == ResponseType::OK) {
                        //                         // client need to know that its invitation works
                        //                         client_name_->message_atop.type_ = PacketType::InvitResponse;
                        //                         client_name_->message_atop.respond_ = ResponseType::OK;
                        //                         PreLayerInstance.pack_Message(client_name_); 
                        //                         client_name_->state = SessionState::WaitForBoard;
                        //                         // change opponent status and game info
                        //                         Client* Client_A;
                        //                         if((Client_A = TransLayerInstance.find_by_username(respond_->user_name_a_)) != NULL) {
                        //                                Client_A->state = SessionState::WaitForBoard;
                        //                                Client_A->game_info_.opponent_ = client_name_;
                        //                                client_name_->game_info_.opponent_ = Client_A;
                        //                         }
                        //                         else {
                        //                                 LOG(Error) << "Can't Find Client A after B responsed." << endl;
                        //                         }
                        //                 }
                        //                 else if(message_->respond_ == ResponseType::RefuseInvit) {
                        //                         client_name_->message_atop.type_ = PacketType::InvitResponse;
                        //                         client_name_->message_atop.respond_ = ResponseType::RefuseInvit;
                        //                         PreLayerInstance.pack_Message(client_name_); 
                        //                         client_name_->state = SessionState::ServerWaiting;
                        //                 }
                        //                 break;
                        //         }
                        // }
                        // break;
                }
                case SessionState::Responding: {
                        switch(message_->type_) {
                                case PacketType::InvitResponse: {
                                        LOG(Info) << "Client A recv invitation response from CLinet B" << endl;
                                        LOG(Info) << "Server B need to forward the message to CLient A" << endl;
                                        if(message_->respond_ == ResponseType::OK) {
                                                // Clinet A need to know that its invitation works
                                                Client* Client_A;
                                                if((Client_A = TransLayerInstance.find_by_username(respond_->user_name_a_)) != NULL) {
                                                       Client_A->message_atop.type_ = PacketType::InvitResponse;
                                                       Client_A->message_atop.respond_ = ResponseType::OK;
                                                       PreLayerInstance.pack_Message(Client_A); 
                                                       Client_A->state = SessionState::WaitForBoard;
                                                       client_name_->state = SessionState::WaitForBoard;
                                                       Client_A->game_info_.opponent_ = client_name_;
                                                       client_name_->game_info_.opponent_ = Client_A;
                                                }
                                                else {
                                                        LOG(Error) << "Can't Find Client A after B responsed." << endl;
                                                }
                                        }
                                        else if(message_->respond_ == ResponseType::RefuseInvit) {
                                                Client* Client_A;
                                                if((Client_A = TransLayerInstance.find_by_username(respond_->user_name_a_)) != NULL) {
                                                       Client_A->message_atop.type_ = PacketType::InvitResponse;
                                                       Client_A->message_atop.respond_ = ResponseType::RefuseInvit;
                                                       PreLayerInstance.pack_Message(Client_A); 
                                                       client_name_->state = SessionState::ServerWaiting;
                                                       Client_A->state = SessionState::ServerWaiting;
                                                }
                                                else {
                                                        LOG(Error) << "Can't Find Client A after B responsed." << endl;
                                                }
                                        }
                                        break;
                                }
                        }
                        break;
                }
                case SessionState::ServerWaiting: {
                        switch(message_->type_) {
                                case PacketType::SendInvit: {
                                        LOG(Info) << "Client A send invitation to Cline B" << endl;
                                        // LOG(Debug) << message_->user_name_ << endl;
                                        Client* Client_B;
                                        if((Client_B = TransLayerInstance.find_by_username(message_->user_name_b_)) !=NULL) {
                                                if(Client_B->state != SessionState::ServerWaiting) {
                                                        respond_->type_ = PacketType::InvitResponse;
                                                        respond_->respond_ = ResponseType::Busy;
                                                        PreLayerInstance.pack_Message(client_name_);
                                                }
                                                else {
                                                        Client_B->message_atop.user_name_a_ = client_name_->host_username_;
                                                        Client_B->message_atop.type_ = PacketType::RecvInvit;
                                                        PreLayerInstance.pack_Message(Client_B);
                                                        client_name_->state = SessionState::WaitInvitResponse;
                                                        Client_B->state = SessionState::Responding;
                                                }
                                        }
                                        else {
                                                respond_->type_ = PacketType::InvitResponse;
                                                respond_->respond_ = ResponseType::UserNotExist;
                                                PreLayerInstance.pack_Message(client_name_);
                                        }
                                        break;
                                }
                                // case PacketType::InvitResponse: {
                                //         LOG(Info) << "Client A recv invitation response from CLinet B" << endl;
                                //         LOG(Info) << "Server B need to forward the message to CLient A" << endl;
                                //         if(message_->respond_ == ResponseType::OK) {
                                //                 // Clinet A need to know that its invitation works
                                //                 Client* Client_A;
                                //                 if((Client_A = TransLayerInstance.find_by_username(respond_->user_name_a_)) != NULL) {
                                //                        Client_A->message_atop.type_ = PacketType::InvitResponse;
                                //                        Client_A->message_atop.respond_ = ResponseType::OK;
                                //                        PreLayerInstance.pack_Message(Client_A); 
                                //                        Client_A->state = SessionState::WaitForBoard;
                                //                        client_name_->state = SessionState::WaitForBoard;
                                //                        Client_A->game_info_.opponent_ = client_name_;
                                //                        client_name_->game_info_.opponent_ = Client_A;
                                //                 }
                                //                 else {
                                //                         LOG(Error) << "Can't Find Client A after B responsed." << endl;
                                //                 }
                                //         }
                                //         else if(message_->respond_ == ResponseType::RefuseInvit) {
                                //                 Client* Client_A;
                                //                 if((Client_A = TransLayerInstance.find_by_username(respond_->user_name_a_)) != NULL) {
                                //                        Client_A->message_atop.type_ = PacketType::InvitResponse;
                                //                        Client_A->message_atop.respond_ = ResponseType::RefuseInvit;
                                //                        PreLayerInstance.pack_Message(Client_A); 
                                //                 }
                                //                 else {
                                //                         LOG(Error) << "Can't Find Client A after B responsed." << endl;
                                //                 }
                                //         }
                                //         break;
                                // }
                        }
                        break;
                }
                case SessionState::WaitForBoard: {
                        switch(message_->type_) {
                               case PacketType::Board: {
                                        LOG(Info) << "Server 1 receive board from client" << endl;
                                        LOG(Info) << "Need to Store it in the Server 2" << endl;
                                        LOG(Info) << "Send Board information to Client 2" << endl;
                                        client_name_->state = SessionState::InGame;
                                        CopyBoard(client_name_, client_name_->game_info_.opponent_);
                                        if(client_name_->game_info_.opponent_->state != SessionState::InGame) {
                                                // I do not need to send board
                                                LOG(Info) << "Do not need to send board." << endl;
                                                break;
                                        }
                                        else {
                                                // send My board
                                                LOG(Info) << "Need to send both board" << endl;
                                                client_name_->message_atop.type_ = PacketType::Board;
                                                PreLayerInstance.pack_Message(client_name_);
                                                // send other's board
                                                client_name_->game_info_.opponent_->message_atop.type_ = PacketType::Board;
                                                PreLayerInstance.pack_Message(client_name_->game_info_.opponent_);
                                        }
                                        break;
                               }
                               break;
                        }
                        break;
                }
                case SessionState::InGame: {
                        switch(message_->type_) {
                                case PacketType::SingleCoord: {
                                        LOG(Info) << "Server receive a single coordinate!" << endl;
                                        LOG(Info) << "Server need to give the information to another side of the game." << endl;
                                        LOG(Info) << "Client " << client_name_->host_username_ << "take a step: " << message_->x << ", " << message_->y << endl;
                                        client_name_->game_info_.opponent_->message_atop.x = message_->x;
                                        client_name_->game_info_.opponent_->message_atop.y = message_->y;
                                        client_name_->game_info_.opponent_->message_atop.type_ = PacketType::SingleCoord;
                                        PreLayerInstance.pack_Message(client_name_->game_info_.opponent_);
                                        break;
                                }
                                case PacketType::DoubleCoord: {
                                        LOG(Info) << "Server receive a double coordinate!" << endl;
                                        LOG(Info) << "Server need to give the information to another side of the game." << endl;
                                        LOG(Info) << "Client " << client_name_->host_username_ << "make a guess: " << message_->head_x << ", " << message_->head_y
                                                  << "  " << message_->tail_x << ", " << message_->tail_y << endl;
                                        client_name_->game_info_.opponent_->message_atop.head_x = message_->head_x;
                                        client_name_->game_info_.opponent_->message_atop.head_y = message_->head_y;
                                        client_name_->game_info_.opponent_->message_atop.tail_x = message_->tail_x;
                                        client_name_->game_info_.opponent_->message_atop.tail_y = message_->tail_y;
                                        client_name_->game_info_.opponent_->message_atop.type_ = PacketType::DoubleCoord;
                                        PreLayerInstance.pack_Message(client_name_->game_info_.opponent_);
                                        break;
                                }
                                case PacketType::GameOver: {
                                        LOG(Info) << "Game Over!" << endl;
                                        LOG(Info) << client_name_->host_username_ << "wins!" << endl;
                                        client_name_->state = SessionState::ServerWaiting;
                                        client_name_->game_info_.opponent_->message_atop.type_ = PacketType::GameOver;
                                        PreLayerInstance.pack_Message(client_name_->game_info_.opponent_);
                                        client_name_->game_info_.opponent_->state = SessionState::ServerWaiting;
                                        break;
                                }
                        }
                        break;
                }
        }

        return ;
}

void ApplicationLayer::CopyBoard(Client* client_A, Client* client_B)
{
        std::memcpy(client_B->game_info_.win_board_, client_A->message_ptoa.board_, 100);
        std::memcpy(client_B->game_info_.plane_coord_, client_A->message_ptoa.plane_coord_, 12);

        return;
}