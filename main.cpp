#include <iostream>
#include <string>
#include "Wordle.hpp"


using namespace std;

int main(int argc, char **argv){
	// 取得執行檔路徑
	string directory = argv[0];

	// 取得執行檔目錄
	while(directory.back() != '/' && directory.back() != '\\'){
		directory.pop_back();
	}

	// 建立wordle
	Wordle wd(directory);

	// 建立困難度和模式
	char difficulty = 'n';
	char mode = 's';

	// 開始執行程式
	while(true){
		cout << "Select difficulty" << endl;
		cout << "n = normal, h = hard" << endl;
		cin >> difficulty;

		cout << "Select mode" << endl;
		cout << "s = solve mode, t = test mode" << endl;
		cin >> mode;

		wd.change_difficulty(difficulty);
		wd.change_mode(mode);

		if(mode == 's'){
			wd.solve();
		}
		else if(mode == 't'){
			int test_times = 0;
			cout << "Input test times" << endl;
			cin >> test_times;
			wd.print_test_result(test_times);
		}
	}
}