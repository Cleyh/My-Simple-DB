/*
* ��ҪC++17֧��
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
* �б�����
*
* ����洢����һ���б�Ļ�����Ϣ�����߱����ݱ��Ĺ��ܡ�
* ���ݿ�����ʱ���ȶ�ȡ������Ϣ��������ֱ�Ӷ�ȡ������ݡ�������Լ�˶�ȡʱ����ڴ档
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
* �б���
* 
* �൱��һ�ű�񣬴洢���������
*/
class DB_LIST
	:public LIST_CONFIG
{
public:
	int _column;
	int _row;
	//��ͷ����¼ÿһ�е�����
	vector<string> _head;
	//��ͷ���ͣ���¼ÿһ��Ӧ���洢ʲô���͵����ݡ�PS��Ŀǰû���κ��жϻ��ƣ�Ҳ����˵����ַ���д��INT����ȥ�����������⡣
	vector<DB::DataType> _type;
	//����������
	vector<vector<string>> _data;
	//�����洢��Ӳ�̵�λ����Ϣ
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
* ���ݿ�����
* 
* ����������ݿ��������Ϣ�������������Ϣ�������൱�ڡ������ļ�����
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
* ���ݿ�����
* 
* ���԰������Ϊһ�����ڹ���Ͳ����б��С���ߡ�
* ����Ҫ�ȶ�ȡ������Ϣ����ȡ��ǰ���ݿ�ӵ�е������б�
* Ȼ����Ҫѡ���б���ʱֻ�н�����ѡ�����������Ż��ȡ�������ݡ�
* �����Ҫ���ݱ����ʹ��operate���������᷵��_sel_List������ʵ�ǲ������ظ�д���ĸ��ֲ����ˣ�
*/
class DB_engine
{
public:
	//���죬ʹ�ô洢λ��
	DB_engine(string DB_path);
	//���죬�½���
	DB_engine(DB_CONFIG& db_cf, string DB_path);
	//�������ͷ�ѡ��ı�
	virtual ~DB_engine();
	//���뵱ǰ���ݿ����ã�Ĭ��ʹ�ù���λ��
	bool load_DB(string DB_path = "default");
	//�رգ�ж�����ã����ݿ�
	//bool close_DB(bool save = true);
	//�������ݿ�����
	bool save_DB(string DB_path = "default");
	//����������
	bool create_cf(string DB_path = "default");

public:
	//��ӱ�����
	void add_List(DB_LIST& ad_list);
	//ɾ������ţ�
	bool del_List(int uid);
	bool del_List(string del_name);

public:
	vector<LIST_CONFIG>& ls_ls() { return _List; }
	//ѡ������ı�
	DB_LIST* select_list(int uid);
	DB_LIST* select_list(string name);
	//ж�ر�Ĭ���Զ�����
	bool close_list(bool save = true);
	//��list���в��������÷�ʽ����operate().save();
	DB_LIST* LS_op();
	bool is_sel();

public:
	string get_name() { return _db_config.get_name(); }

private:
	//��¼���ݿ�ӵ�е�List
	vector<LIST_CONFIG> _List;
	//ѡ�в�����List
	DB_LIST* _sel_List;
	map<thread::id, DB_LIST*> _ls_map;
	//���ݿ�����
	DB_CONFIG _db_config;
	//���ݿ�洢��ַ
	string _db_path;
};

