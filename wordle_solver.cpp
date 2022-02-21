#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <fstream>
#include <type_traits>
#include <ctime>
#include <random>
#include <chrono>
#include <future>
#include <thread>
using namespace std;


// 程式所需變數
const int total_words = 12972; // 總字數
const int total_diffs = 243; // 兩單字經diff運算後可得的最大值+1
const int total_threads = 8; // 總線程數

char difficulty; // n = normal difficulty, h = hard difficulty
char mode; // s = solve mode, t = test mode

vector<string> all_words(total_words); // 所有可以猜的單字
vector<vector<int>> all_words_diff(total_words, vector<int>(total_words, 0)); // 所有單字互相diff後的值
// vector<int> possible_ans_idx; // 可能答案的index


// 清空所有可以猜的單字index回到初始狀態
void clear_possible_ans_idx(vector<int> &possible_ans_idx){
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
	vector<int> mp(total_diffs, -1);
	int idx = 0;
	for(int i = 0; i < data_size; i++){
		if(mp[data[i]] == -1){
			mp[data[i]] = idx;
			discretize_data[idx]++;
			idx++;
		}
		else{
			discretize_data[mp[data[i]]]++;
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
int filter_answer(const int &best_guess_idx, const T &raw_result, vector<int> &possible_ans_idx){
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
int guess(vector<int> &possible_ans_idx){ 
	// 初始參數
	double min_variance = 999999999.0;
	int best_guess_idx  = 0;
	

	// 預處理得知lares為遊戲開始時的最佳猜測
	if(possible_ans_idx.size() == total_words){
		best_guess_idx = 7313; // lares 的 index
		return best_guess_idx;
	}
	

	//取得所有可猜測字對可能答案的變異數，並選擇變異數最小的作為最佳猜測
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


// 將讀取到的字串寫入至容器中
void load_line(const string &line, const int &row){
	string tmp;
	int column = 0;
	for(auto ch : line){
		if(ch == ' '){
			all_words_diff[row][column] = stoi(tmp);
			column++;
			tmp.clear();
		}
		else{
			tmp.push_back(ch);
		}
	}
	return;
}



//初始化程式
void init(){
	cout << "Initializing..." << endl;

	vector<thread> threads(8);


	// 開啟必要檔案
  	ifstream words ("/Users/ryanovovo/Documents/GitHub/wordle-solver/words.txt");
  	ifstream diff  ("/Users/ryanovovo/Documents/GitHub/wordle-solver/all_words_diff.txt");


  	//檢查是否成功開啟檔案
  	assert(words.is_open());
  	assert(diff.is_open());

  	string line; // 暫存讀入的文字

  	//將檔案內容導入容器中

  	// 將可以猜的單字導入到容器中
	for(int i = 0; getline(words, line); i++){
		all_words[i] = line;
	}
	words.close();
  	
	// 將所有單字的diff值導入容器中
	for(int i = 0; getline(diff, line); i++){
		if(i < total_threads){
			threads[i] = thread(load_line, line, i);
		}
		else{
			threads[i%total_threads].join();
			threads[i%total_threads] = thread(load_line, line, i);
		}
	}
	for(int i = 0; i < total_threads; i++){
		threads[i].join();
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



int get_random_guess_times(){
	vector<int> possible_ans_idx;
	clear_possible_ans_idx(possible_ans_idx);
	int guess_times = 1;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 gen(seed);
	uniform_int_distribution<> distrib(0, total_words-1);
	int answer = distrib(gen);
	for(int i = 0; i < 2000; i++){
		guess_times++;
		int best_guess_idx = guess(possible_ans_idx);
		int raw_result = diff_answer_and_guess(answer, best_guess_idx);
		int correct_ans_idx = filter_answer(best_guess_idx, raw_result, possible_ans_idx);
		if(correct_ans_idx > 0 || raw_result == 242){
			cout << "Answer: " << all_words.at(correct_ans_idx) << " guess " << guess_times << " times" << endl;
			return guess_times;
		}
	}
	return -1;
}


// 測試指定的猜測次數，並回傳統計次數
vector<int> test(int test_times){
	vector<int> counter;

	vector<future<int>> threads(total_threads);
	for(int i = 0; i < test_times; i++){
		if(i < total_threads){
			threads[i] = async(get_random_guess_times);
		}
		else{
			int guess_times = threads[i%total_threads].get();
			threads[i%total_threads] = async(get_random_guess_times);
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	
	if(test_times > total_threads){
		for(int i = 0; i < total_threads; i++){
			int guess_times = threads[i%total_threads].get();
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	else{
		for(int i = 0; i < test_times; i++){
			int guess_times = threads[i%total_threads].get();
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	
	return counter;
}


// 開始解wordle
void solve(){
	vector<int> possible_ans_idx;
	while(true){
		clear_possible_ans_idx(possible_ans_idx);
		while(true){
			int best_guess_idx = guess(possible_ans_idx);
			cout << "best guess: " << all_words.at(best_guess_idx) << endl;
			string raw_result;
			cin >> raw_result;
			if(raw_result == "stop"){
				return;
			}
			int correct_ans_idx = filter_answer(best_guess_idx, raw_result, possible_ans_idx);
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
		while(true){
			int test_times = 0;
			double avg = 0.0;
			
			cout << "Input test times" << endl;
			cin >> test_times;

			vector<int> counter = test(test_times);
			
			for(unsigned int i = 0; i < counter.size(); i++){
				cout << i << " guess correct: " << counter[i] << endl;
				avg += (double)i * (double)counter[i];
			}

			avg /= (double)test_times;
			cout << "Average guess: " << avg << endl;
		}
		
	}
}

