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
		//����ܷ�ָ�
		if (token.size() < 2 || token.front() != '"' || token.back() != '"') {
			return {};
		}
		// ȥ����ͷ�ͽ�β��˫����
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
			cerr << "������������������Ĳ���" << endl;
			json j = hint_to_json("error", "������������������Ĳ���", "0");
			_return = j.dump(-1);
			break;
		}

		if (!db.is_use()) 
		{
			cerr << "����ѡ�����ݿ⣡" << endl;
			json j = hint_to_json("error", "����ѡ�����ݿ⣡", "0");
			_return = j.dump(-1);
			break;
		}

		//ADD -R [add_data]
		if (tokens.size() >= 3 && tokens[1] == "-R")
		{
			if (!db.DB_op()->is_sel())
			{
				cerr << "����ѡ���" << endl;
				json j = hint_to_json("error", "����ѡ���", "0");
				_return = j.dump(-1);
				break;
			}

			vector<string> arg = split_args(tokens[2]);
			if (arg.size() <= 0) {
				cerr << "ָ���������!" << endl;
				json j = hint_to_json("error", "ָ���������!", "1");
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
				cerr << "����ѡ���" << endl;
				break;
			}

			vector<string> arg;
			if (tokens.size() >= 4) arg = split_args(tokens[3]);
			
			/*
			if (arg.size() <= 0) {
				cerr << "ָ���������!" << endl;
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
				cerr << "ָ���������!" << endl;
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
			cerr << "����ѡ�����ݿ⣡" << endl;
			_return = "[error],[\"����ѡ�����ݿ⣡\"]";
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
				cerr << "����ѡ���" << endl;
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
				cerr << "����ѡ���" << endl;
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
			//�������idɾ����������ɾ��
			catch (const invalid_argument& e) {
				if (db.DB_op()->del_List(tokens[1])) {
					cout << "delete success!" << endl;
					break;
				}
			}
			//���ɾ��ʧ��
			catch (const string& e) {
				if (e == "Delete_Failed") cerr << e << endl;
				break;
			}
		}
		break;

	case INS:
		if (!db.is_use()) {
			cerr << "����ѡ�����ݿ⣡" << endl;
			break;
		}
		else if (!db.DB_op()->is_sel()) {
			cerr << "����ѡ���" << endl;
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
			cerr << "����ѡ�����ݿ⣡" << endl;
			break;
		}
		else if (!db.DB_op()->is_sel()) {
			cerr << "����ѡ���" << endl;
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
					cerr << "ָ���������!" << endl;
					break;
				}
				vector<string> arg = split_args(tokens[3]);
				if (!db.DB_op()->LS_op()->set_row(row_index, arg)) {
					cerr << "������ʧ��!" << endl;
					break;
				}
				cout << "�� " << row_index << " �е������Ѹı�" << endl;
			}

			// SET -C [col] [data] [from] [to]
			else if (tokens[1] == "-C") {
				//col
				int column_index = 0;
				try {
					column_index = stoi(tokens[2]);
				}
				catch (const invalid_argument& e) {
					cerr << "ָ���������!" << endl;
					break;
				}

				vector<string> arg = split_args(tokens[3]);
				
				//���[data]���滹�в���
				if (tokens.size() >= 5) {
					//from
					int start_row = 0;
					try {
						start_row = stoi(tokens[4]);
					}
					catch (const invalid_argument& e) {
						//fromת�����ɹ������0��ʼ����
						if (!db.DB_op()->LS_op()->set_col(column_index, arg, 0)) {
							cerr << "������ʧ��!" << endl;
						}
						cout << "�� 0 �е��� " << arg.size() << " �е������Ѹı䣡" << endl;
						break;
					}
					//to
					int end_row = -1;
					if (tokens.size() >= 6) {
						try {
							//from endת���ɹ���ʹ��from��end������
							end_row = stoi(tokens[5]);
							if (!db.DB_op()->LS_op()->set_colc(column_index, arg, start_row, end_row)) {
								cerr << "������ʧ��!" << endl;
								break;
							}
							cout << "��" << start_row << " �е��� " << end_row << " �е������Ѹı䣡" << endl;
						}
						catch (const invalid_argument& e) {
							//end���ɹ������from��ʼ��������Ϊ��Χ��������
							if (!db.DB_op()->LS_op()->set_col(column_index, arg, start_row)) {
								cerr << "������ʧ��!" << endl;
								break;
							}
							cout << "��" << start_row << " �е��� " << arg.size() << " �е������Ѹı䣡" << endl;
						}
					}
					else
					{
						if (!db.DB_op()->LS_op()->set_col(column_index, arg, start_row)) {
							cerr << "������ʧ��!" << endl;
						}
						cout << "��" << start_row << " �е��� " << arg.size() << " �е������Ѹı䣡" << endl;
						break;
					}
				}
				//[data]�����޲���
				else {
					if (!db.DB_op()->LS_op()->set_colc(column_index, arg, 0,db.DB_op()->LS_op()->_data.size())) {
						cerr << "������ʧ��!" << endl;
						break;
					}
					cout << "�� " << column_index << " �е������Ѹı�" << endl;
				}
			}

			// SET [row] [col] [data]
			else {
				try {
					int row_index = stoi(tokens[1]);
					int column_index = stoi(tokens[2]);
					if (!db.DB_op()->LS_op()->set(row_index, column_index, tokens[3])) {
						cerr << "���õ�Ԫ��ʧ��!" << endl;
						break;
					}
					cout << "�� " << row_index << " �е� " << column_index << " �е������Ѹı�" << endl;
				}
				catch (const invalid_argument& e) {
					cerr << "ָ���������!" << endl;
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
				cerr << "��û��ѡ���κ����ݿ⣬�޷��鿴�����б�" << endl;
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
			cerr << "δѡ�����ݿ⣡" << endl;
			break;
		}
		else if(!db.DB_op()->is_sel()) {
			cerr << "δѡ���" << endl;
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
						cerr << "δ��ȡ�����У����в����ڣ�" << endl;
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
					cerr << "ѡ��ʧ�ܣ���ѡ����һ�������ڵ����ݿ⣡" << endl;
					break;
				}
			}
			catch (const invalid_argument& e) {
				if (!db.use_db(tokens[2])) {
					cerr << "ѡ��ʧ�ܣ���ѡ����һ�������ڵ����ݿ⣡" << endl;
					break;
				}
			}
			cout << "ѡ�����ݿ⣺" << db.DB_op()->get_name() << endl;
			break;
		}
		//SEL -LS [name]
		else if (tokens.size() >= 3 && tokens[1] == "-LS") {
			if (!db.is_use()) {
				cerr << "δѡ�����ݿ⣬����ѡ�����ݿ⣡" << endl;
				break;
			}
			try {
				int id = stoi(tokens[2]);
				if (!db.DB_op()->select_list(id)) {
					cerr << "ѡ��ʧ�ܣ���ѡ����һ�������ڵı�" << endl;
					break;
				}
			}
			catch (const invalid_argument& e) {
				if (!db.DB_op()->select_list(tokens[2])) {
					cerr << "ѡ��ʧ�ܣ���ѡ����һ�������ڵı�" << endl;
					break;
				}
			}
			cout << "ѡ���б�" << db.DB_op()->LS_op()->get_name() << endl;
			break;
		}
		break;

	case CLOSE:
		//CLOSE -LS/-DB
		if (tokens.size() >= 1) {
			if (!db.close_DB()) {
				cerr << "�ر�ʧ�ܣ���Ϊ�㻹δѡ���κ����ݿ���" << endl;
				break;
			}
		}
		else if (tokens.size() >= 2 && tokens[1] == "-LS") {
			if (!db.is_use()) {
				cerr << "�ر�ʧ�ܣ���Ϊ�㻹δѡ���κ����ݿ���" << endl;
				break;
			}
			if (!db.DB_op()->close_list()) {
				cerr << "�ر�ʧ�ܣ���Ϊ�㻹δѡ���κ����ݿ���" << endl;
				break;
			}
		}
		else if (tokens.size() >= 2 && tokens[1] == "-DB") {
			if (!db.close_DB()) {
				cerr << "�ر�ʧ�ܣ���Ϊ�㻹δѡ���κ����ݿ���" << endl;
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
				cerr << "�������ݿ�ʧ�ܣ�" << endl;
				break;
			}
			else
			{
				cout << "�ѳɹ��������ݿ⣬��ʹ�� SEL -DB " << tokens[1] << " ָ����ѡ������" << endl;
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
					cerr << "ɾ��ʧ�ܣ�" << endl;
					break;
				}
				else
				{
					cout << "ɾ���ɹ���" << endl;
					break;
				}
			}
			catch (const invalid_argument& e)
			{
				cerr << "����Ҫ����id��" << endl;
				break;
			}
		}
		break;

	default:
		cerr << "δָ֪��������飡" << endl;
		break;
	}
	if (is_set_socket()) {
		return_data(_return);
	}
}
