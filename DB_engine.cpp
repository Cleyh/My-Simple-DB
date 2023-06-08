#include "DB_engine.h"

/*
* DB_LIST的实现
*/

vector<string> split(const string& str, char delimiter) {
	vector<string> result;
	stringstream ss(str);
	string token;
	while (getline(ss, token, delimiter)) {
		result.push_back(token);
	}
	return result;
}

string join(const vector<string>& vec, char delimiter) {
	string result;
	for (const auto& str : vec) {
		if (!result.empty()) result += delimiter;
		result += str;
	}
	return result;
}


DB_LIST::DB_LIST(LIST_CONFIG ls_cf)
	:LIST_CONFIG(ls_cf)
{
}

DB_LIST::DB_LIST(string file_path)
	:LIST_CONFIG("")
{
	_file_path = file_path;
	if (!load(file_path)) {
		set_name("load_failed");
	}
}

bool DB_LIST::load(string file_path)
{
	ifstream file(file_path);
	if (!file.is_open()) return false;

	_file_path = file_path;

	json j;
	file >> j;

	_data.clear();
	_head.clear();

	set_name(j["config"]["name"]);
	_head = split(j["config"]["head"], ',');

	vector<pair<int, vector<string>>> data_with_uid;
	for (const auto& row : j["data"]) {
		vector<string> row_data;
		for (const auto& head : _head) {
			row_data.push_back(row[head]);
		}
		int uid = stoi(row["_uid_"].get<string>());
		data_with_uid.push_back(make_pair(uid, row_data));
	}

	sort(data_with_uid.begin(), data_with_uid.end());

	for (const auto& row : data_with_uid) {
		_data.push_back(row.second);
	}

	file.close();
	return true;
}

bool DB_LIST::save(string file_path)
{
	if (file_path == "default") file_path = _file_path;
	ofstream file(file_path);
	if (!file.is_open()) return false;

	json j;
	j["config"]["name"] = get_name();
	j["config"]["head"] = join(_head, ',');
	int uid = 0;
	for (const auto& row : _data) {
		json row_json;
		for (size_t i = 0; i < _head.size(); i++) {
			row_json[_head[i]] = row[i];
		}
		row_json["_uid_"] = to_string(uid++);
		j["data"].push_back(row_json);
	}

	file << setw(4) << j << endl;
	file.close();
	return true;
}

void DB_LIST::set_head(const vector<string>& head)
{
	_head = head;
}

void DB_LIST::add_row(const vector<string>& row)
{
	if (row.size() != _head.size()) return;
	_data.push_back(row);
}

void DB_LIST::add_col(const string& column_name, const vector<string>& column_data)
{
	_head.push_back(column_name);
	for (size_t i = 0; i < _data.size(); i++) {
		if (i < column_data.size()) {
			_data[i].push_back(column_data[i]);
		}
		else {
			_data[i].push_back("");
		}
	}
}

bool DB_LIST::ins_row(int row_index, const vector<string>& row)
{
	if (row_index < 0 || row_index > _data.size() || row.size() != _head.size()) return false;
	_data.insert(_data.begin() + row_index, row);
	return true;
}

bool DB_LIST::ins_col(int column_index, const string& column_name, const vector<string>& column_data)
{
	if (column_index < 0 || column_index > _head.size()) return false;
	_head.insert(_head.begin() + column_index, column_name);
	for (size_t i = 0; i < _data.size(); i++) {
		if (i < column_data.size()) {
			_data[i].insert(_data[i].begin() + column_index, column_data[i]);
		}
		else {
			_data[i].insert(_data[i].begin() + column_index, "");
		}
	}
	return true;
}

bool DB_LIST::del_row(int row_index)
{
	if (row_index < 0 || row_index >= _data.size()) return false;
	_data.erase(_data.begin() + row_index);
	return true;
}

bool DB_LIST::del_col(int column_index)
{
	if (column_index < 0 || column_index >= _head.size()) return false;
	_head.erase(_head.begin() + column_index);
	for (auto& row : _data) {
		row.erase(row.begin() + column_index);
	}
	return true;
}

bool DB_LIST::set_row(int row_index, const vector<string>& row) {
	if (row_index < 0 || row_index >= _data.size() || row.size() > _head.size()) return false;
	_data[row_index] = row;
	return true;
}

bool DB_LIST::set_colc(int column_index, const vector<string>& column_data, int start_row, int end_row) {
	if (column_index < 0 || column_index >= _head.size()) return false;
	if (start_row < 0) start_row = 0;
	if (end_row < 0 || end_row >= _data.size()) end_row = _data.size() - 1;
	for (int i = start_row; i <= end_row; i++) {
		if (i - start_row < column_data.size()) {
			_data[i][column_index] = column_data[i - start_row];
		}
		else {
			_data[i][column_index] = "";
		}
	}
	return true;
}

