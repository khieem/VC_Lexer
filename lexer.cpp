#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <regex>

using namespace std;

/////// FILE PATH CONSTANTS ////////////
const string SRC = "in.vc";
const string DST = "out.vctok";
const string DAT = "transition.dat";


/////// GLOBAL VARIABLES ///////////
set<string> keywords;
string endstates[25];
map<string, int> table[25]; // transition table, implemented as a map
string source_code; // source code file as a string for convenience
vector<string> tokens; // kết quả phân tích

//////// FUNCTION PROTOTYPES //////////
void load_data(); // đọc dữ liệu từ file .dat vào 3 bảng: keywords, endstates và table 
void remove_comments(istream& file); // xóa bỏ comment từ "file" và lưu vào "source_code"
string reduce(char c); // chuyển ký tự 'c' về dạng đầu vào DFA
void write_output(string des = "out.vctok"); // ghi kết quả phân tích ra "des"


//////////// MAIN //////////////
int main(int argc, char const *argv[])
{
	ifstream file;
	switch (argc)
	{
		case 1:
			file.open(SRC);
			if (!file.good())
			{
				cerr << "Hay dat ma nguon trong file \"" << SRC << "\"";
				return -1;
			}
			break;
		case 2:
			file.open(argv[1]);
			if (!file.good())
			{
				cerr << "Khong tim thay \"" << argv[1] << "\"";
				return -1;
			}
			break;
	}
	
	load_data();	
	remove_comments(file);

	int linecount = 1;
	int i = 0, j = 0;
	char c;
	int state = 0;
	string input;
	bool error = false;
	vector<char> buffer;
	while (c = source_code[j++])
	{
		if (int(c) < 0) break; // đừng đặt điều kiện này vào trong while
		if (c == '\n') {
			++linecount;
			continue;
		}
		input = reduce(c);

		if (table[state].count(input) == 0)	{
			if (table[state].count("other") == 0) // k nhận input
			{
				error = true;
				cout << "Dong " << linecount << ". '" << string(buffer.begin(), buffer.end()) + c  << "'" << " is not recognized!" << endl;
				buffer.clear();
				state = 0;
				continue;
			}
			else // nhận input qua other
			{

				state = table[state].at("other");
				--j;
			}
		}
		else // nhận input bình thường
		{
			if (input != "ws") buffer.push_back(c);
			state = table[state].at(input);
		}
		string line;
		if (endstates[state] != "") // state laf trang thai ket thuc
		{
			string s = {buffer.begin(), buffer.end()};
			if (s == "true" || s == "false")
			{
				line = s + " " + "literal";
			}
			else if (keywords.count(s))
			{
				line = s + " " + "keyword";
			}
			else line = s + " " + endstates[state];
			buffer.clear();
			tokens.push_back(line);
			state = 0;
		}
		// cout << c << state << " ";
	}

	if (!error)
	write_output();
	return 0;
}


////// FUNCTION IMPLEMENTATIONS //////
void load_data()
{
	ifstream file;
	file.open(DAT);
	if (!file.good())
	{
		cerr << "Khong tim thay \"" << DAT << "\"";
		exit(-1);
	}
	string line;
	string word;
	getline(file, line); // KEYWORDS

	getline(file, line); // boolean break continue else for float if int return void while
	stringstream ss(line);
	while (ss >> word)
		keywords.insert(word);
	ss.clear();

	getline(file, line); // 

	getline(file, line); // ENDSTATES
	int state;
	string attr;
	while (getline(file, line) && line != "")
	{
		ss.str(line);
		ss >> state >> attr;
		endstates[state] = attr;
		ss.clear();
	}

	getline(file, line); // DFA

	regex pt("[\\w\\.\"<>!=|&+*/-]+ +\\d+"); // khớp với cặp "string int"
	smatch sm;

	string input;
	int output;
	while (getline(file, line))
	{
		state = stoi(line.substr(0, 2)); // state nằm ở đầu dòng
		while (regex_search(line, sm, pt)) // tìm từng cặp input output
		{
			ss.str(sm[0]);
			ss >> input >> output;
			table[state][input] = output;
			line = sm.suffix().str();
			ss.clear();
		}
	}
}

void remove_comments(istream& file)
{
	source_code = {istreambuf_iterator<char>(file), istreambuf_iterator<char>{}};
	int n = source_code.length();

	/* CHÚ Ý. dùng mảng sao chép từng ký tự thay cho phép nối string
	chạy hiệu quả hơn rất nhiều hưng khiến từ bị xê dịch trong dòng
	chưa rõ có thể gây lỗi trong việc đếm dòng hay không. nếu có thì chuyển sang cách nối string
	*/
	// string res = "";
	char res[n];
	for (size_t i = 0; i < n; i++)
		res[i] = ' ';	

	bool single_cmt = false;
	bool multi_cmt = false;

	int last = 0;
	for (int i = 0; i < n; ++i)
	{
		
		// đang trong single comment và gặp kết dòng
		if (single_cmt == true && source_code[i] == '\n')
		{
			single_cmt = false;
			// res += '\n';
			res[i] = '\n';
		}

		// đang trong multi comment và gặp "*/"
		else if (multi_cmt == true && source_code[i] == '*' && source_code[i+1] == '/')
			multi_cmt = false, i++;

		// bỏ qua tất cả ký tự nằm trong comment, kể cả nnững comment khác
		else if (single_cmt || multi_cmt)
		{
			if (source_code[i] == '\n')
				// res += source_code[i];
				res[i] = source_code[i];
		}
		else if (source_code[i] == '/' && source_code[i+1] == '/')
			single_cmt = true, i++;
		else if (source_code[i] == '/' && source_code[i+1] == '*')
			multi_cmt = true, i++;

		// giữ lại những ký tự không nằm trong comment
		else res[i] = source_code[i];
		// else res += source_code[i];
	}
	source_code = res;
}

string reduce(char c)
{
	if (c == 'e' || c == 'E') return "E"; // e_notation_float
	if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_') return "letter";
	if ('0' <= c && c <= '9') return "digit";
	if (c == '{' || c == '}'|| c == '['|| c == ']' || c == '('|| c == ')'|| c == ';'|| c == ',')
		return "seperator";
	if (c == ' ' || c == '\t' || c == '\n') return "ws";
	return string(1, c);
}

void write_output(string des)
{
	ofstream file(des);
	for (const string& s : tokens)
		file << s << '\n';
}