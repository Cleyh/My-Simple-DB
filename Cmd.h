#pragma once
#include <vector>
#include <string>
#include "DB_server.h"
using namespace std;

class Cmd
{
public:
	virtual ~Cmd() {};
	virtual void execute(DB_server&, std::vector<std::string>&) = 0;
	virtual void return_data(std::string& data);
	void set_socket(tcp::socket& socket) { _socket = &socket; _socket_iset = true; }
	bool is_set_socket() { return _socket_iset; }
	string _r;

public:
	json vector_to_json(const vector<vector<string>>& data, string type);
	json hint_to_json(const string& type, const string& content, const string& code);

private:
	tcp::socket* _socket;
	bool _socket_iset = false;
};

