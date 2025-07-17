#include "DB_server.h"
#include "Cmd.h"
#include "Cmd_Factory.h"
using namespace std;

/*
 * DB_server的实现
 */

DB_server::DB_server(string path)
    : _sf_path(path)
{
    load_config(_sf_path);
    _server = nullptr;
}

DB_server::~DB_server()
{
    for (auto &item : _db_map)
    {
        delete item.second;
    }
}

void DB_server::load_config(string path)
{
    if (path == "default")
        path = _sf_path;
    else
        _sf_path = path;

    string db_list_path = path + "/config/db_list_config.json";

    ifstream file(db_list_path);

    if (!file.is_open())
    {
        cerr << "Error loading DB" << endl;
        return;
    }

    _sf_db.clear();

    json j;
    file >> j;

    for (auto &db : j["db"])
    {
        DB_CONFIG new_db(db["name"]);
        for (auto &ls : db["list"])
        {
            new_db._list_.push_back(LIST_CONFIG(ls["name"]));
        }
        _sf_db.push_back(new_db);
    }

    string config_path = path + "/config/config.json";
    // 待定
}

bool DB_server::save_config(string path)
{
    if (path == "default")
        path = _sf_path;
    else
        _sf_path = path;

    string db_list_path = path + "/config/db_list_config.json";

    ofstream file(db_list_path);

    if (!file.is_open())
    {
        cerr << "Saving server config failed, error opening file!" << endl;
        return false;
    }

    json j;

    for (auto &n : _sf_db)
    {
        json j_list;
        j_list["name"] = n.get_name();
        for (auto &m : n._list_)
        {
            json d_list;
            d_list["name"] = m.get_name();
            j_list["list"].push_back(d_list);
        }
        j["db"].push_back(j_list);
    }

    file << setw(4) << j << endl;
    file.close();
    return true;
}

DB_engine *DB_server::use_db(string name)
{
    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == name; });
    if (it == _sf_db.end())
    {
        // cerr << "You are selecting an inxistent DB" << endl;
        return nullptr;
    }
    return use_db(it - _sf_db.begin());
}

DB_engine *DB_server::use_db(int sid)
{
    if (sid < 0 || sid >= _sf_db.size())
    {
        // cerr << "You are selecting an inxistent DataBase." << endl;
        return nullptr;
    }

    thread::id this_id = std::this_thread::get_id();
    if (_db_map.find(this_id) == _db_map.end())
    {
        // 如果当前线程的 DB_engine 实例不存在，则创建一个新实例
        _db_map[this_id] = new DB_engine(_sf_path + "/db/" + _sf_db[sid].get_name());
    }
    else
    {
        // 如果当前线程的 DB_engine 实例已存在，则先关闭它
        close_DB();
        // 然后创建一个新实例
        _db_map[this_id] = new DB_engine(_sf_path + "/db/" + _sf_db[sid].get_name());
    }
    cout << "You selected the DataBase: " << _sf_db[sid].get_name() << endl;
    return _db_map[this_id];
}

bool DB_server::close_DB()
{
    std::thread::id this_id = std::this_thread::get_id();
    if (_db_map.find(this_id) == _db_map.end())
        return false;

    DB_engine *db_instance = _db_map[this_id];
    db_instance->close_list();
    db_instance->save_DB();

    delete db_instance;
    _db_map.erase(this_id);

    return true;
}

bool DB_server::is_use()
{
    std::thread::id this_id = std::this_thread::get_id();
    auto it = _db_map.find(this_id);
    if (it == _db_map.end())
        return false;
    return true;
}

DB_engine *DB_server::DB_op()
{
    std::thread::id this_id = std::this_thread::get_id();
    auto it = _db_map.find(this_id);
    if (it == _db_map.end())
        return nullptr;
    else
        return _db_map[this_id];
}

bool DB_server::new_db(DB_CONFIG &new_db_cf)
{
    /*
    //设置uid
    for (auto& db : _sf_db)
    {
        if (db.get_name() == new_db_cf.get_name()) {
            new_db_cf.set_name(new_db_cf.get_name() + "_c");
        }
    }
    new_db_cf.set_uid(_sf_db.size());
    */
    // 文件读写
    string saving_path = _sf_path + "/db/" + new_db_cf.get_name();

    if (!filesystem::create_directories(saving_path))
    {
        cerr << "Create database failed, becuase create floder failed！" << endl;
        return false;
    }

    DB_engine saving(new_db_cf, saving_path);
    if (!saving.save_DB())
    {
        return false;
    }

    _sf_db.push_back(new_db_cf);
    save_config();
    return true;
}

