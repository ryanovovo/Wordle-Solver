#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <fstream>
using namespace std;


const int total_words = 12972;
const int total_diffs = 243;

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

// 選擇變異數最小的單字作為最佳候選單字 
// true = 已找出正確答案, false = 仍要繼續找

bool guess(){
	// 初始參數

	double min_variance = 999999999.0;
	int best_guess_idx  = 0;

	
	// 預處理得知lares為遊戲開始時的最佳猜測
	if(possible_ans_idx.size() == total_words){
		best_guess_idx = 7313; // lares 的 index
	}
	else{
		//取得所有可猜測字對可能答案的變異數，並選擇最小的那個作為最佳猜測

		for(unsigned int i = 0; i < total_words; i++){
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

	
	// 印出最佳猜測的單字到螢幕

	cout << "Best guess: " << all_words.at(best_guess_idx) << endl;


	// 取得結果

	string raw_result;
	cin >> raw_result;
	int result = get_encoded_result(raw_result);


	// 將結果與可能的答案比對

	vector<int> new_possible_ans_idx;
	for(unsigned int i = 0; i < possible_ans_idx.size(); i++){
		if(diff_answer_and_guess(possible_ans_idx[i], best_guess_idx) == result){
			new_possible_ans_idx.push_back(possible_ans_idx[i]);
		}
	}
	possible_ans_idx.clear();
	possible_ans_idx = new_possible_ans_idx;
	

	//輸出最佳猜測

	if(possible_ans_idx.size() == 1){
		cout << "Correct answer: " << all_words.at(possible_ans_idx.at(0)) << endl;
		return true;
	}
	else{
		return false;
	}
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

  	ifstream words ("/Users/ryanovovo/Documents/wordle-solver/words.txt");
  	ifstream dist  ("/Users/ryanovovo/Documents/wordle-solver/all_words_dist.txt");
  	ifstream diff  ("/Users/ryanovovo/Documents/wordle-solver/all_words_diff.txt");

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
	while(true){
		if(guess()){
			clear_possible_ans_idx();
		}
	}
}