#include "parse_arguments.hpp"

using namespace std;

void print_help() {
	std::cout << "使用方法：" << "./ts <初始devid，必须9位> <子进程数（1-50000）>" << std::endl;
}

bool is_empty_or_comment(string &s) {
	return (s.empty() || (s[0] == '#'));
}

Options parse_arguments(ifstream &ifs, bool is_client) {
	Options opts;

	while (ifs) {
		string line_buf;
		string k, equal, v;
		getline(ifs, line_buf);
		istringstream iss(line_buf);
		iss >> k;
		if (is_empty_or_comment(k)) continue;

		if (!is_client) {
			iss >> equal;
			if (is_empty_or_comment(equal)) continue;
		}

		iss >> v;
		if (is_empty_or_comment(v)) continue;

		// insert into opts if not already exist
		opts.emplace(k, v);
	}

	// fill in default values
	if (is_client) {
		for (const auto &el : default_client_config) {
			opts.insert(el);
		}
	}


	return opts;
}



//////////////////////////////      test      ///////////////////////////
// Options get_test_arguments() {
// 	Options opts;
// 	return opts;
// }

// Options test(bool is_client, bool is_clean) {
// 	string fn_conf = is_client ? kFnConfClient : kFnConfServer;
// 	fn_conf = (is_clean ? "clean_" : "disordered_") + fn_conf;
// 	ifstream ifs(fn_conf);

// 	Options opt = parse_arguments(ifs, is_client);
// 	// print opt
// 	cout << "========= opt ==========\n"
// 		<< "size: " << opt.size() << endl;
// 	for (auto it = opt.begin(); it != opt.end(); it++) {
// 		cout << "k: " << it->first << endl
// 		       	<< "v: " << it->second << endl << endl;
// 	}
// 	return opt;
// }

// int main(int argc, char *argv[]) {
// 	cout << argv[1] << " " << argv[2] << endl;
// 	bool is_client = strncmp("client", argv[1], 6) == 0;
// 	bool is_clean  = strncmp("clean", argv[2], 5) == 0;
// 	cout << "is_client: " << boolalpha << is_client << endl
// 	       << "is_clean: " << is_clean << endl << endl;

// 	test(is_client, is_clean);
// }
