#pragma once
#include <memory>
#include <string>
#include "Cmd.h"
#include "DB_server.h"
using namespace std;

class Cmd_Factory
{
public:
    unique_ptr<Cmd> create_Cmd(DB_server& db, vector<string>& tokens);
};

