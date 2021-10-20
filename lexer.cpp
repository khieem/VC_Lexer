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

// transition table, implemented as a map
map<string, int> table[25];

// source code file as a string for convenience
string source_code;

// kết quả phân tích
vector<string> tokens;

// đọc dữ liệu từ file .dat vào 3 bảng: keywords, endstates và table
void load_data();

// xóa bỏ comment từ "file" và lưu vào "source_code"
void remove_comments( istream& file );

// chuyển ký tự 'c' về dạng đầu vào DFA
string reduce( char c );

// ghi kết quả phân tích ra "des"
void write_output( string des = "out.vctok" );

// báo lỗi ở trạng thái "state" tại dòng "line"
void inform_error_at( int line, int state, vector<char>& buffer );


int main( int argc, char const *argv[] )
{
	ifstream file;
	switch ( argc )
	{
		case 1:
			file.open( SRC );
			if ( !file.good() )
			{
				cerr << "Put VC source code in \"" << SRC << "\"";
				return -1;
			}
			break;
		case 2:
			file.open( argv[ 1 ] );
			if ( !file.good() )
			{
				cerr << "Can not find \"" << argv[1] << "\"";
				return -1;
			}
			break;
	}
	
	load_data();	
	remove_comments( file );

	int linecount = 1;
	int j = 0;
	char c;
	int state = 0;
	string input;
	bool error = false;
	vector<char> buffer;

	while ( c = source_code[ j++ ] )
	{
		if ( int( c ) < 0 ) break; // đừng đặt điều kiện này vào trong while
		if ( c == '\n' ) ++linecount;

		input = reduce( c );
		if ( table[ state ].count( input ) == 0 )
		{
			// tại state không nhận input => lỗi
			if ( table[ state ].count( "other" ) == 0)
			{
				error = true;
				
				/* ngoại lệ tại state=4 với ký tự đọc được là 'e'
				 * automat đợi 1 chữ số hoặc +- để hoàn thành dạng e_notation_float
				 * tuy nhiên vẫn có thể chấp nhận chữ cái trong trường hợp
				 * indentifier là "2efgh". điều kiện if này để xử lý
				 * trường hợp đó, giúp hạn chế sửa đổi automat
            */
				if ( state == 4 )
				{
					// lấy lại ký tự ngay sau ký tự 'e'
					buffer.push_back( source_code[ j-1 ] );

					// tiếp tục lấy tất cả ký tự còn lại. vd 2ed[dfjegow] = 5;
					while ( isalpha( source_code[ j ] ) || isdigit( source_code[ j ] ) )
					{
						buffer.push_back( source_code[ j++ ] );
					}
				}
				
				inform_error_at(linecount, state, buffer);

				// khởi động lại automat
				buffer.clear();
				state = 0;
				continue;
			}
			else // nhận input qua "other"
			{
				/**
				 * xử lý indentifier không đúng
				 * (bên trên là trường hợp riêng khi có 'e' ngay sau digit đứng đầu tiên)
				 * bởi sau 'e' chỉ có thể nhận digit hoặc +-, gặp letter sẽ rơi vào if bên trên
				 * trong khi sau digit có thể nhận "other", trong đó có cả letter
				 * làm thế này để tránh phải sửa automat và sửa nhiều code, dù code sẽ bị lặp
				 */
				if ( state == 1 && ( input == "letter" || input == "E" ) )
				{
					error = true;
					while ( isalpha( source_code[ j ] ) || isdigit( source_code[ j ] ) )
					{
						buffer.push_back( source_code[ j++ ] );
					}
					inform_error_at(linecount, state, buffer);
					buffer.clear();
					state = 0;
					continue;
				}
				// nhận mọi ký tự nếu đang trong string literal
				else if ( state == 11 )
				{
					buffer.push_back(c);
					continue;
				}
				// nếu không thì lùi j đi 1 ký tự do trạng thái kết thúc đọc quá 1 ký tự
				else --j;

				state = table[ state ][ "other" ];
			}
		}
		else // nhận input bình thường
		{
			if ( input != "ws" ) buffer.push_back( c );
			state = table[ state ][ input ];
		}

		string pair, word;
		if ( endstates[ state ] != "" ) // state là trạng thái kết thúc
		{
			word = { buffer.begin(), buffer.end() };
			if ( word == "true" || word == "false" )
			{
				pair = word + " " + "literal";
			}
			else if ( keywords.count( word ) )
			{
				pair = word + " " + "keyword";
			}
			else pair = word + " " + endstates[ state ];

			buffer.clear();
			tokens.push_back( pair );
			state = 0;
		}
	}

	if ( !error )
	{
		write_output();
		cout << "Sucessfully analyzed! Output written to \"" << DST << "\"" << endl;
	}
	return 0;
}


