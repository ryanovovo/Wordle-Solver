#ifndef __WORDLE_HPP
#define __WORDLE_HPP

#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <type_traits>
#include <ctime>
#include <random>
#include <chrono>
#include <future>
#include <thread>
#include <algorithm>
#include <string>
#include "progressbar.hpp"


using namespace std;

class Wordle{
public:
	void solve();
	void print_test_result(int);
	void change_difficulty(char);
	void change_mode(char);

	Wordle(string);
	Wordle(string, char, char);

private:

	// math function
	// ----------------------------------------------------------------
	double variance(const vector<int> &);
	vector<int> discretize(vector<int> &data);
	// ----------------------------------------------------------------

	// solve function
	// ----------------------------------------------------------------
	vector<int> test(int);
	void init_possible_ans_idx(vector<int> &);
	int diff(const int &, const int &);
	int encode(const string &);
	template <typename T>
	int filter_answer(const int &, const T &, vector<int> &);
	int get_random_guess_times();
	int guess(vector<int> &);
	// ----------------------------------------------------------------
	

	// basic settings
	// ----------------------------------------------------------------
	string directory;

	int total_words;
	int total_diffs;
	int total_threads;

	char difficulty;
	char mode;

	vector<string> all_words;
	vector<vector<int>> all_words_diff;

	void load_line(const string&, const int&);
	void load_file();

	// ----------------------------------------------------------------
};


// 初始化環境
inline Wordle::Wordle(string dir){
	directory = dir;
	total_threads = max(1, (int)std::thread::hardware_concurrency());
	total_diffs = 243;
	total_words = 12972;
	all_words = vector<string>(total_words);
	all_words_diff = vector(total_words, vector<int>(total_words, 0));
	load_file();
	change_difficulty('n');
	change_mode('s');

}


// 初始化環境
inline Wordle::Wordle(string dir, char Difficulty, char Mode){
	directory = dir;
	total_threads = max(1, (int)std::thread::hardware_concurrency());
	total_diffs = 243;
	total_words = 12972;
	all_words = vector<string>(total_words);
	all_words_diff = vector(total_words, vector<int>(total_words, 0));
	load_file();
	change_difficulty(Difficulty);
	change_mode(Mode);
}


