#ifndef PRESENT_H
#define PRESENT_H
#include "../include/presentation.hpp"
// #include "../include/application.hpp"
// #include "../include/transfer.hpp"

extern ApplicationLayer AppLayerInstance;
extern TransferLayer TransLayerInstance;

using namespace std;

void PresentationLayer::pack_ErrorOccurs(Client* client) {
    vector<uint8_t> temp;

    //descriptor
    uint8_t descriptor = (uint8_t) PacketType::Refuse;
    temp.clear();
    temp.push_back(descriptor);

    //length = 1
    temp.push_back((uint8_t)0);
    temp.push_back((uint8_t)1);

    //response
    temp.push_back((uint8_t)ResponseType::ErrorOccurs);

    client->send_buffer.push(temp);    

    return;
}

void PresentationLayer::broadcast_Offline(Client* client) {
    AppLayerInstance.BroadcastOffline(client);

    return;
}

bool PresentationLayer::check_passwordFormat(unsigned char *password){
    unsigned char * ptr = password;
    if(!strcmp((char *)password, "123456")) return true;
    int count = 0;
    while((*ptr) != '\0'){
        count ++;
        ptr ++;
    }

    if(count < 8 || count > 28)
        return false;
    return true;
}

vector<uint8_t> PresentationLayer::pack_Response(Message_To_Pre message){
    static vector<uint8_t> temp;
    uint8_t* descriptor;
    uint16_t length;

    //descriptor
    descriptor = (uint8_t *)&message.type_;
    temp.clear();
    temp.push_back(*descriptor);

    switch(message.type_){
        case PacketType::SyncEnd:
            //length = 0
            temp.push_back((uint8_t)0); 
            temp.push_back((uint8_t)0);
            break;

        case PacketType::InfoResponse:
        case PacketType::PasswordResponse:
        case PacketType::Refuse:
        case PacketType::InvitResponse:
            //length = 1
            length = (uint16_t)1;
            temp.push_back((uint8_t)(length >> 8) ); 
            temp.push_back((uint8_t)length );
            temp.push_back(*((uint8_t*)&message.respond_));  
            break;
    }

    return temp;
}

vector<uint8_t> PresentationLayer::pack_Invit(Message_To_Pre message){
    vector<uint8_t> temp;
    uint16_t length;
    string str;

    // descriptor
    temp.push_back((uint8_t)message.type_);

    //push_back user name length
    length = (uint16_t)(message.user_name_a_.length() );
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );

    //push_back user name
    const char* c;
    c = message.user_name_a_.c_str();
    while((*c) != '\0'){
        temp.push_back((uint8_t)(*c) );
        c++;
    }

    return temp;
}

vector<uint8_t> PresentationLayer::pack_UserName(Message_To_Pre * message){
    vector<uint8_t> temp;
    uint16_t length;
    string str;

    // descriptor
    temp.push_back((uint8_t)PacketType::UserName);

    //push_back user name length
    str = *(message->onlineuser_.begin()); 
    length = (uint16_t)(str.length());
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );

    //push_back user name
    const char* c;
    c = str.c_str();
    while((*c) != '\0'){
        temp.push_back((uint8_t)(*c) );
        c++;
    }

    //erase host name
    message->onlineuser_.erase(message->onlineuser_.begin());     
    
    return temp;
}


std::vector<uint8_t> PresentationLayer::pack_UserChange(Message_To_Pre message) {
    vector<uint8_t> temp;

    //descriptor
    temp.push_back((uint8_t)message.type_);

    //push_back user name length
    string str = message.user_change_;
    uint16_t length = (uint16_t)(str.length());
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );
    
    //push back user name
    const char* c;
    c = str.c_str();
    while((*c) != '\0'){
        temp.push_back((uint8_t)(*c) );
        c++;
    }

    return temp;
}

vector<uint8_t> PresentationLayer::pack_Board(struct GameInfo game_info_){
    vector<uint8_t> temp;
    
    // descriptor
    temp.push_back((uint8_t)PacketType::Board);

    // length
    int length = (uint16_t)12;
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );

    //plane coordinate
    // std::vector<uint8_t> temp(game_info_.plane_coord_, game_info_.plane_coord_ + 12);
    for(int i = 0; i < 12; i++){
        temp.push_back(game_info_.plane_coord_[i]);
    }
    return temp;
}

vector<uint8_t> PresentationLayer::pack_SingleCoord(Message_To_Pre message){
    vector<uint8_t> temp;

    // descriptor
    temp.push_back((uint8_t)PacketType::SingleCoord);

    // length
    int length = (uint16_t)2;
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );

    // single
    temp.push_back((uint8_t)message.x);
    temp.push_back((uint8_t)message.y);

    return temp;
}

