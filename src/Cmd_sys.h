#pragma once
#include <map>
#include <regex>
#include <unordered_map>
#include <string>
#include "Cmd.h"
#include "DB_server.h"

//系统指令
const enum SYS_COMMAND
{
    ADD, DEL, INS,
    SET, GET,
    SEL, CLOSE,
    CREATE, REMOVE
};

extern std::unordered_map<std::string, SYS_COMMAND> sys_cmd;

class Cmd_sys :
    public Cmd
{
public:
    void show_list(std::vector<std::string> _head, std::vector<std::vector<std::string>> _data);
    void show_list(int row_index, std::vector<std::string> _head, std::vector<std::vector<std::string>> _data);
    void execute(DB_server& db, std::vector<std::string>& tokens) override;
};

