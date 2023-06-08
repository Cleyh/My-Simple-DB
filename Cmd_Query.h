#pragma once
#include "Cmd.h"
class Cmd_Query :
    public Cmd
{
	void execute(DB_server&, std::vector<std::string>&);
};