vector<uint8_t> PresentationLayer::pack_DoubleCoord(Message_To_Pre message){
    vector<uint8_t> temp;

    // descriptor
    temp.push_back((uint8_t)PacketType::DoubleCoord);

    // length
    int length = (uint16_t)4;
    temp.push_back((uint8_t)(length >> 8) );
    temp.push_back((uint8_t)(length) );

    // single
    temp.push_back((uint8_t)message.head_x);
    temp.push_back((uint8_t)message.head_y);
    temp.push_back((uint8_t)message.tail_x);
    temp.push_back((uint8_t)message.tail_y);

    return temp;
}

StatusCode PresentationLayer::pack_Message(Client *client){
    Client *recv_client;
    DataPacket packet;
    Message_To_Pre message, message_temp;
    vector<uint8_t> temp_str;
    unsigned char *temp_data;

    message = client->message_atop;

    // special handling for multi-login
    if(message.type_ == PacketType::Refuse){
        //TODO: refuse and disconnect
        client->send_buffer.push(pack_Response(message));
    }
    
    // Broadcast the new log in information
    if(message.type_ == PacketType::OnlineUser || message.type_ == PacketType::OfflineUser){
        client->send_buffer.push(pack_UserChange(message));
    }

    //cases based on status

    if((client->state == SessionState::Acceptance)) {
        if(message.type_ == PacketType::InfoResponse) {
            temp_str = pack_Response(message);
            client->send_buffer.push(temp_str);
        }
    }

    //WaitForPasswd, Error:
    if((client->state == SessionState::WaitForPasswd) ) {
        switch (message.type_)
        {
        case PacketType::PasswordResponse:
            temp_str = pack_Response(message);
            client->send_buffer.push(temp_str);
            break;
        }
    }

    //ServerWaiting: 
    if((client->state == SessionState::ServerWaiting) ){
        //synchronize 
        if(message.type_ == PacketType::OnlineList){
            //sync online user list 
            while(message.onlineuser_.size() != 0){
                //online user names
                temp_str = pack_UserName(&message);
                client->send_buffer.push(temp_str);
                //history
                // temp_str = pack_History(&message);
                // client->send_buffer.push(temp_str);
            }
            cout << "client send_buffer length " << client->send_buffer.size() << endl;

            //Sync End
            message_temp.type_ = PacketType::SyncEnd;
            client->send_buffer.push(pack_Response(message_temp));
        }

        // RecvInvit
        if(message.type_ == PacketType::RecvInvit){
            temp_str = pack_Invit(message);
            client->send_buffer.push(temp_str);
        }

        //InvitResponse
        if(message.type_ == PacketType::InvitResponse){
            temp_str = pack_Response(message);
            client->send_buffer.push(temp_str);
        }
    }
    
    //WaitInfoResponse:
    if((client->state == SessionState::WaitInvitResponse) ) {
        if(message.type_ == PacketType::InvitResponse){
            client->send_buffer.push(pack_Response(message) );
        }
    }

    //InGame:
    if(client->state == SessionState::InGame){
        //send board set by opponent
        if(message.type_ == PacketType::Board){
            temp_str = pack_Board(client->game_info_);
            client->send_buffer.push(temp_str);
        }

        //single coord
        if(message.type_ == PacketType::SingleCoord){
            temp_str = pack_SingleCoord(message);
            client->send_buffer.push(temp_str);
        }

        //double coord
        if(message.type_ == PacketType::DoubleCoord){
            temp_str = pack_DoubleCoord(message);
            client->send_buffer.push(temp_str);
        }

        //gameOver
        if(message.type_ == PacketType::GameOver){
            std::vector<uint8_t> temp;
            temp.push_back((uint8_t)PacketType::GameOver);
            temp.push_back((uint8_t)0);
            temp.push_back((uint8_t)0);

            client->send_buffer.push(temp);
        }
    }

    return StatusCode::OK; 
}

StatusCode PresentationLayer::unpack_DataPacket(Client *client){
    Message_To_Pre message_atop;
    while( client->recv_buffer.has_complete_packet()){
        //client->recv_buffer.has_complete_packet()
        DataPacket packet;
        Message_To_App *message = &client->message_ptoa;
        unsigned char *temp_data;

        //packet from TransLayer
        packet = client->recv_buffer.dequeue_packet();
        packet_size = packet.data.size() + 3;

        //string unpacking 
        if((packet.type == PacketType::Info) 
            || (packet.type == PacketType::Password)
            || (packet.type == PacketType::SendInvit) ){
            temp_data = unpack_String(packet);
            switch(packet.type){
                case PacketType::Info:
                    message->user_name_ = (char *)temp_data;
                    break;
                case PacketType::Password:
                    message->password_ = (char *)temp_data;
                    break;
                case PacketType::SendInvit:
                    message->user_name_b_ = (char *)temp_data;
                    break;
            }
        }//end of if
        
        //response unpacking
        if((packet.type == PacketType::InvitResponse) ){
            message->respond_ = unpack_Response(packet);
        }

        //board unpacking
        if((packet.type == PacketType::Board) ){
            unpack_Board(packet, message);
        }

        //single coord unpacking
        if(packet.type == PacketType::SingleCoord){
            unpack_SingleCoord(packet, message);
        }

        //double coord unpacking
        if(packet.type == PacketType::DoubleCoord){
            unpack_DoubleCoord(packet, message);
        }

        //GameOver
        if(packet.type == PacketType::GameOver) {
            ;
        }

        client->message_ptoa.type_ = packet.type;
        // cout << (packet.type == PacketType::Password) << endl;
        // cout << (client->message_ptoa.type_ == PacketType::Password) << endl;
    
        // // cout << messageuser_name_ << endl;
        // // cout << &client << endl;
        // // cout << "debug2" << endl;
        AppLayerInstance.MessageToApp(client);
    }
    return StatusCode::OK;
}

