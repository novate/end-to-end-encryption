#include "../include/parse_arguments.hpp"

using namespace std;

bool is_empty_or_comment(string &s) {
	return (s.empty() || (s[0] == '#') || (s.length() >= 2 && s[0] == '/' && s[1] == '/'));
}

Options parse_arguments(ifstream &ifs) {
	Options opts;

	while (ifs) {
		string line_buf;
		string k, equal, v;
		getline(ifs, line_buf);
		istringstream iss(line_buf);
		iss >> k;
		if (is_empty_or_comment(k)) continue;

		iss >> equal;
		if (is_empty_or_comment(equal)) continue;

		iss >> v;
		if (is_empty_or_comment(v)) continue;

		if(k == "设备连接间隔" && (stoi(v) < 5 || stoi(v) > 600)) {
			v = "15";
		}
		else if(k == "设备采样间隔" && (stoi(v) < 15 || stoi(v) > 3600)) {
			v = "900";
		}
		else if(k == "未应答超时" && (stoi(v) < 2 || stoi(v) > 120)) {
			v = "30";
		}
		else if(k == "传输超时" && (stoi(v) < 2 || stoi(v) > 120)) {
			v = "60";
		}
		else if(k == "主日志大小" && (stoi(v) > 102400)) {
			v = "102400";
		}
		else if(k == "分日志大小" && (stoi(v) > 102400)) {
			v = "1024";
		}

		// insert into opts if not already exist
		opts.emplace(k, v);
	}

	// fill in default values
	for (const auto &el : default_client_config) {
		opts.insert(el);
	}

	return opts;
}
