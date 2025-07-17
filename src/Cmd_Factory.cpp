#include "Cmd_Factory.h"
#include "Cmd_sys.h"
#include "Cmd_Query.h"
using namespace std;

unique_ptr<Cmd> Cmd_Factory::create_Cmd(DB_server& db, vector<string>& tokens)
{
	if (sys_cmd.find(tokens[0]) != sys_cmd.end()) 
	{
		return make_unique<Cmd_sys>();
	}
	else if (tokens[0] == "QUERY" || tokens[0] == "query")
	{
		return make_unique<Cmd_Query>();
	}
	return nullptr;
}
