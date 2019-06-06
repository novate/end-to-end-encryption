#include "shared_library.hpp"

int log_init(std::ofstream &log_stream, const std::string log_name, const Level level, const bool* const log_env, const bool on_screen, const bool is_trunc) {
    // log_stream must not be opened before getting into this function.
    if (log_stream.is_open()) {
        return -1;
    }
	if (is_trunc) {
		log_stream.open(log_name, ios::out|ios::trunc);
	}
    else {
		log_stream.open(log_name, ios::out|ios::app);
	}
    if (!log_stream.is_open()) {
        return -2;
    }
    Log::get().setLogStream(log_stream);
    Log::get().setLevel(level);
    Log::get().setEnv(log_env);
    Log::get().setOnScreen(on_screen);
    return 0;
}

std::string logify_data(const uint8_t* data, const int len) {
    std::stringstream ss, ss_word;
    // ss_word.str(std::string());
    int i;
    for (i = 0; i < len; i++) {
        if (i % 16 == 0) {
            ss << ss_word.str() << std::endl;
            ss_word.clear();    //clear any bits set
            ss_word.str(std::string());
            ss << ' ' << setw(4) << setfill('0') << hex << uppercase << i << ": ";
        }
        else if (i % 8 == 0) {
            ss << "- ";
        }
        ss << setw(2) << setfill('0') << hex << uppercase << +data[i] << ' ';
        // print printable char.
        char ch = (data[i] > 31 && data[i] < 127) ? data[i] : '.';
        ss_word << ch;
        // ss_word << data[i];
    }  
    if (i%16==0){
        ss << setw(0) << ss_word.str();
    }
    else {
        auto interval = 3 * (16 - (i % 16)) + (i % 16 > 8 ? 0 : 2);
        // cout << "i: " << i << ", interval: " << interval << endl;
        ss << setw(interval) << setfill(' ') << ' ' << setw(0) << ss_word.str();
    }
    return ss.str();
}

void encrypt_auth(u_int& random_num, u_int& svr_time, uint8_t* auth, const int length) {
    svr_time = (u_int)time(0);
    svr_time = svr_time ^ (u_int)0xFFFFFFFF;
    srand(svr_time);
    random_num = (u_int)rand();
    int pos = random_num % 4093;
    for (int i = 0; i < length; i++) {
        auth[i] = auth[i] ^ kSecret[pos];
        pos = (pos+1)%4093;
    }
}

bool decrypt_auth(const u_int random_num, uint8_t* auth, const int length) {
	const uint8_t c_auth[33] = "yzmond:id*str&to!tongji@by#Auth^";
    int pos = random_num % 4093;
    for (int i = 0; i < length; i++) {
        auth[i] = auth[i] ^ kSecret[pos];
		if (i > length-32 && auth[i] != c_auth[i-length+32]) {
			return false;
		}
        pos = (pos+1)%4093;
    }
	return true;
}

void create_random_str(uint8_t* random_string, const int length) {
    uint8_t *p = new uint8_t[length];
    srand((unsigned)time(NULL));
    for(int i = 0; i < length; i++) {
        p[i] = rand() % 256;
    }
    memcpy(random_string, p, length);
    delete[] p;
    return;
}

// return size of the file, if open file error, return 0.
// must read no more than maxLength(8191/4095) bytes.
int read_dat(const std::string dat_name, char* result, int maxLength) {
	ifstream ifs;
	ifs.open(dat_name, std::ifstream::in);
	if (!ifs.is_open()) {
		return -1;
	}
	// read no more than 8191 bytes.
	ifs.seekg(0, std::ios::end);
	int length = ifs.tellg();
	length = length > maxLength ? maxLength : length;
	ifs.seekg(0, std::ios::beg);
	ifs.read(result, length);
	ifs.close();
	return length;
}
