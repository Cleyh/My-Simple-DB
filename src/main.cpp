#include "DB_engine.h"
#include "DB_server.h"
using namespace std;
int main() 
{
	try {
		DB_server db_server("../../example-dbin");
		//db_server.start_network_server(6565);
		//run on: http://127.0.0.1:6565
		char a;
		string command;
		while (command != "exit")
		{
			cout << "$> ";
			getline(cin, command);
			if (command == "stop_network") {
				db_server.stop_network_server();
				continue;
			}
			else if (command == "start_network") {
				db_server.start_network_server(6565);
				continue;
			}
			else if (command == "") {
				continue;
			}
			db_server.cmd_handle_host(command);
		}
	}
	catch (const boost::system::system_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	
}