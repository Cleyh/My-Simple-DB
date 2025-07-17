#pragma once
#include <memory>
#include <string>
#include "Cmd.h"
#include "DB_server.h"

class Cmd_Factory
{
public:
    std::unique_ptr<Cmd> create_Cmd(DB_server& db, std::vector<std::string>& tokens);
};

