#include "Cmd.h"
using namespace std;

void Cmd::return_data(std::string& data)
{
    _r = data;
    boost::system::error_code error;
    //boost::asio::write(_socket, boost::asio::buffer(data),error);
    if (error) {
        cout << "Failed to send data to client" << endl;
    }
}

json Cmd::vector_to_json(const vector<vector<string>>& data, string type)
{
    /*
    * {
    *   "type":"data",
    *   "data"[
    *       ["xxx","yyy","zzz"],
    *       ["data","data","data"],
    *       ["data","data","data"]
    *   ]
    * }
    */
    json j;
    j["type"] = type;
    j["data"] = json::array();
    for (const auto& row : data)
    {
        j["data"].push_back(row);
    }
    return j;
}

json Cmd::hint_to_json(const string& type, const string& content, const string& code)
{
    json j;
    j["type"] = type;
    j["data"] = { { "code", code }, { content } };
    return j;
}