// 計算陣列的變異數
inline double Wordle::variance(const vector<int> &data){
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


// 將陣列離散化
inline vector<int> Wordle::discretize(vector<int> &data){
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


// 將可能的儲存答案index的陣列初始化
inline void Wordle::init_possible_ans_idx(vector<int> &possible_ans_idx){
	possible_ans_idx.clear();
	for(int i = 0; i < total_words; i++){
		possible_ans_idx.push_back(i);
	}
	return;
}


// 計算正確答案和猜測答案的diff值
inline int Wordle::diff(const int &ans_idx, const int &guess_idx){
	return all_words_diff[ans_idx][guess_idx];
}


// 將wordle回傳的結果編碼
inline int Wordle::encode(const string &raw_result){
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


// 篩出可能的答案
template <typename T>
inline int Wordle::filter_answer(const int &best_guess_idx, const T &raw_result, vector<int> &possible_ans_idx){
	// 將結果轉換為編碼後的格式
	int result = 0;
	if constexpr(is_same_v<T, int>){
		result = raw_result;
	}
	else if constexpr(is_same_v<T, string>){
		result = encode(raw_result);
	}
	else{
		assert(false);
	}
	

	// 將結果與可能的答案比對
	vector<int> new_possible_ans_idx;
	for(unsigned int i = 0; i < possible_ans_idx.size(); i++){
		if(diff(possible_ans_idx[i], best_guess_idx) == result){
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


// 猜測可能答案
inline int Wordle::guess(vector<int> &possible_ans_idx){ 
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
				diff_result.push_back(diff(possible_ans_idx[j], i));
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
				diff_result.push_back(diff(possible_ans_idx[j], possible_ans_idx[i]));
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


// 取得單次隨機猜測所需的次數
inline int Wordle::get_random_guess_times(){
	vector<int> possible_ans_idx;
	init_possible_ans_idx(possible_ans_idx);
	int guess_times = 1;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 gen(seed);
	uniform_int_distribution<> distrib(0, total_words-1);
	int answer = distrib(gen);
	for(int i = 0; i < 2000; i++){
		guess_times++;
		int best_guess_idx = guess(possible_ans_idx);
		int raw_result = diff(answer, best_guess_idx);
		int correct_ans_idx = filter_answer(best_guess_idx, raw_result, possible_ans_idx);
		if(correct_ans_idx > 0 || raw_result == 242){
			// cout << "Answer: " << all_words.at(correct_ans_idx) << " guess " << guess_times << " times" << endl;
			return guess_times;
		}
	}
	return -1;
}


// 測試指定的猜測次數，並回傳統計次數
inline vector<int> Wordle::test(int test_times){
	vector<int> counter;
	progressbar bar(test_times);
	vector<future<int>> threads(total_threads);
	for(int i = 0; i < test_times; i++){
		bar.update();
		if(i < total_threads){
			threads[i] = async(launch::async, &Wordle::get_random_guess_times, this);
		}
		else{
			int guess_times = threads[i%Wordle::total_threads].get();
			threads[i%total_threads] = async(launch::async, &Wordle::get_random_guess_times, this);
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	
	cout << endl;
	if(test_times > total_threads){
		for(int i = 0; i < total_threads; i++){
			int guess_times = threads[i%Wordle::total_threads].get();
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	else{
		for(int i = 0; i < test_times; i++){
			int guess_times = threads[i%Wordle::total_threads].get();
			if(guess_times >= counter.size()){
				counter.resize(guess_times+1);
			}
			counter[guess_times]++;
		}
	}
	
	return counter;
}


// 輸出測試結果
inline void Wordle::print_test_result(int test_times){
	if(test_times == 0){
		return;
	}

	cout << "Testing..." << endl;
	double avg = 0.0;

	auto start_time = chrono::high_resolution_clock::now();
	vector<int> counter = test(test_times);
	
	auto end_time = chrono::high_resolution_clock::now();
	for(unsigned int i = 0; i < counter.size(); i++){
		cout << i << " guess correct: " << counter[i] << endl;
		avg += (double)i * (double)counter[i];
	}

	avg /= (double)test_times;
	cout << "Average guess: " << avg << endl;

	chrono::duration<double, milli> ms_double = end_time - start_time;
	cout << "Execution time: " << ms_double.count() << "ms" << endl;
	cout << "Finished testing" << endl;
	return;
}


// 解wordle
inline void Wordle::solve(){
	vector<int> possible_ans_idx;
	init_possible_ans_idx(possible_ans_idx);
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
	return;
}


// 更改困難度
inline void Wordle::change_difficulty(char Difficulty){
	difficulty = Difficulty;
	return;
}

// 更改模式
inline void Wordle::change_mode(char Mode){
	mode = Mode;
	return;
}


// 將讀取到的字串寫入至容器中
inline void Wordle::load_line(const string &line, const int &row){
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


// 將必要檔案讀入容器中
inline void Wordle::load_file(){
	cout << "Total Threads: " << total_threads << endl;

	auto start_time = chrono::high_resolution_clock::now();

	cout << "Initializing..." << endl;

	progressbar bar(total_words);

	vector<thread> threads(total_threads);

	string words_path = directory;
	string all_words_diff_path = directory;

	words_path.append("words.txt");
	all_words_diff_path.append("all_words_diff.txt");

	
	// 開啟必要檔案
  	ifstream words (words_path);
  	ifstream diff  (all_words_diff_path);


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
		bar.update();
		if(i < total_threads){
			threads[i] = thread(&Wordle::load_line, this, line, i);
		}
		else{
			threads[i%total_threads].join();
			threads[i%total_threads] = thread(&Wordle::load_line, this, line, i);
		}
	}
	for(int i = 0; i < total_threads; i++){
		threads[i].join();
	}
	cout << endl;
	diff.close();

	auto end_time = chrono::high_resolution_clock::now();
	chrono::duration<double, milli> ms_double = end_time - start_time;
	cout << "Load time: " << ms_double.count() << "ms" << endl;
	cout << "Finished Initializing" << endl;
	return;
}

#endif