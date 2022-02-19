#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <fstream>
#include <type_traits>
#include <thread>
#include <future>
using namespace std;


const int total_words = 12972;
const int total_diffs = 243;

char difficulty; // n = normal mode, h = hard mode

vector<string> all_words(total_words);
vector<vector<int>> all_words_dist(total_words, vector<int>(total_diffs));
vector<vector<int>> all_words_diff(total_words, vector<int>(total_words));
map<string, int> word_idx;
vector<int> possible_ans_idx;


// 計算答案和猜測的diff值

int diff_answer_and_guess(const int &ans_idx, const int &guess_idx){
	return all_words_diff[ans_idx][guess_idx];
}

// 取得字串的index

int get_word_idx(const string &word){
	return word_idx[word];
}

// 計算數據的變異數

double variance(const vector<int> &v){
	int n = v.size();
	double avg = 0.0;
	double sum = 0.0;
	for(int i = 0; i < n; i++){
		sum += (double)v[i];
	}
	avg = sum / (double)n;
	double var = 0.0;
	for(int i = 0; i < n; i++){
		var += (avg-(double)v[i]) * (avg-(double)v[i]) * (double)v[i] / (double)n;
	}
	return var;
}


int get_encoded_result(const string &raw_result){
	int result = 0;
  	int pow = 1;
  	for(int i = 0; i < 5; i++){
  		if(raw_result[i] == 'o'){
  			result += pow*2;
  		}
  		else if(raw_result[i] == '_'){
  			result += pow;
  		}
  		pow *=3;
  	}
  	return result;
}


template <typename T>
int get_answer(const int &best_guess_idx, const T &raw_result){
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



//回傳{最佳猜測結果的idx, 最小變異數}

pair<int, double> get_min_variance(const int &l, const int &r){
	// 初始參數

	double min_variance = 999999999.0;
	int best_guess_idx  = 0;


	// 預處理得知lares為遊戲開始時的最佳猜測

	if(possible_ans_idx.size() == total_words){
		best_guess_idx = 7313; // lares 的 index
		return {best_guess_idx, 0.0};
	}


	//取得範圍內可猜測字對可能答案的變異數，並選擇最小的那個作為最佳猜測

	if(difficulty == 'n'){ // normal mode
		for(unsigned int i = l; i < r; i++){
			vector<int> distribution(total_diffs, 0);
			for(unsigned int j = 0; j < possible_ans_idx.size(); j++){
				distribution[diff_answer_and_guess(possible_ans_idx[j], i)]++;
			}
			double var = variance(distribution);
			if(var < min_variance){
				min_variance = var;
				best_guess_idx = i;
			}
		}
	}
	else{ // hard mode
		for(unsigned int i = l; i < r; i++){
			vector<int> distribution(total_diffs, 0);
			for(unsigned int j = 0; j < possible_ans_idx.size(); j++){
				distribution[diff_answer_and_guess(possible_ans_idx[j], possible_ans_idx[i])]++;
			}
			double var = variance(distribution);
			if(var < min_variance){
				min_variance = var;
				best_guess_idx = possible_ans_idx[i];
			}
		}
	}
	return {best_guess_idx, min_variance};
}


int guess(){ 
	
	int best_guess_idx = 0;
	double min_variance = 99999999.0;


	int total_threads = 4;
	int segment = 0;
	vector<future<pair<int, double>>> futures;


	if(difficulty == 'n'){
		segment = total_words / total_threads;
		futures.push_back(async(get_min_variance, total_words-segment, total_words));
	}
	else{
		segment = possible_ans_idx.size() / total_threads;
		futures.push_back(async(get_min_variance, possible_ans_idx.size()-segment, possible_ans_idx.size()));
	}

	for(int i = 0; i < total_threads-1; i++){
		futures.push_back(async(get_min_variance, i * segment, (i+1) * segment));
	}

	for(auto &f : futures){
		auto res = f.get();
		if(res.second < min_variance){
			min_variance = res.second;
			best_guess_idx = res.first;
		}
	}
	

	// 回傳最佳猜測答案的編號 

	return best_guess_idx;
}



// 清空所有可能的答案回到初始狀態

void clear_possible_ans_idx(){
	possible_ans_idx.clear();
	for(int i = 0; i < total_words; i++){
		possible_ans_idx.push_back(i);
	}
	return;
}


//初始化程式

void init(){

	cout << "Initializing..." << endl;


	// 開啟必要檔案

  	ifstream words ("/Users/ryanovovo/Documents/GitHub/wordle-solver/words.txt");
  	ifstream dist  ("/Users/ryanovovo/Documents/GitHub/wordle-solver/all_words_dist.txt");
  	ifstream diff  ("/Users/ryanovovo/Documents/GitHub/wordle-solver/all_words_diff.txt");

  	//檢查是否成功開啟檔案

  	assert(words.is_open());
  	assert(dist.is_open());
  	assert(diff.is_open());

  	//將檔案內容導入容器中

  	for(int i = 0; i < total_words; i++){
  		possible_ans_idx.push_back(i);
  	}


  	string line;
	for(int i = 0; getline(words, line); i++){
		all_words[i] = line;
		word_idx[line] = i;
	}
	words.close();
  


	for(int i = 0; getline(dist, line); i++){
		string tmp;
		int it = 0;
		for(auto ch : line){
			if(ch == ' '){
				all_words_dist[i][it] = stoi(tmp);
				it++;
				tmp.clear();
			}
			else{
				tmp.push_back(ch);
			}
		}
	}
	dist.close();
 
  
	
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

  	return;
}

int main(){
	init();

	cout << "choose game difficulty" << endl;
	cout << "n = normal mode, h = hard mode" << endl;
	cin >> difficulty;

	for(int i = 0; i < total_words; i++){
		clear_possible_ans_idx();
		while(true){
			int best_guess_idx = guess();
			int raw_result = diff_answer_and_guess(i, best_guess_idx);
			int correct_ans_idx = get_answer(best_guess_idx, raw_result);
			cout << raw_result << endl;
			if(correct_ans_idx > 0 || raw_result == 242){
				cout << "Correst answer: " << all_words.at(correct_ans_idx) << endl;
				break;
			}
		}
	}
}