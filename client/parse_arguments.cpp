#include "parse_arguments.hpp"

using namespace std;

void print_help() {
	std::cout << "ʹ�÷�����" << "./ts <��ʼdevid������9λ> <�ӽ�������1-50000��>" << std::endl;
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
