#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <fstream>
#include <type_traits>
#include <ctime>
#include <random>
#include <chrono>
using namespace std;


// 程式所需變數
const int total_words = 12972; // 總字數
const int total_diffs = 243; // 兩單字經diff運算後可得的最大值+1

char difficulty; // n = normal difficulty, h = hard difficulty
char mode; // s = solve mode, t = test mode

vector<string> all_words(total_words); // 所有可以猜的單字
vector<vector<int>> all_words_diff(total_words, vector<int>(total_words)); // 所有單字互相diff後的值
vector<int> possible_ans_idx; // 可能答案的index


// 清空所有可以猜的單字index回到初始狀態
void clear_possible_ans_idx(){
	possible_ans_idx.clear();
	for(int i = 0; i < total_words; i++){
		possible_ans_idx.push_back(i);
	}
	return;
}

// 計算答案和猜測的diff值
int diff_answer_and_guess(const int &ans_idx, const int &guess_idx){
	return all_words_diff[ans_idx][guess_idx];
}


// 計算數據的變異數
double variance(const vector<int> &data){
	int data_size = data.size();
	double avg = 0.0;
	double sum = 0.0;
	for(int i = 0; i < data_size; i++){
		sum += (double)data[i];
	}
	avg = sum / (double)data_size;
	double var = 0.0;
	for(int i = 0; i < data_size; i++){
		var += (avg-(double)data[i]) * (avg-(double)data[i]) / (double)data_size;
	}
	return var;
}


// 將原始數據離散化
vector<int> discretize(vector<int> &data){
	int data_size = data.size();
	vector<int> discretize_data(data_size, 0);
	vector<int> is_used(total_diffs, -1);
	int idx = 0;
	for(int i = 0; i < data_size; i++){
		if(is_used[data[i]] == -1){
			is_used[data[i]] = idx;
			discretize_data[idx]++;
			idx++;
		}
		else{
			discretize_data[is_used[data[i]]]++;
		}
	}
	return discretize_data;
}


// 回傳編碼後的結果
int get_encoded_result(const string &raw_result){
	int encoded_result = 0;
  	int pow = 1;
  	for(int i = 0; i < 5; i++){
  		if(raw_result[i] == 'o'){
  			encoded_result += pow*2;
  		}
  		else if(raw_result[i] == '_'){
  			encoded_result += pow;
  		}
  		pow *=3;
  	}
  	return encoded_result;
}


// 將可能的答案篩出
template <typename T>
int filter_answer(const int &best_guess_idx, const T &raw_result){
	// 將結果轉換為編碼後的格式
	int result = 0;
	if constexpr(is_same_v<T, int>){
		result = raw_result;
	}
	else if constexpr(is_same_v<T, string>){
		result = get_encoded_result(raw_result);
	}
	else{
		assert(false);
	}
	

	// 將結果與可能的答案比對
	vector<int> new_possible_ans_idx;
	for(unsigned int i = 0; i < possible_ans_idx.size(); i++){
		if(diff_answer_and_guess(possible_ans_idx[i], best_guess_idx) == result){
			new_possible_ans_idx.push_back(possible_ans_idx[i]);
		}
	}
	possible_ans_idx.clear();
	possible_ans_idx = new_possible_ans_idx;
	

	// 若找到正確解答則回傳解答的index, 否則回傳-1
	if(possible_ans_idx.size() == 1){
		// cout << "Correct answer: " << all_words.at(possible_ans_idx.at(0)) << endl;
		return possible_ans_idx.at(0);
	}
	else{
		return -1;
	}
}