unsigned char * PresentationLayer::unpack_String(DataPacket packet){
    vector<uint8_t>::iterator iter;
    static unsigned char temp[kMaxPacketLength];

    int data_len = 0;
    for(iter = packet.data.begin(); iter != packet.data.end(); iter++){
        temp[data_len++] = *iter;
    }
    temp[data_len] = '\0';
    
    return temp;
}

ResponseType PresentationLayer::unpack_Response(DataPacket packet){
    vector<uint8_t>::iterator iter;

    iter = packet.data.begin();

    return (ResponseType)(*iter);
}

//function:
//   this will retrun a 5x5 plane in upward
void PresentationLayer::plane(int curr, int *Plane[5]){
    for(int x = 0; x < 5; x++)
        Plane[x][1] = curr*10;
    
    Plane[2][2] = Plane[1][3] = Plane[3][3] = curr*10;
    Plane[2][0] = curr*10+1;
    Plane[2][3] = curr*10+2;
    
    return;
}

void PresentationLayer::unpack_Board(DataPacket packet, Message_To_App * message){
    int (*board)[10];
    board = message->board_;
    vector<uint8_t>::iterator iter;

    int x1,x2,y1,y2,curr = 1;
    for(iter = packet.data.begin(); iter != packet.data.end(); iter+=4){
        x1 = (int)(*iter - '0');
        y1 = (int)(*(iter+1) - '0');
        x2 = (int)(*(iter+2) - '0');
        y2 = (int)(*(iter+3) - '0');
        
        message->plane_coord_[(curr-1)*4  ] = (int)(*iter);
        message->plane_coord_[(curr-1)*4+1] = (int)(*(iter+1));
        message->plane_coord_[(curr-1)*4+2] = (int)(*(iter+2));
        message->plane_coord_[(curr-1)*4+3] = (int)(*(iter+3));

        //head & tail
        board[x1][y2] = curr*10+1;
        board[x2][y2] = curr*10+2;

        //body
        if(x1 == x2){    //vertical
            if(y1 < y2){    //up
                //the fuselage
                for(int y = y1+1; y < y2; y++)
                    board[x1][y] = curr*10;
                //the wing
                for(int x = x1-2; x <= x1+2; x++)
                    board[x][y1+1] = curr*10;
                //the tail
                board[x1-1][y2] = board[x1+1][y2] = curr*10;
            }
            else{    //down
                // the fuselage
                for(int y = y1-1; y > y2; y--)
                    board[x1][y] = curr*10;
                //the wing
                for(int x = x1-2; x <= x1+2; x++)
                    board[x][y1-1] = curr*10;
                //the tail
                board[x1-1][y2] = board[x1+1][y2] = curr*10;
            }
        }
        else{   //horizental
            if(x1 < x2){    //left
                //the fuselage
                for(int x = x1+1; x < x2; x++)
                    board[x][y1] = curr*10;
                //the wing
                for(int y = y1-2; y <= y1+2; y++)
                    board[x1+1][y] = curr*10;
                //the tail
                board[x2][y1-1] = board[x2][y1+1] = curr*10;
            }
            else{    //right
                //the fuselage
                for(int x = x1-1; x > x2; x--)
                    board[x][y1] = curr*10;
                //the wing
                for(int y = y1-2; y <= y1+2; y++)
                    board[x1-1][y] = curr*10;
                //the tail
                board[x2][y1-1] = board[x2][y1+1] = curr*10;
            }
        }
        curr += 1;
    }//end of for

    return;
}

void PresentationLayer::unpack_SingleCoord(DataPacket packet, Message_To_App * message){
    vector<uint8_t>::iterator iter;
    
    iter = packet.data.begin();
    message->x = (int)(*iter - '0');
    message->y = (int)(*(iter+1) - '0');
    
    return;
}

void PresentationLayer::unpack_DoubleCoord(DataPacket packet, Message_To_App * message){
    vector<uint8_t>::iterator iter;
    
    iter = packet.data.begin();
    message->head_x = (int)(*iter - '0');
    message->head_y = (int)(*(iter+1) - '0');
    message->tail_x = (int)(*(iter+2)) - '0';
    message->tail_y = (int)(*(iter+3) - '0');

    return;
}

#endif
