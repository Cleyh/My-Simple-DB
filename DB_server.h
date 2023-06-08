#pragma once

#include "DB_engine.h"
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

/*
* ���ݿ�������Ϣ
* 
* ����һ�����ݿ�Ļ�����Ϣ�����������������������ٵص������ݿ���ѡ�
* �����롰DB_engine.h����� LIST_CONFIG ����
*/
struct sf_DB
{
	string name;
	string uid;
	int index;
};

/*
* ���������ã�������
* 
* ��Ӣ�ı�ʾ���׼ȷ��General Settings��
* ��ҪĿ�ľ��Ǵ洢����ĸ������ã���������ò��ûʲô��Ҫ�ĵط���
*/
struct sf_config
{
	int _port;
};

/*
* ����Ա����
* 
* Ҳ�� General Settings ��һ���֣��������ö���ܹ���Ա�����������Ƶط���ÿ�����ݿ⡣
* Ŀǰ��û����Ҫ��
*/
struct sf_admin
{
	string _name;
	string _psw;
};

class network_server;

/*
* ���ݿ������
* 
* ��Ҫ���ࡣ���൱�ڹ����������ݿ��С���ߡ�
* �������л��ƺ͡�DB_engine�����ƣ�
* �ȶ�ȡ�����ļ�����ȡ�������Ϣ���磺����Щ���ݿ⡭����
* �κ������û�ѡ��Ҫʹ�õ����ݿ⣨use_db��,��ʱ��Ż��_use_db����new������Ҳ���Ƕ�ȡ���ݿ���Ϣ�ˡ�
* ���ž��Ƕ����ݿ����ز����ˣ���ʵ������дһ��operate������������_use_db��
* ���������ʹ���д�����Ͳ��ÿ��ˣ����磺operate().operate().add_row(...);����ʵҲûʲô��ϵ����
* ������Ҫ�ٲ�����صĴ����װһ���¡�
*/
class DB_server
{
private:
	//DB_engine* _use_db;
	map<thread::id, DB_engine*> _db_map;
	vector<DB_CONFIG> _sf_db;

	string _sf_path;
	
public:
	DB_server(string path);
	virtual ~DB_server();
	//������������ļ�
	void load_config(string path = "default");
	bool save_config(string path = "default");

public:
	vector<DB_CONFIG>& ls_DB() { return _sf_db; }
	//ʹ�����ݿ�
	DB_engine* use_db(string name);
	DB_engine* use_db(int sid);
	bool close_DB();
	bool is_use();
	DB_engine* DB_op();
	
	bool new_db(DB_CONFIG& new_db_cf);
	bool del_db(int uid);

public:
	//query���
	vector<string> query_srow(string db_name, string ls_name, int row);
	vector<string> query_srow(DB_engine& db, int row);
	vector<string> query_scol(string db_name, string ls_name, int col);
	vector<string> query_scol(DB_engine& db, int col);
	vector<vector<string>> query_rows(string db_name, string ls_name, vector<int>& row);
	vector<vector<string>> query_rows(string db_name, string ls_name);
	vector<vector<string>> query_cols(string db_name, string ls_name, vector<int>& col);
	vector<vector<string>> query_Q(string db_name, string ls_name, string head, string target);
	vector<int> query_w(string db_name, string ls_name, string head, string target);
	vector<string> query_head(string db_name, string ls_name);
public:
	//ָ���
	string cmd_handle(tcp::socket& g_socket, string cmd);
	//ָ������أ�
	bool cmd_handle_host(string cmd);
	//�����������
	bool start_network_server(unsigned int port);
	//ֹͣ�������
	bool stop_network_server();

private:
	/*
	* ���������������ص���
	* ���Ǹ����ͺͽ���������������ݣ�
	* һ������»���ܵ�һ���ض���string�ַ�����Ҳ���ǡ�ָ��ˡ�
	* ����ָ��Ĵ���ʹ�������cmd_handle(string cmd);�����С�
	* Ҳ�����������յ�һ���ַ����󣬻���ַ�������cmd_handle����������handle�����С�
	*/
	class network_server
	{
	private:
		boost::asio::io_context io_context;
		int _port;
		DB_server* _parent;
		//���ӵĿͻ���
		vector<thread> _client_threads;
		tcp::acceptor* _acceptor;
		// ����ͻ�������
		void handle_client(tcp::socket socket);
		//��ʶ
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