bool DB_LIST::set_col(int column_index, const vector<string>& column_data, int start_row) {
	if (column_index < 0 || column_index >= _head.size() || start_row < 0 || start_row >= _data.size()) return false;
	for (size_t i = 0; i < column_data.size() && start_row + i < _data.size(); i++) {
		_data[start_row + i][column_index] = column_data[i];
	}
	return true;
}

bool DB_LIST::set(int row_index, int column_index, const string& value) {
	if (row_index < 0 || row_index >= _data.size() || column_index < 0 || column_index >= _head.size()) return false;
	_data[row_index][column_index] = value;
	return true;
}

/*
* LIST_CONFIG的实现
*/
LIST_CONFIG::LIST_CONFIG(string name)
{
	replace(_name.begin(), _name.end(), ' ', '_');
	_name = name;
}

/*
* DB_CONFIG的实现
*/

DB_CONFIG::DB_CONFIG(string db_name)
{
	replace(db_name.begin(), db_name.end(), ' ', '_');
	_db_name = db_name;
	_uid = 0;
	_psw = "";
	_list_number = 0;
}

void DB_CONFIG::set_name(string db_name)
{
	replace(db_name.begin(), db_name.end(), ' ', '_');
	_db_name = db_name;
}

/*
* DB_engine的实现
*/

DB_engine::DB_engine(string DB_path)
	:_db_path(DB_path), _db_config("")
{
	if (DB_path != "default") _db_path = DB_path;
	load_DB();
}

DB_engine::DB_engine(DB_CONFIG& db_cf, string DB_path)
	:_db_path(DB_path), _db_config(db_cf)
{
}

DB_engine::~DB_engine()
{
	for (auto& item : _ls_map)
	{
		delete item.second;
	}
}

bool DB_engine::load_DB(string DB_path)
{
	string config_path = _db_path + "/db_config.json";
	
	ifstream file(config_path);
	if (!file.is_open()) {
		cerr << "Error opening DB" << endl;
		return false;
	}
	
	json j;
	file >> j;

	//读取数据库配置
	_db_config = DB_CONFIG(j["config"]["db_name"]);

	//读取列表
	for (const auto& item : j["list"])
	{
		LIST_CONFIG list_config(item["name"]);
		_List.push_back(list_config);
		_db_config._list_.push_back(list_config);
	}

	file.close();

	return true;
}

/*
bool DB_engine::close_DB(bool save)
{
	if (save) save_DB();

	_db_path = "";
	_List.clear();
	_db_config = DB_CONFIG("");
	if (_sel_List) {
		close_list();
		delete[] _sel_List;
	}
	return true;
}
*/

bool DB_engine::save_DB(string DB_path)
{
	if (DB_path == "default") DB_path = _db_path;

	string config_path = DB_path + "/db_config.json";

	ofstream file(config_path);
	if (!file.is_open()) {
		cerr << "Saving DB config failed, error opening DB!" << endl;
		return false;
	}

	json j;
	j["config"]["db_name"] = _db_config.get_name();
	
	if (_List.size() == 0) {
		//j["list"] = json::array();
	}
	else
	{
		for (auto& list : _List) {
			json j_list;
			j_list["name"] = list.get_name();
			j["list"].push_back(j_list);
		}
	}
	
	file << setw(4) << j << endl;
	file.close();
	return true;
}

//暂时没用上，实现内容与save_DB一样
bool DB_engine::create_cf(string DB_path)
{
	if (DB_path == "default") DB_path = _db_path;

	string config_path = DB_path + "/db_config.json";
	ofstream file(DB_path);
	if (!file.is_open()) {
		cerr << "Error opening DB" << endl;
		return false;
	}

	json j;
	j["config"]["db_name"] = _db_config.get_name();

	for (auto& list : _List) {
		json j_list;
		j_list["name"] = list.get_name();
		j["list"].push_back(j_list);
	}

	file << setw(4) << j << endl;
	file.close();

	return true;
}

void DB_engine::add_List(DB_LIST& ad_list)
{
	//LIST_CONFIG new_list_config = ad_list_config;
	//LIST_CONFIG new_list_config = ad_list.get_cf();
	int new_uid = _List.size();

	//检查是否重复
	auto it = std::find_if(_List.begin(), _List.end(), [&](LIST_CONFIG& lc) {
		return lc.get_name() == ad_list.get_name();
		});
	if (it != _List.end()) {
		//比如说，已经存在表“test”，此时就会将其改变为“test_2”
		ad_list.set_name(ad_list.get_name() + "_" + to_string(new_uid));
	}

	_List.push_back(ad_list);

	string list_path = _db_path + "/" + _List[new_uid].get_name() + ".txt";

	ad_list.save(list_path);

	cout << "The list saved as [" << ad_list.get_name() << "]." << endl;
}

