/*
* 需要C++17支持
*/
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <json.hpp>
#include <boost/filesystem.hpp>
using namespace std;
using json = nlohmann::json;

namespace DB {
	enum DataType
	{
		INT = 0,
		CHAR = 1,
		STRING = 2,
		FLOAT = 3,
		BOOL = 4
	};
}

/*
* 列表配置
*
* 这里存储的是一个列表的基本信息，不具备操纵表格的功能。
* 数据库运行时，先读取配置信息，而不用直接读取表格数据。这样节约了读取时间和内存。
*/
struct LIST_CONFIG
{
private:
	string _name;
public:
	virtual ~LIST_CONFIG() { _name = ""; }
	LIST_CONFIG(string name);
	void set_name(string name) { _name = name; }
	string get_name() { return _name; }
};

/*
* 列表类
* 
* 相当于一张表格，存储具体的数据
*/
class DB_LIST
	:public LIST_CONFIG
{
public:
	int _column;
	int _row;
	//表头，记录每一列的名字
	vector<string> _head;
	//表头类型，记录每一列应当存储什么类型的数据。PS：目前没有任何判断机制，也就是说你把字符串写到INT里面去都不会有问题。
	vector<DB::DataType> _type;
	//表格具体数据
	vector<vector<string>> _data;
	//这个表存储在硬盘的位置信息
	string _file_path;

public:
	virtual ~DB_LIST() {};
	DB_LIST(LIST_CONFIG ls_cf);
	DB_LIST(string file_path);
	bool load(string file_path);
	bool save(string file_path = "default");
	LIST_CONFIG& get_cf() { return *this; }

public:
	void set_head(const vector<string>& head);
	void add_row(const vector<string>& row);
	void add_col(const string& column_name, const vector<string>& column_data);
	
	bool ins_row(int row_index, const vector<string>& row);
	bool ins_col(int column_index, const string& column_name, const vector<string>& column_data);

	bool del_row(int row_index);
	bool del_col(int column_index);
public:
	bool set_row(int row_index, const vector<string>& row);
	bool set_colc(int column_index, const vector<string>& column_data, int start_row, int end_row);
	bool set_col(int column_index, const vector<string>& column_data, int start_row);
	bool set(int row_index, int column_index, const string& value);
};

/*
* 数据库配置
* 
* 这里的是数据库的配置信息，包含其基本信息，可以相当于“自述文件”。
*/
struct DB_CONFIG
{
private:
	string _db_name;
	int _uid;
	string _psw;
	int _list_number;
public:
	DB_CONFIG(string db_name);
	vector<LIST_CONFIG> _list_;
	void set_name(string db_name);
	void set_uid(int uid) { _uid = uid; }
	string get_name() { return _db_name; }
};

/*
* 数据库引擎
* 
* 可以把他理解为一个用于管理和操纵列表的小工具。
* 它需要先读取配置信息，获取当前数据库拥有的所有列表。
* 然后需要选择列表，此时只有进行了选择操作，程序才会读取表格的数据。
* 如果需要操纵表格，则使用operate函数，它会返回_sel_List。（其实是不想再重复写表格的各种操纵了）
*/
class DB_engine
{
public:
	//构造，使用存储位置
	DB_engine(string DB_path);
	//构造，新建库
	DB_engine(DB_CONFIG& db_cf, string DB_path);
	//析构，释放选择的表
	virtual ~DB_engine();
	//载入当前数据库配置，默认使用构造位置
	bool load_DB(string DB_path = "default");
	//关闭（卸载配置）数据库
	//bool close_DB(bool save = true);
	//保存数据库配置
	bool save_DB(string DB_path = "default");
	//创建新配置
	bool create_cf(string DB_path = "default");

public:
	//添加表及内容
	void add_List(DB_LIST& ad_list);
	//删除表（序号）
	bool del_List(int uid);
	bool del_List(string del_name);

public:
	vector<LIST_CONFIG>& ls_ls() { return _List; }
	//选择操作的表
	DB_LIST* select_list(int uid);
	DB_LIST* select_list(string name);
	//卸载表，默认自动保存
	bool close_list(bool save = true);
	//对list进行操作，调用方式例：operate().save();
	DB_LIST* LS_op();
	bool is_sel();

public:
	string get_name() { return _db_config.get_name(); }

private:
	//记录数据库拥有的List
	vector<LIST_CONFIG> _List;
	//选中操作的List
	DB_LIST* _sel_List;
	map<thread::id, DB_LIST*> _ls_map;
	//数据库属性
	DB_CONFIG _db_config;
	//数据库存储地址
	string _db_path;
};