bool DB_server::del_db(int uid)
{
    if (uid >= _sf_db.size() || uid < 0)
    {
        cerr << "You are deleting an inxistent DB" << endl;
        return false;
    }

    std::string db_name = _sf_db[uid].get_name();
    for (const auto &entry : _db_map)
    {
        if (entry.second->get_name() == db_name)
        {
            cerr << "Cannot delete database because it is currently in use" << endl;
            return false;
        }
    }

    string list_path = _sf_path + "/db/" + _sf_db[uid].get_name();
    if (filesystem::remove_all(list_path))
        cerr << "Delete failed!" << endl;

    cout << "Delete DataBase successful!" << _sf_db[uid].get_name() << endl;

    _sf_db.erase(_sf_db.begin() + uid);
    save_config();
    return true;
}

bool DB_server::is_args_right(string db_name, string ls_name)
{
    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });

    if (it == _sf_db.end())
        return false;
    else
    {
        auto it1 = std::find_if(it->_list_.begin(), it->_list_.end(),
                                [&](LIST_CONFIG &item)
                                { return item.get_name() == ls_name; });
        if (it1 == it->_list_.end())
            return false;
    }
    return true;
}

vector<string> DB_server::query_srow(string db_name, string ls_name, int row)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);
    if (row >= db.LS_op()->_data.size() || row < 0)
        return vector<string>();
    else
        return db.LS_op()->_data[row];
}

vector<string> DB_server::query_srow(DB_engine &db, int row)
{
    if (row >= db.LS_op()->_data.size() || row < 0)
        return vector<string>();
    return db.LS_op()->_data[row];
}

vector<string> DB_server::query_scol(string db_name, string ls_name, int col)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);

    if (col >= db.LS_op()->_head.size() || col < 0)
        return vector<string>();

    vector<string> result;
    // result.push_back(db.LS_op()->_head[col]);

    for (auto &items : db.LS_op()->_data)
    {
        result.push_back(items[col]);
    }

    return result;
}

vector<string> DB_server::query_scol(DB_engine &db, int col)
{
    if (col >= db.LS_op()->_head.size() || col < 0)
        return vector<string>();

    vector<string> result;
    // result.push_back(db.LS_op()->_head[col]);

    for (auto &items : db.LS_op()->_data)
    {
        result.push_back(items[col]);
    }

    return result;
}

vector<vector<string>> DB_server::query_rows(string db_name, string ls_name, vector<int> &row)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();

    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);
    vector<vector<string>> result;
    result.reserve(row.size()); // 预分配内存
    for (auto &i : row)
        result.push_back(query_srow(db, i));
    return result;
}

vector<vector<string>> DB_server::query_rows(string db_name, string ls_name)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);
    int all = db.ls_ls().size();

    vector<vector<string>> result;
    result.reserve(all);
    for (int i = 0; i < all; i++)
        result.push_back(query_srow(db, i));
    return result;
}

vector<vector<string>> DB_server::query_cols(string db_name, string ls_name, vector<int> &col)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });

    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);
    db.select_list(ls_name);

    vector<vector<string>> result;
    result.reserve(col.size()); // 预分配内存
    for (auto &i : col)
        result.push_back(query_scol(db, i));
    return result;
}

vector<vector<string>> DB_server::query_Q(string db_name, string ls_name, string head, string target)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);

    // 找到 head 列在表中的位置
    int col = -1;
    for (size_t i = 0; i < db.LS_op()->_head.size(); i++)
    {
        if (db.LS_op()->_head[i] == head)
        {
            col = i;
            break;
        }
    }
    if (col == -1)
        return {};

    // 遍历表中的所有行，找到那些在 head 列下，数据为 target 的行
    vector<vector<string>> result;
    for (const auto &row : db.LS_op()->_data)
    {
        if (row[col] == target)
        {
            result.push_back(row);
        }
    }

    return result;
}

vector<int> DB_server::query_w(string db_name, string ls_name, string head, string target)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);

    // 找到 head 列在表中的位置
    int col = -1;
    for (size_t i = 0; i < db.LS_op()->_head.size(); i++)
    {
        if (db.LS_op()->_head[i] == head)
        {
            col = i;
            break;
        }
    }
    if (col == -1)
        return vector<int>();

    // 遍历表中的所有行，找到那些在 head 列下，数据为 target 的行
    vector<int> result;
    for (size_t i = 0; i < db.LS_op()->_data.size(); i++)
    {
        if (db.LS_op()->_data[i][col] == target)
        {
            result.push_back(i);
        }
    }

    return result;
}