// 計算最佳猜測的單字編號
int guess(){ 
	// 初始參數
	double min_variance = 999999999.0;
	int best_guess_idx  = 0;
	

	// 預處理得知lares為遊戲開始時的最佳猜測
	if(possible_ans_idx.size() == total_words){
		best_guess_idx = 7313; // lares 的 index
		return best_guess_idx;
	}
	

	//取得所有可猜測字對可能答案的變異數，並選擇最小的那個作為最佳猜測
	if(difficulty == 'n'){ // normal mode
		for(unsigned int i = 0; i < total_words; i++){
			vector<int> diff_result;
			for(unsigned int j = 0; j < possible_ans_idx.size(); j++){
				diff_result.push_back(diff_answer_and_guess(possible_ans_idx[j], i));
			}
			vector<int> distribution = discretize(diff_result);
			double var = variance(distribution);
			if(var < min_variance){
				min_variance = var;
				best_guess_idx = i;
			}
		}
	}
	else{ // hard mode
		for(unsigned int i = 0; i < possible_ans_idx.size(); i++){
			vector<int> diff_result;
			for(unsigned int j = 0; j < possible_ans_idx.size(); j++){
				diff_result.push_back(diff_answer_and_guess(possible_ans_idx[j], possible_ans_idx[i]));
			}
			vector<int> distribution = discretize(diff_result);
			double var = variance(distribution);
			if(var < min_variance){
				min_variance = var;
				best_guess_idx = possible_ans_idx[i];
			}
		}
	}
	// 回傳最佳猜測答案的編號 
	return best_guess_idx;
}


//初始化程式
void init(){
	cout << "Initializing..." << endl;


	// 開啟必要檔案
  	ifstream words ("/Users/ryanovovo/Documents/GitHub/wordle-solver/words.txt");
  	ifstream diff  ("/Users/ryanovovo/Documents/GitHub/wordle-solver/all_words_diff.txt");


  	//檢查是否成功開啟檔案
  	assert(words.is_open());
  	assert(diff.is_open());

  	string line; // 暫存讀入的文字

  	//將檔案內容導入容器中

  	//將可能的答案編號（0~total_words）導入容器中
  	for(int i = 0; i < total_words; i++){
  		possible_ans_idx.push_back(i);
  	}

  	// 將可以猜的單字導入到容器中
	for(int i = 0; getline(words, line); i++){
		all_words[i] = line;
	}
	words.close();
  	
	// 將所有單字的diff值導入容器中
	for(int i = 0; getline(diff, line); i++){
		string tmp;
		int it = 0;
		for(auto ch : line){
			if(ch == ' '){
				all_words_diff[i][it] = stoi(tmp);
				it++;
				tmp.clear();
			}
			else{
				tmp.push_back(ch);
			}
		}
	}
	diff.close();

	cout << "Finished Initializing" << endl;

	// 選取困難度
	cout << "choose game difficulty" << endl;
	cout << "n = normal mode, h = hard mode" << endl;
	cin >> difficulty;

	// 選取模式
	cout << "Select mode" << endl;
	cout << "s = solve mode, t = test mode" << endl;
	cin >> mode;
	
  	return;
}


// 測試指定範圍內的單字所需的猜測次數，並回傳統計次數
vector<int> test(int times){
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	minstd_rand0 rnd (seed);
	vector<int> counter;
	for(int i = 0; i < times; i++){
		clear_possible_ans_idx();
		int cnt = 1;
		int answer = (rnd() % total_words);
		while(true){
			cnt++;
			int best_guess_idx = guess();
			int raw_result = diff_answer_and_guess(answer, best_guess_idx);
			int correct_ans_idx = filter_answer(best_guess_idx, raw_result);
			if(correct_ans_idx > 0 || raw_result == 242){
				cout << "Answer: " << all_words.at(correct_ans_idx) << " guess " << cnt << " times" << endl;
				break;
			}
		}
		if(cnt >= counter.size()){
			counter.resize(cnt+1);
		}
		counter[cnt]++;
	}
	return counter;
}


// 開始解wordle
void solve(){
	while(true){
		clear_possible_ans_idx();
		while(true){
			int best_guess_idx = guess();
			cout << "best guess: " << all_words.at(best_guess_idx) << endl;
			string raw_result;
			cin >> raw_result;
			if(raw_result == "stop"){
				return;
			}
			int correct_ans_idx = filter_answer(best_guess_idx, raw_result);
			if(correct_ans_idx > 0){
				cout << "Correct answer: " << all_words.at(correct_ans_idx) << endl;
				break;
			}
		}
	}
}


int main(){
	init();
	if(mode == 's'){
		solve();
	}
	else if(mode == 't'){
		int times;
		cout << "Input test times" << endl;
		cin >> times;
		vector<int> counter = test(times);
		double avg = 0;
		for(int i = 0; i < counter.size(); i++){
			cout << i << " guess correct: " << counter[i] << endl;
			avg += (double)i * (double)counter[i];
		}
		avg /= (double)times;
		cout << "Average guess: " << avg << endl;
	}
}







