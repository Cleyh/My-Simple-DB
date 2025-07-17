#include "Cmd_Query.h"
using namespace std;

/*
// vector转json
json vector_to_json(const vector<vector<string>>& data)
{
	// {
	//   "type":"data",
	//   "data"[
	//       ["xxx","yyy","zzz"],
	//       ["data","data","data"],
	//       ["data","data","data"]
	//   ]
	// }
	//
	json j;
	j["type"] = "data";
	j["data"] = json::array();
	for (const auto& row : data)
	{
		j["data"].push_back(row);
	}
	return j;
}*/

/*
// 生成错误信息的 json 对象
json error_to_json(const string& type, const string& content, const string& code)
{
	json j;
	j["type"] = type;
	j["data"] = { { "code", code }, { content } };
	return j;
}*/

void Cmd_Query::execute(DB_server& _db, std::vector<std::string>& _tokens)
{
	string _return;
	/*
	* query -get [DB] [LS] -ALL查询某库里的某表所有数据
	* query -info 获取所有数据库信息，数据库信息包括 数据库名称、所含列表
	*/
	
	if (_tokens.size() == 2 && _tokens[1] == "-info") {
		json j;
		j["db"] = json::array();
		for (auto& db : _db.ls_DB()) {
			json db_json;
			db_json["name"] = db.get_name();
			db_json["list"] = json::array();
			for (auto& ls : db._list_) {
				json ls_json;
				ls_json["name"] = ls.get_name();
				db_json["list"].push_back(ls_json);
			}
			j["db"].push_back(db_json);
		}
		_return = j.dump(-1);
	}
	else if (_tokens.size() == 5 && _tokens[4] == "-ALL"){
		auto _query = _db.query_rows(_tokens[2], _tokens[3]);
		auto _head = _db.query_head(_tokens[2], _tokens[3]);

		_query.insert(_query.begin(), _head);

		json j = vector_to_json(_query, "data");
		_return = j.dump(-1);
	}
	
	else
	{
		string _db_name = _tokens[2];
		string _ls_name = _tokens[3];
		string _mode = _tokens[1];
		string _op = _tokens[4];
		if (_mode == "-get")
		{
			/*
			* query -get [DB][LS] -R [ROW] 查某行
			* query -get [DB][LS] -Rt [ROW] [ROW]查某行到某行
			* query -get [DB][LS] -R [ROW] [ROW] [ROW] ... 查某行和某行和某行和...
			*/
			if (_op == "-R" || _op == "-Rt" || _op == "Rh")
			{
				if (_tokens.size() == 6)  // 查某行
				{
					int row;
					try
					{
						row = stoi(_tokens[5]);
					}
					catch (const invalid_argument&)
					{
						json j = hint_to_json("error", "参数错误", "1");
						_return = j.dump(-1);
						return;
					}

					vector<string> result = _db.query_srow(_db_name, _ls_name, row);
					vector<string>head = _db.query_head(_db_name, _ls_name);
					json j = vector_to_json({ head,result }, "data");
					_return = j.dump(-1);
				}
				else if (_tokens.size() == 7 && _tokens[5] == "-Rt")  // 查某行到某行
				{
					int start_row, end_row;
					try
					{
						start_row = stoi(_tokens[5]);
						end_row = stoi(_tokens[6]);
					}
					catch (const invalid_argument&)
					{
						json j = hint_to_json("error", "参数错误", "1");
						_return = j.dump(-1);
						return;
					}

					vector<int> rows;
					for (int i = start_row; i <= end_row; i++) rows.push_back(i);
					vector<vector<string>> result = _db.query_rows(_db_name, _ls_name, rows);
					vector<string>head = _db.query_head(_db_name, _ls_name);

					result.insert(result.begin(), head);

					json j = vector_to_json(result, "data");
					_return = j.dump(-1);
				}
				else  // 查某几行
				{
					vector<int> rows;
					for (size_t i = 5; i < _tokens.size(); i++)
					{
						int row;
						try
						{
							row = stoi(_tokens[i]);
						}
						catch (const invalid_argument&)
						{
							json j = hint_to_json("error", "参数错误", "1");
							_return = j.dump(-1);
							return;
						}
						rows.push_back(row);
					}
					vector<vector<string>> result = _db.query_rows(_db_name, _ls_name, rows);
					vector<string>head = _db.query_head(_db_name, _ls_name);

					result.insert(result.begin(), head);

					json j = vector_to_json(result, "data");
					_return = j.dump(-1);
				}
			}
			/*
			* query -get [DB] [LS] -C [COW] 查某列
			* query -get [DB] [LS] -Ct [COW] [COW] 查某列到某列
			* query -get [DB] [LS] -C [COW] [COW] [COW] ... 查某几列（同查行操作）
			*/
			else if (_op == "-C")
			{
				if (_tokens.size() == 6)  // 查某列
				{
					int col;
					try
					{
						col = stoi(_tokens[5]);
					}
					catch (const invalid_argument&)
					{
						json j = hint_to_json("error", "参数错误", "1");
						_return = j.dump(-1);
						return;
					}
					vector<vector<string>> result;
					vector<string> query = _db.query_scol(_db_name, _ls_name, col);
					for (int i = 0; i < query.size(); i++) {
						result.push_back(vector<string>{ query[i] });
					}
					string head = _db.query_head(_db_name, _ls_name)[col];//取头
					result.insert(result.begin(), vector<string>{ head });

					json j = vector_to_json({ result }, "data");
					_return = j.dump(-1);
				}
				else if (_tokens.size() == 7 && _tokens[5] == "-Ct")  // 查某列到某列
				{
					int start_col, end_col;
					try
					{
						start_col = stoi(_tokens[5]);
						end_col = stoi(_tokens[6]);
					}
					catch (const invalid_argument&)
					{
						json j = hint_to_json("error", "参数错误", "1");
						_return = j.dump(-1);
						return;
					}

					vector<int> cols;
					vector<string> head;
					for (int i = start_col; i <= end_col; i++) {
						cols.push_back(i);
						head.push_back(_db.query_head(_db_name, _ls_name)[i]);
					}
					vector<vector<string>> result = _db.query_cols(_db_name, _ls_name, cols);
					
					result.insert(result.begin(), head);
					
					json j = vector_to_json(result, "data");
					_return = j.dump(-1);
				}
				else  // 查某几列
				{
					vector<int> cols;
					vector<string> head;
					for (size_t i = 5; i < _tokens.size(); i++)
					{
						int col;
						try
						{
							col = stoi(_tokens[i]);
						}
						catch (const invalid_argument&)
						{
							json j = hint_to_json("error", "参数错误", "1");
							_return = j.dump(-1);
							return;
						}
						cols.push_back(col);
						head.push_back(_db.query_head(_db_name, _ls_name)[i]);
					}
					vector<vector<string>> result = _db.query_cols(_db_name, _ls_name, cols);
					result.insert(result.begin(), head);

					json j = vector_to_json(result, "data");
					_return = j.dump(-1);
				}
			}
			else if (_op == "-Q")
			{
				string head = _tokens[5];
				string target = _tokens[6];
				vector<vector<string>> result = _db.query_Q(_db_name, _ls_name, head, target);
				vector<string> _head = _db.query_head(_db_name, _ls_name);
				result.insert(result.begin(), _head);
				json j = vector_to_json(result, "data");
				_return = j.dump(-1);
			}
		}
		/*
		* query -get [DB] [LS] -Q [head] [string] 查找某列表里，在 head 列下，数据为 string 的所有行的数据
		* query -where [DB] [LS] -Q [head] [string] 查找某列表里，在 head 列下，数据为 string 的所有行的行号
		*/
		else if (_mode == "-where")
		{
		if (_tokens.size() >= 7 && _op == "-Q")
			{
				string head = _tokens[5];
				string target = _tokens[6];
				vector<int> result = _db.query_w(_db_name, _ls_name, head, target);
				
				std::vector<std::string> str_vec;
				for (int value : result) {
					str_vec.push_back(std::to_string(value));
				}

				json j = vector_to_json({ str_vec }, "data");
				_return = j.dump(-1);
			}
		}
	}
	cout << _return << endl;
	if (is_set_socket()) {
		return_data(_return);
	}
}