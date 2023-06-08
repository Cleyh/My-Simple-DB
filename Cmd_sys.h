#pragma once
#include <map>
#include <regex>
#include <unordered_map>
#include <string>
#include "Cmd.h"
#include "DB_server.h"

//œµÕ≥÷∏¡Ó
const enum SYS_COMMAND
{
    ADD, DEL, INS,
    SET, GET,
    SEL, CLOSE,
    CREATE, REMOVE
};

extern unordered_map<string, SYS_COMMAND> sys_cmd;

class Cmd_sys :
    public Cmd
{
public:
    void show_list(vector<string> _head, vector<vector<string>> _data);
    void show_list(int row_index, vector<string> _head, vector<vector<string>> _data);
    void execute(DB_server& db, vector<string>& tokens) override;
};

