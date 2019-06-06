#include "../include/instances.hpp"
using namespace std;
using namespace fly;

ApplicationLayer AppLayerInstance;
PresentationLayer PreLayerInstance;
TransferLayer TransLayerInstance;

void output(queue<vector <uint8_t> > &test)
{
        while(!test.empty()) {
                vector<uint8_t> test_vec = test.front();
                vector<uint8_t>::iterator it;
                for(it = test_vec.begin(); it != test_vec.end(); it++) {
                        cout << (unsigned)*it << "*";
                }
                cout << endl;
                test.pop();
        }
}

int main()
{
        // init log
        Log::get().setLogStream(cout);
        Log::get().setLevel(Debug);

        int listener = TransLayerInstance.get_listener(20343);
        TransLayerInstance.select_loop(listener);

        // int socket_fd = 100;
        // int buf_size = 10000;
        // Client client_test(socket_fd, buf_size);
        // uint8_t test_info[9] = {0,0,6,'C','y','a','n','i', 'c'};
        // client_test.recv_buffer.enqueue(test_info, 9);

        // PreLayerInstance.unpack_DataPacket(&client_test);
        // cout << client_test.recv_buffer.has_complete_packet() << endl;
        // output(client_test.send_buffer);
	
	// uint8_t test_passwd[9] = {2, 0, 6, '1', '2', '3', '4', '5', '6'};
	// client_test.recv_buffer.enqueue(test_passwd, 9);
        // PreLayerInstance.unpack_DataPacket(&client_test);
        // cout << client_test.message_ptoa.password_ << endl;
        // output(client_test.send_buffer);

	// uint8_t test_new_passwd[9] = {2, 0, 6, '6', '2', '5', '3', '2', '1'};
	// client_test.recv_buffer.enqueue(test_new_passwd, 9);
        // PreLayerInstance.unpack_DataPacket(&client_test);
        // cout << client_test.message_ptoa.password_ << endl;
        // output(client_test.send_buffer);

        // uint8_t test_text_user[9] = {9,0,6,'Y','Y','Y','u','n', 'a'};
	// client_test.recv_buffer.enqueue(test_text_user, 9);
        // PreLayerInstance.unpack_DataPacket(&client_test);
        // cout << client_test.state << endl;

        // uint8_t test_text[9] = {10,0,6,'A','B','C','D','E', 'F'};
	// client_test.recv_buffer.enqueue(test_text, 9);
        // PreLayerInstance.unpack_DataPacket(&client_test);
        // cout << client_test.message_ptoa.user_name_ << endl;
        // cout << client_test.state << endl;

        return 0;
}