void load_data()
{
	ifstream file;
	file.open( DAT );
	if ( !file.good() )
	{
		cerr << "Khong tim thay \"" << DAT << "\"";
		exit( -1 );
	}
	string line;
	string word;
	getline( file, line ); // KEYWORDS

	getline( file, line ); // boolean break continue else for float if int return void while
	stringstream ss( line );
	while ( ss >> word )
		keywords.insert( word );
	ss.clear();

	getline( file, line ); // 

	getline( file, line ); // ENDSTATES
	int state;
	string attr;
	while ( getline( file, line ) && line != "" )
	{
		ss.str( line );
		ss >> state >> attr;
		endstates[ state ] = attr;
		ss.clear();
	}

	getline( file, line ); // DFA

	regex pt( "[\\w\\.\"<>!=|&+*/-]+ +\\d+" ); // khớp với cặp "string int"
	smatch sm;

	string input;
	int output;
	while ( getline( file, line ) )
	{
		state = stoi( line.substr( 0, 2 ) ); // state nằm ở đầu dòng
		while ( regex_search( line, sm, pt ) ) // tìm từng cặp input output
		{
			ss.str( sm[ 0 ] );
			ss >> input >> output;
			table[ state ][ input ] = output;
			line = sm.suffix().str();
			ss.clear();
		}
	}
}

void remove_comments( istream& file )
{
	source_code = { istreambuf_iterator<char>(file), istreambuf_iterator<char>{} };
	int n = source_code.length();

	/* CHÚ Ý. dùng mảng sao chép từng ký tự thay cho phép nối string
	chạy hiệu quả hơn rất nhiều hưng khiến từ bị xê dịch trong dòng
	chưa rõ có thể gây lỗi trong việc đếm dòng hay không. nếu có thì chuyển sang cách nối string
	*/
	// string res = "";
	vector<char> res(n);
	for ( size_t i = 0; i < n; i++ )
		res[i] = ' ';	

	bool single_cmt = false;
	bool multi_cmt = false;

	for ( int i = 0; i < n; ++i )
	{		
		// đang trong single comment và gặp kết dòng
		if ( single_cmt == true && source_code[i] == '\n' )
		{
			single_cmt = false;
			// res += '\n';
			res[ i ] = '\n';
		}

		// đang trong multi comment và gặp "*/"
		else if ( multi_cmt == true && source_code[ i ] == '*' && source_code[ i+1 ] == '/' )
			multi_cmt = false, i++;

		// bỏ qua tất cả ký tự nằm trong comment, kể cả nnững comment khác
		else if ( single_cmt || multi_cmt )
		{
			if ( source_code[ i ] == '\n' )
				// res += source_code[i];
				res[ i ] = source_code[ i ];
		}
		else if ( source_code[ i ] == '/' && source_code[ i+1 ] == '/' )
			single_cmt = true, i++;
		else if ( source_code[ i ] == '/' && source_code[ i+1 ] == '*' )
			multi_cmt = true, i++;

		// giữ lại những ký tự không nằm trong comment
		else res[ i ] = source_code[ i ];
		// else res += source_code[i];
	}
	for(size_t i=0;i<n;i++)
		source_code[i]=res[i];
}

string reduce( char c )
{
	if ( c == 'e' || c == 'E' ) return "E"; // e_notation_float
	if ( 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' ) return "letter";
	if ( '0' <= c && c <= '9' ) return "digit";
	if ( c == '{' || c == '}'|| c == '['|| c == ']' || c == '('|| c == ')'|| c == ';'|| c == ',' )
		return "seperator";
	if ( c == ' ' || c == '\t' || c == '\n' ) return "ws";
	return string( 1, c );
}

void write_output( string des )
{
	ofstream file( des );
	for ( const string& s : tokens )
		file << s << '\n';
}

void inform_error_at( int line, int state, vector<char>& buffer )
{
	cout << SRC << ": line " << line << ": \"" 
	     << string( buffer.begin(), buffer.end() ) << "\"" 
		  << " is not a valid " 
		  << ( state != 21 && state != 23 ? "indentifier" : "operator" ) << endl;
}