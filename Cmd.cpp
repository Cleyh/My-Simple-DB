#include "Cmd.h"

void Cmd::return_data(std::string& data)
{
    _r = data;
    boost::system::error_code error;
    //boost::asio::write(_socket, boost::asio::buffer(data),error);
    if (error) {
        cout << "Failed to send data to client" << endl;
    }
}
