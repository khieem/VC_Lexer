# VC Lexer
## Tổng quan
Đây là chương trình phân tích từ tố cho ngôn ngữ VC. Mã VC đầu vào nằm trong file `*.vc`, đầu ra là [`out.vctok`](out.vctok) chứa danh sách các từ tố và chức năng tương ứng, mỗi từ một dòng, trong trường hợp không có bất kỳ lỗi nào xuất hiện trong mã nguồn. Nếu không, một danh sách gồm những token lỗi và số dòng tương ứng sẽ được in ra màn hình console.

## Bắt đầu
Mã nguồn nằm tại [`lexer.cpp`](lexer.cpp)  
```
> g++ lexer.cpp -o lexer
> ./lexer
``` 
Chương trình sẽ mặc định tìm đến file `in.vc`. Tuy nhiên có thể cung cấp thêm tham số `path` là đường dẫn tới  mã nguồn  
```
> ./lexer path
```
## Cấu hình
[`transition.dat`](transition.dat) chứa các thông tin cần thiết cho việc phân tích và đoán nhận từ tố. Thông tin được lưu thành 3 phần như sau, lưu ý giữa mỗi phần phải cách nhau 1 dòng trống.

- KEYWORDS. Danh sách các từ khóa, dùng để phân biệt với định danh (identifier).
- ENDSTATES. Các trạng thái kết thúc và vai trò tương ứng với mỗi trạng thái kết thúc đó. Mỗi trạng thái trên một dòng.
- DFA. Thông tin về automat. Mỗi trạng thái trên một dòng, sau mỗi trạng thái là các cặp `(input output)` tương ứng.
