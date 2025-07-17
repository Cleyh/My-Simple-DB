#pragma once

#include "DB_engine.h"
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

/*
* 数据库配置信息
* 
* 这是一个数据库的基本信息，仅仅是用来方便程序更快速地调用数据库而已。
* 作用与“DB_engine.h”里的 LIST_CONFIG 类似
* 已弃用
*/
struct sf_DB
{
	std::string name;
	std::string uid;
	int index;
};

/*
* 程序总设置，总配置
* 
* 用英文表示会更准确：General Settings。
* 主要目的就是存储程序的各种设置
* 目前还没有使用上。
*/
struct sf_config
{
	int _port;
};

/*
* 管理员设置
* 
* 也是 General Settings 的一部分，可以设置多个总管理员，允许不加限制地访问每个数据库。
* 目前还没有用上。
*/
struct sf_admin
{
	std::string _name;
	std::string _psw;
};

class network_server;

/*
* 数据库服务器
* 
* 主要的类。它相当于管理所有数据库的小工具。
* 它的运行机制和“DB_engine”类似：
* 先读取配置文件，获取整体的信息（如：有哪些数据库……）
* 任何再由用户选择要使用的数据库（use_db）,这时候才会对_use_db进行new操作，也就是读取数据库信息了。
* 接着就是对数据库的相关操作了，其实可以再写一个operate（）函数返回_use_db。
* 但是这样就代码写起来就不好看了，例如：operate().operate().add_row(...);（其实应该也没什么关系）
*/
class DB_server
{
private:
	//DB_engine* _use_db;
	std::map<std::thread::id, DB_engine*> _db_map;
	std::vector<DB_CONFIG> _sf_db;

	std::string _sf_path;
	
public:
	DB_server(std::string path);
	virtual ~DB_server();
	//加载软件配置文件
	void load_config(std::string path = "default");
	bool save_config(std::string path = "default");

public:
	std::vector<DB_CONFIG>& ls_DB() { return _sf_db; }
	//使用数据库
	DB_engine* use_db(std::string name);
	DB_engine* use_db(int sid);
	bool close_DB();
	bool is_use();
	DB_engine* DB_op();
	
	bool new_db(DB_CONFIG& new_db_cf);
	bool del_db(int uid);

private:
	bool is_args_right(std::string db_name, std::string ls_name);

public:
	// query 相关的指令

	std::vector<std::string> query_srow(std::string db_name, std::string ls_name, int row);
	std::vector<std::string> query_srow(DB_engine& db, int row);
	std::vector<std::string> query_scol(std::string db_name, std::string ls_name, int col);
	std::vector<std::string> query_scol(DB_engine& db, int col);
	std::vector<std::vector<std::string>> query_rows(std::string db_name, std::string ls_name, std::vector<int>& row);
	std::vector<std::vector<std::string>> query_rows(std::string db_name, std::string ls_name);
	std::vector<std::vector<std::string>> query_cols(std::string db_name, std::string ls_name, std::vector<int>& col);
	std::vector<std::vector<std::string>> query_Q(std::string db_name, std::string ls_name, std::string head, std::string target);
	std::vector<int> query_w(std::string db_name, std::string ls_name, std::string head, std::string target);
	std::vector<std::string> query_head(std::string db_name, std::string ls_name);
public:
	//指令处理
	std::string cmd_handle(tcp::socket& g_socket, std::string cmd);
	//指令处理（本地）
	bool cmd_handle_host(std::string cmd);
	//启动网络服务
	bool start_network_server(unsigned int port);
	//停止网络服务
	bool stop_network_server();

private:
	/*
	* 这个是网络运行相关的类
	* 它们负责发送和接收来自网络的数据，
	* 一般情况下会接受到一个特定的std::string字符串，也就是“指令”了。
	* 对于指令的处理，使用上面的cmd_handle(std::string cmd);来进行。
	* 也就是这个类接收到一个字符串后，会把字符串传入cmd_handle里，具体操作有handle来进行。
	*/
	class network_server
	{
	private:
		boost::asio::io_context io_context;
		int _port;
		DB_server* _parent;
		//连接的客户端
		std::vector<std::thread> _client_threads;
		tcp::acceptor* _acceptor;
		// 处理客户端连接
		void handle_client(tcp::socket socket);
		//标识
		bool _is_stop;
		void cleanup_threads();
	public:
		virtual ~network_server();
		network_server(int port, DB_server* parent);
		void start();
		void stop();
		bool is_run() { return !_is_stop; }
	};
	network_server* _server;
};

