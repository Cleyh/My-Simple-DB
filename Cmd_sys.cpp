#include "Cmd_sys.h"

unordered_map<string, SYS_COMMAND> sys_cmd = {
	{"ADD", ADD}, {"DEL",DEL }, {"INS",INS},
	{"SET",SET}, {"GET",GET},
	{"SEL",SEL}, {"CLOSE",CLOSE},
	{"CREATE",CREATE}, {"REMOVE",REMOVE}
};

string join(const vector<size_t>& vec, string delimiter) {
	string result;
	for (const auto& str : vec) {
		if (!result.empty()) result += delimiter;
		result += str;
	}
	return result;
}

void Cmd_sys::show_list(vector<string> _head, vector<vector<string>> _data)
{
	vector<size_t> column_widths(_head.size());
	for (size_t i = 0; i < _head.size(); i++) {
		column_widths[i] = _head[i].size();
	}
	for (const auto& row : _data) {
		for (size_t i = 0; i < row.size(); i++) {
			column_widths[i] = max(column_widths[i], row[i].size());
		}
	}

	const string separator = "+-" + join(column_widths, "-+-") + "-+";
	cout << separator << endl;

	cout << "| ";
	for (size_t i = 0; i < _head.size(); i++) {
		cout << setw(column_widths[i]) << _head[i] << " | ";
	}
	cout << endl;

	cout << separator << endl;

	for (const auto& row : _data) {
		cout << "| ";
		for (size_t i = 0; i < row.size(); i++) {
			cout << setw(column_widths[i]) << row[i] << " | ";
		}
		cout << endl;
	}

	cout << separator << endl;
}

void Cmd_sys::show_list(int row_index, vector<string> _head, vector<vector<string>> _data)
{
	if (row_index < 0 || row_index >= _data.size()) return;

	vector<size_t> column_widths(_head.size());
	for (size_t i = 0; i < _head.size(); i++) {
		column_widths[i] = _head[i].size();
	}
	for (size_t i = 0; i < _data[row_index].size(); i++) {
		column_widths[i] = max(column_widths[i], _data[row_index][i].size());
	}

	const string separator = "+-" + join(column_widths, "-+-") + "-+";
	cout << separator << endl;

	cout << "| ";
	for (size_t i = 0; i < _head.size(); i++) {
		cout << setw(column_widths[i]) << _head[i] << " | ";
	}
	cout << endl;

	cout << separator << endl;

	cout << "| ";
	for (size_t i = 0; i < _data[row_index].size(); i++) {
		cout << setw(column_widths[i]) << _data[row_index][i] << " | ";
	}
	cout << endl;

	cout << separator << endl;

	return;
}

vector<string> split_args(string& str)
{
	vector<string> arges;
	stringstream ss(str);
	string token;
	while (getline(ss, token, ',')) {
		//检查能否分割
		if (token.size() < 2 || token.front() != '"' || token.back() != '"') {
			return {};
		}
		// 去掉开头和结尾的双引号
		token = token.substr(1, token.size() - 2);
		arges.push_back(token);
	}
	return arges;
}