bool DB_engine::del_List(int uid)
{
	if (uid >= _List.size() || uid < 0) {
		cerr << "You are deleting an inxistent list" << endl;
		return false;
	}

	//检查删除的表是否正在使用
	std::string ls_name = _List[uid].get_name();
	for (const auto& entry : _ls_map)
	{
		if (entry.second->get_name() == ls_name)
		{
			cerr << "Cannot delete database because it is currently in use" << endl;
			return false;
		}
	}

	//删除文件
	string list_path = _db_path + "/" + _List[uid].get_name() + ".txt";
	if (remove(list_path.c_str()) != 0)
	{
		//cerr << "Delete failed!" << endl;
		throw string("Delete_Failed");
	}

	//擦除一个list config
	_List.erase(_List.begin() + uid);

	return true;
}

bool DB_engine::del_List(string del_name)
{
	if (del_name == "") return false;

	auto it = std::find_if(_List.begin(), _List.end(),
		[&](LIST_CONFIG& item) { return item.get_name() == del_name; });
	if (it == _List.end()) {
		return false;
	}

	//检查删除的表是否正在使用
	for (const auto& entry : _ls_map)
	{
		if (entry.second->get_name() == del_name)
		{
			cerr << "Cannot delete database because it is currently in use" << endl;
			return false;
		}
	}

	string list_path = _db_path + "/" + it->get_name() + ".txt";
	if (remove(list_path.c_str()) != 0) 
	{
		cerr << "Delete failed!" << endl;
		throw string("Delete_Failed");
	}

	//擦除一个list config
	_List.erase(it);

	return true;
}

DB_LIST* DB_engine::select_list(int uid)
{
	if (uid >= _List.size() || uid < 0) {
		cerr << "You are selecting an inxistent list" << endl;
		return nullptr;
	}

	thread::id this_id = std::this_thread::get_id();
	if (_ls_map.find(this_id) == _ls_map.end())
	{
		// 如果当前线程的 DB_LIST 实例不存在，则创建一个新实例
		_ls_map[this_id] = new DB_LIST(_List[uid]);
	}
	else
	{
		// 如果当前线程的 DB_LIST 实例已存在，则先关闭它，然后创建一个新实例
		close_list();
		_ls_map[this_id] = new DB_LIST(_List[uid]);
	}

	string sel_path = _db_path + "/" + _List[uid].get_name() + ".txt";
	_ls_map[this_id]->load(sel_path);
	
	return _ls_map[this_id];
}

DB_LIST* DB_engine::select_list(string name)
{
	auto it = std::find_if(_List.begin(), _List.end(),
		[&](LIST_CONFIG& item) { return item.get_name() == name; });
	if (it == _List.end()) {
		cerr << "You are selecting an inxistent list" << endl;
		return nullptr;
	}

	thread::id this_id = std::this_thread::get_id();
	if (_ls_map.find(this_id) == _ls_map.end())
	{
		// 如果当前线程的 DB_LIST 实例不存在，则创建一个新实例
		_ls_map[this_id] = new DB_LIST(*it);
	}
	else
	{
		// 如果当前线程的 DB_LIST 实例已存在，则先关闭它，然后创建一个新实例
		close_list();
		_ls_map[this_id] = new DB_LIST(*it);
	}

	string sel_path = _db_path + "/" + it->get_name() + ".txt";
	_ls_map[this_id]->load(sel_path);
	
	return _ls_map[this_id];
}

bool DB_engine::close_list(bool save)
{
	thread::id this_id = std::this_thread::get_id();
	if (_ls_map.find(this_id) == _ls_map.end()) return false;

	DB_LIST* ls_instance = _ls_map[this_id];

	if (save) ls_instance->save();

	delete ls_instance;
	_ls_map.erase(this_id);

	return true;
}

DB_LIST* DB_engine::LS_op()
{
	std::thread::id this_id = std::this_thread::get_id();
	auto it = _ls_map.find(this_id);
	if (it == _ls_map.end()) return nullptr;
	else return _ls_map[this_id];
}

bool DB_engine::is_sel()
{
	std::thread::id this_id = std::this_thread::get_id();
	auto it = _ls_map.find(this_id);
	if (it == _ls_map.end()) return false;
	else return true;
}