vector<string> DB_server::query_head(string db_name, string ls_name)
{
    if (!is_args_right(db_name, ls_name))
        return {};

    auto it = std::find_if(_sf_db.begin(), _sf_db.end(),
                           [&](DB_CONFIG &item)
                           { return item.get_name() == db_name; });
    int id = it - _sf_db.begin();
    string path = _sf_path + "/db/" + _sf_db[id].get_name();
    DB_engine db(path);

    db.select_list(ls_name);
    return db.LS_op()->_head;
}

string DB_server::cmd_handle(tcp::socket &g_socket, string cmd)
{
    vector<string> result;
    istringstream iss(cmd);
    string token;
    while (iss >> token)
    {
        result.push_back(token);
    }
    unique_ptr<Cmd> _cmd;
    _cmd = Cmd_Factory().create_Cmd(*this, result);
    if (!_cmd)
        return string();

    _cmd->set_socket(g_socket);
    _cmd->execute(*this, result);
    string r = _cmd->_r;
    return r;
}

bool DB_server::cmd_handle_host(string cmd)
{
    vector<string> result;
    istringstream iss(cmd);
    string token;
    while (iss >> token)
    {
        result.push_back(token);
    }
    unique_ptr<Cmd> _cmd;
    _cmd = Cmd_Factory().create_Cmd(*this, result);
    if (!_cmd)
        return false;

    _cmd->execute(*this, result);
    return true;
}

bool DB_server::start_network_server(unsigned int port)
{
    if (_server)
    {
        return false;
    }
    _server = new network_server(port, this);
    _server->start();
    return true;
}

bool DB_server::stop_network_server()
{
    if (_server)
    {
        _server->stop();
        delete[] _server;
        return true;
    }
    return false;
}

/*
 * net_work_server的实现
 */

void DB_server::network_server::network_server::cleanup_threads()
{
    auto it = _client_threads.begin();
    while (it != _client_threads.end())
    {
        if (!it->joinable())
        {
            it = _client_threads.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DB_server::network_server::handle_client(tcp::socket _socket)
{
    cout << "New client connected" << endl;
    std::cout << "Client connected: " << _socket.remote_endpoint().address().to_string() << ":" << _socket.remote_endpoint().port() << std::endl;

    for (;;)
    {
        std::string cmd;
        boost::system::error_code error;
        asio::read_until(_socket, asio::dynamic_buffer(cmd), "\n", error);
        if (error)
        {
            _parent->close_DB();
            cout << "Client disconnected" << endl;
            break;
        }
        cout << "Received command: " << cmd;
        string result = _parent->cmd_handle(_socket, cmd);

        result += '\n';
        uint32_t length = htonl(result.size()); // 将字符串长度转换为网络字节序
        // boost::asio::write(_socket, boost::asio::buffer(&length, sizeof(length))); // 发送字符串长度
        boost::asio::write(_socket, boost::asio::buffer(result)); // 发送字符串
        // 异步发送字符串长度
        // boost::asio::async_write(_socket, boost::asio::buffer(&length, sizeof(length)), [](const boost::system::error_code&, std::size_t) {});
        // 异步发送字符串
        // boost::asio::async_write(_socket, boost::asio::buffer(result), [](const boost::system::error_code&, std::size_t) {});
        // io_context.run();
        cout << length << endl;
        cout << result << endl;
        cout << endl;
    }
    _parent->close_DB();
    _socket.close();
}

DB_server::network_server::~network_server()
{
    stop();
    if (_acceptor)
        delete _acceptor;
}

DB_server::network_server::network_server(int port, DB_server *parent)
    : _port(port), _parent(parent), _is_stop(true)
{
    _acceptor = nullptr;
}

void DB_server::network_server::start()
{
    cout << "Starting server on port " << _port << endl;

    _acceptor = new tcp::acceptor(io_context, tcp::endpoint(tcp::v4(), _port)); // 保存 acceptor 对象的地址
    std::cout << "Listening on " << _acceptor->local_endpoint().address().to_string() << " : " << _acceptor->local_endpoint().port() << std::endl;

    _is_stop = false;

    while (!_is_stop)
    {
        tcp::socket socket(io_context);
        _acceptor->accept(socket);

        cleanup_threads();
        // 为每个客户端连接创建一个新线程，并将其添加到 _client_threads 中
        _client_threads.push_back(thread(&network_server::handle_client, this, std::move(socket)));
    }

    // 等待所有客户端连接的线程都结束
    for (auto &t : _client_threads)
        t.join();
}

void DB_server::network_server::stop()
{
    _is_stop = true;
    if (_acceptor)
        _acceptor->close();
}