void Cmd_sys::execute(DB_server& db, vector<string>& tokens)
{
	SYS_COMMAND op = sys_cmd.find(tokens[0])->second;
	string _return;
	switch (op)
	{
	case ADD:
		if (tokens.size() < 3) {
			cerr << "参数不完整，请检查你的参数" << endl;
			json j = hint_to_json("error", "参数不完整，请检查你的参数", "0");
			_return = j.dump(-1);
			break;
		}

		if (!db.is_use()) 
		{
			cerr << "请先选择数据库！" << endl;
			json j = hint_to_json("error", "请先选择数据库！", "0");
			_return = j.dump(-1);
			break;
		}

		//ADD -R [add_data]
		if (tokens.size() >= 3 && tokens[1] == "-R")
		{
			if (!db.DB_op()->is_sel())
			{
				cerr << "请先选择表！" << endl;
				json j = hint_to_json("error", "请先选择表！", "0");
				_return = j.dump(-1);
				break;
			}

			vector<string> arg = split_args(tokens[2]);
			if (arg.size() <= 0) {
				cerr << "指令参数错误!" << endl;
				json j = hint_to_json("error", "指令参数错误!", "1");
				_return = j.dump(-1);
				break;
			}
			db.DB_op()->LS_op()->add_row(arg);
		}

		//ADD -C [add_head] [add_data]
		else if (tokens.size() >=3  && tokens[1] == "-C")
		{
			if (!db.DB_op()->is_sel())
			{
				cerr << "请先选择表！" << endl;
				break;
			}

			vector<string> arg;
			if (tokens.size() >= 4) arg = split_args(tokens[3]);
			
			/*
			if (arg.size() <= 0) {
				cerr << "指令参数错误!" << endl;
				break;
			}
			*/

			db.DB_op()->LS_op()->add_col(tokens[2], arg);
		}
		
		//ADD [add_name] [add_head]
		else if (tokens.size() >= 3)
		{
			vector<string> arg2 = split_args(tokens[2]);
			if (arg2.size() <= 0) {
				cerr << "指令参数错误!" << endl;
				break;
			}
			LIST_CONFIG new_cf(tokens[1]);
			DB_LIST new_ls(new_cf);
			new_ls.set_head(arg2);
			db.DB_op()->add_List(new_ls);
			break;
		}
		break;

	case DEL:
		if (!db.is_use())
		{
			cerr << "请先选择数据库！" << endl;
			_return = "[error],[\"请先选择数据库！\"]";
			break;
		}

		//DEL -R [row]
		if (tokens.size() >= 3 && tokens[1] == "-R")
		{
			int row = 0;
			try {
				row = stoi(tokens[2]);
			}
			catch (const invalid_argument& e) {
				break;
			}
			if (!db.DB_op()->is_sel())
			{
				cerr << "请先选择表！" << endl;
				break;
			}
			db.DB_op()->LS_op()->del_row(row);
		}
		//DEL -C [col]
		else if (tokens.size() >= 3 && tokens[1] == "-C")
		{
			int col = 0;
			try {
				col = stoi(tokens[2]);
			}
			catch (const invalid_argument& e) {
				break;
			}
			if (!db.DB_op()->is_sel())
			{
				cerr << "请先选择表！" << endl;
				break;
			}
			db.DB_op()->LS_op()->del_col(col);
		}
		//DEL [del_name]
		else if (tokens.size() >= 2)
		{
			int id = 0;
			try {
				id = stoi(tokens[1]);
				if (db.DB_op()->del_List(id)) {
					cout << "delete success!" << endl;
					//[info],["delete success!"]
					break;
				}
			}
			//如果不是id删除则尝试名称删除
			catch (const invalid_argument& e) {
				if (db.DB_op()->del_List(tokens[1])) {
					cout << "delete success!" << endl;
					break;
				}
			}
			//如果删除失败
			catch (const string& e) {
				if (e == "Delete_Failed") cerr << e << endl;
				break;
			}
		}
		break;

	case INS:
		if (!db.is_use()) {
			cerr << "请先选择数据库！" << endl;
			break;
		}
		else if (!db.DB_op()->is_sel()) {
			cerr << "请先选择表！" << endl;
			break;
		}

		// INS -R [row] [data]
		if (tokens.size() >= 4 && tokens[1] == "-R") {
			int row_index = stoi(tokens[2]);
			vector<string> arg = split_args(tokens[3]);
			db.DB_op()->LS_op()->ins_row(row_index, arg);
		}

		// INS -C [col] [head] [data]
		else if (tokens.size() >= 4 && tokens[1] == "-C") {
			int column_index = stoi(tokens[2]);
			vector<string> arg;
			if (tokens.size() >= 5) arg = split_args(tokens[4]);
			db.DB_op()->LS_op()->ins_col(column_index, tokens[3], arg);
		}
		break;

	case SET:
		if (!db.is_use()) {
			cerr << "请先选择数据库！" << endl;
			break;
		}
		else if (!db.DB_op()->is_sel()) {
			cerr << "请先选择表！" << endl;
			break;
		}

		if (tokens.size() >= 4) {
			// SET -R [row] [data]
			if (tokens[1] == "-R") {
				int row_index = 0;
				try {
					row_index = stoi(tokens[2]);
				}
				catch (const invalid_argument& e) {
					cerr << "指令参数错误!" << endl;
					break;
				}
				vector<string> arg = split_args(tokens[3]);
				if (!db.DB_op()->LS_op()->set_row(row_index, arg)) {
					cerr << "设置行失败!" << endl;
					break;
				}
				cout << "第 " << row_index << " 行的内容已改变" << endl;
			}

			// SET -C [col] [data] [from] [to]
			else if (tokens[1] == "-C") {
				//col
				int column_index = 0;
				try {
					column_index = stoi(tokens[2]);
				}
				catch (const invalid_argument& e) {
					cerr << "指令参数错误!" << endl;
					break;
				}

				vector<string> arg = split_args(tokens[3]);
				
				//如果[data]后面还有参数
				if (tokens.size() >= 5) {
					//from
					int start_row = 0;
					try {
						start_row = stoi(tokens[4]);
					}
					catch (const invalid_argument& e) {
						//from转换不成功，则从0开始设置
						if (!db.DB_op()->LS_op()->set_col(column_index, arg, 0)) {
							cerr << "设置列失败!" << endl;
						}
						cout << "第 0 行到第 " << arg.size() << " 行的内容已改变！" << endl;
						break;
					}
					//to
					int end_row = -1;
					if (tokens.size() >= 6) {
						try {
							//from end转换成功，使用from到end的设置
							end_row = stoi(tokens[5]);
							if (!db.DB_op()->LS_op()->set_colc(column_index, arg, start_row, end_row)) {
								cerr << "设置列失败!" << endl;
								break;
							}
							cout << "第" << start_row << " 行到第 " << end_row << " 行的内容已改变！" << endl;
						}
						catch (const invalid_argument& e) {
							//end不成功，则从from开始，以内容为范围进行设置
							if (!db.DB_op()->LS_op()->set_col(column_index, arg, start_row)) {
								cerr << "设置列失败!" << endl;
								break;
							}
							cout << "第" << start_row << " 行到第 " << arg.size() << " 行的内容已改变！" << endl;
						}
					}
					else
					{
						if (!db.DB_op()->LS_op()->set_col(column_index, arg, start_row)) {
							cerr << "设置列失败!" << endl;
						}
						cout << "第" << start_row << " 行到第 " << arg.size() << " 行的内容已改变！" << endl;
						break;
					}
				}
				//[data]后面无参数
				else {
					if (!db.DB_op()->LS_op()->set_colc(column_index, arg, 0,db.DB_op()->LS_op()->_data.size())) {
						cerr << "设置列失败!" << endl;
						break;
					}
					cout << "第 " << column_index << " 列的内容已改变" << endl;
				}
			}

			// SET [row] [col] [data]
			else {
				try {
					int row_index = stoi(tokens[1]);
					int column_index = stoi(tokens[2]);
					if (!db.DB_op()->LS_op()->set(row_index, column_index, tokens[3])) {
						cerr << "设置单元格失败!" << endl;
						break;
					}
					cout << "第 " << row_index << " 行第 " << column_index << " 列的内容已改变" << endl;
				}
				catch (const invalid_argument& e) {
					cerr << "指令参数错误!" << endl;
					break;
				}
			}
		}
		break;

	case GET:
		//GET -DB
		if (tokens.size() >= 2 && tokens[1] == "-DB") {
			int i = 0;
			cout << "----====[DB LIST]====----" << endl;
			for (auto n : db.ls_DB()) {
				cout << "[" << i << "] " << n.get_name() << endl;
				i++;
			}
			cout << "----====[DB LIST]====----" << endl;
			break;
		}
		//GET -LS
		else if (tokens.size() >= 2 && tokens[1] == "-LS")
		{
			if (!db.is_use()) {
				cerr << "你没有选择任何数据库，无法查看所有列表" << endl;
				break;
			}
			int i = 0;
			cout << "----====[" << db.DB_op()->get_name() << " LIST]====----" << endl;
			for (auto n : db.DB_op()->ls_ls()) {
				cout << "[" << i << "] " << n.get_name() << endl;
				i++;
			}
			cout << "----====[" << db.DB_op()->get_name() << " LIST]====----" << endl;
			break;
		}
		
		if (!db.is_use()) {
			cerr << "未选择数据库！" << endl;
			break;
		}
		else if(!db.DB_op()->is_sel()) {
			cerr << "未选择表！" << endl;
			break;
		}
		
		//GET -R [row]
		if (tokens.size() >= 3 && tokens[1] == "-R")
		{
			if(tokens[2] == "-ALL") show_list(db.DB_op()->LS_op()->_head, db.DB_op()->LS_op()->_data);
			else {
				try {
					int row = stoi(tokens[2]);
					if (row < 0 || row >= db.DB_op()->LS_op()->_data.size()) {
						cerr << "未获取到该行，此行不存在！" << endl;
						break;
					}
					else {
						show_list(row, db.DB_op()->LS_op()->_head, db.DB_op()->LS_op()->_data);
						break;
					}
				}
				catch (const invalid_argument& e) {
					show_list(db.DB_op()->LS_op()->_head, db.DB_op()->LS_op()->_data);
				}
				break;
			}
		}
		//GET [row] [col]
		else if (tokens.size() >= 3)
		{
			break;
		}
		break;

	case SEL:
		//SEL -DB [name]
		if (tokens.size() >= 3 && tokens[1] == "-DB" ) {
			try {
				int _uid = stoi(tokens[2]);
				if (!db.use_db(_uid)) {
					cerr << "选中失败，你选择了一个不存在的数据库！" << endl;
					break;
				}
			}
			catch (const invalid_argument& e) {
				if (!db.use_db(tokens[2])) {
					cerr << "选中失败，你选择了一个不存在的数据库！" << endl;
					break;
				}
			}
			cout << "选中数据库：" << db.DB_op()->get_name() << endl;
			break;
		}
		//SEL -LS [name]
		else if (tokens.size() >= 3 && tokens[1] == "-LS") {
			if (!db.is_use()) {
				cerr << "未选择数据库，请先选择数据库！" << endl;
				break;
			}
			try {
				int id = stoi(tokens[2]);
				if (!db.DB_op()->select_list(id)) {
					cerr << "选中失败，你选择了一个不存在的表！" << endl;
					break;
				}
			}
			catch (const invalid_argument& e) {
				if (!db.DB_op()->select_list(tokens[2])) {
					cerr << "选中失败，你选择了一个不存在的表！" << endl;
					break;
				}
			}
			cout << "选中列表：" << db.DB_op()->LS_op()->get_name() << endl;
			break;
		}
		break;

	case CLOSE:
		//CLOSE -LS/-DB
		if (tokens.size() >= 1) {
			if (!db.close_DB()) {
				cerr << "关闭失败，因为你还未选择任何数据库或表" << endl;
				break;
			}
		}
		else if (tokens.size() >= 2 && tokens[1] == "-LS") {
			if (!db.is_use()) {
				cerr << "关闭失败，因为你还未选择任何数据库或表" << endl;
				break;
			}
			if (!db.DB_op()->close_list()) {
				cerr << "关闭失败，因为你还未选择任何数据库或表" << endl;
				break;
			}
		}
		else if (tokens.size() >= 2 && tokens[1] == "-DB") {
			if (!db.close_DB()) {
				cerr << "关闭失败，因为你还未选择任何数据库或表" << endl;
				break;
			}
		}
		break;

	case CREATE:
		//CREATE [name]
		if (tokens.size() >= 2) 
		{
			DB_CONFIG new_cf(tokens[1]);
			if (!db.new_db(new_cf))
			{
				cerr << "创建数据库失败！" << endl;
				break;
			}
			else
			{
				cout << "已成功创建数据库，请使用 SEL -DB " << tokens[1] << " 指令来选中它！" << endl;
				break;
			}
		}
		break;

	case REMOVE:
		if (tokens.size() >= 2)
		{
			try {
				int i = -1;
				i = stoi(tokens[1]);
				if (!db.del_db(i)) {
					cerr << "删除失败！" << endl;
					break;
				}
				else
				{
					cout << "删除成功！" << endl;
					break;
				}
			}
			catch (const invalid_argument& e)
			{
				cerr << "你需要输入id！" << endl;
				break;
			}
		}
		break;

	default:
		cerr << "未知指令，请你检查检查！" << endl;
		break;
	}
	if (is_set_socket()) {
		return_data(_return);
	}
}
