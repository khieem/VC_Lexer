#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <regex>

using namespace std;

const string SRC = "in.vc";
const string DST = "out.vctok";
const string DAT = "transition.dat";

set<string> keywords;
string endstates[25];
map<string, int> table[25];

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
	getline(file, line); // bỏ dòng đầu tiên, tạm thời cứ thế này đã
	getline(file, line);
	stringstream ss(line);
	while (ss >> word)
	{
		keywords.insert(word);
	}
	ss.clear();

	getline(file, line); // dong trong
	getline(file, line); // ENDSTATES
	getline(file, line);
	int state;
	string attr;
	while (getline(file, line) && line != "")
	{
		ss.str(line);
		ss >> state >> attr;
		endstates[state] = attr;
		ss.clear();
	}

	getline(file, line);

	regex pt("[\\w\\.\"<>!=|&+*/-]+ +\\d+");
	smatch sm;

	string input;
	int output;
	while (getline(file, line))
	{
		state = stoi(line.substr(0, 2));
		while (regex_search(line, sm, pt))
		{
			ss.str(sm[0]);
			ss >> input >> output;
			table[state][input] = output;
			line = sm.suffix().str();
			ss.clear();
		}
	}

	map<string, int>::iterator it;
	for (size_t i = 0; i < 25; i++)
	{
		cout << i << " ";
		for (auto r : table[i])
		{
			cout << "(" << r.first << " " << r.second << ")" << " ";
		}
		cout << endl;
		
	}
	

	// cout << line;
	// for (size_t i = 0; i < 25; i++)
	// {
	// 	cout << endstates[i] << " ";
	// }
	

	exit(1);
	// set<string>::iterator it;
	// for (it = keywords.begin(); it != keywords.end(); ++it)
	// {
	// 	cout << *it << "\n";
	// }
}

int main(int argc, char const *argv[])
{
	ifstream in;
	switch (argc)
	{
		case 1:
			in.open(SRC);
			if (!in.good())
			{
				cerr << "Hay dat ma nguon trong file \"" << SRC << "\"";
				return -1;
			}
			break;
		case 2:
			in.open(argv[1]);
			if (!in.good())
			{
				cerr << "Khong tim thay \"" << argv[1] << "\"";
				return -1;
			}
			break;
	}
	
	load_data();
	return 1;

	

	

	string line;
	while (getline(in, line))
	{
		stringstream ss(line);


	}
	return 0;
}
// cin >> state
// while (cin >> key && cin >> value)
// {
//    table[state][key] = value
// }


// table

// #
// int a = 1
//   14 

// universal symbol table

// int state = 0


// if input == e or == E 
// {
//    role = exponent
// }

// nhen

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
