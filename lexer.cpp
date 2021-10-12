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


//////// FUNCTION PROTOTYPES //////////
void load_data(); // đọc dữ liệu từ file .dat vào 3 bảng: keywords, endstates và table 
void remove_comments(istream& file); // xóa bỏ comment từ "file" và lưu vào "source_code"

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

	return 0;
}
// string role;
// char last;
// while (input)
// {
//    last 
//    if (is_number(input))
//    {
//       role = number
//    }
//    state = tb[state][role];
//    if (!state)
//    {
//       state = tb[state][other]
//    }
//    if is_end_state(state)
//    {

//       a = tach_tu() // apples
//       if is_key_word(a)      
//       {
//          symbol_table[a] = keywords[a];
//       }
//       state = 0
//       continue
//    }
//    last = input
// }


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

	// duyệt từ đầu đến cuối, sao chép những ký tự không trong comment sang mảng này
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
			single_cmt = false;

		// đang trong multi comment và gặp "*/"
		else if (multi_cmt == true && source_code[i] == '*' && source_code[i+1] == '/')
			multi_cmt = false, i++;

		// bỏ qua tất cả ký tự nằm trong comment, kể cả nnững comment khác
		else if (single_cmt || multi_cmt)
			continue;
		else if (source_code[i] == '/' && source_code[i+1] == '/')
			single_cmt = true, i++;
		else if (source_code[i] == '/' && source_code[i+1] == '*')
			multi_cmt = true, i++;

		// giữ lại những ký tự không nằm trong comment
		else res[i] = source_code[i];
	}
	source_code = res;
}