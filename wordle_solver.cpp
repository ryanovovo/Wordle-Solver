#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;


inline int diff_two_strings(const string &lhs, const string &rhs){
	int res = 0;
	int pow = 1;
	vector<int> is_used_lhs(5, 0);
	vector<int> is_used_rhs(5, 0);
	for(int i = 0; i < 5; i++){
		if(lhs[i] == rhs[i]){
			res += pow * 2;
			is_used_lhs[i] = 1;
			is_used_rhs[i] = 1;
		}
		pow *= 3;
	}
	for(int i = 0; i < 5; i++){
		pow = 1;
		if(is_used_lhs[i] == 1){
			continue;
		}
		for(int j = 0; j < 5; j++){
			if(lhs[i] == rhs[j] && is_used_rhs[j] != 1){
				res += pow;
				is_used_lhs[i] = 1;
				is_used_rhs[j] = 1;
				break;
			}
			pow *= 3;
		}
	}
	return res;
}

inline double standard_deviation(const vector<int> &v){
	int n = v.size();
	double avg = 0.0;
	double sum = 0.0;
	for(int i = 0; i < n; i++){
		sum += (double)v[i];
	}
	avg = sum / (double)n;
	double stdev = 0.0;
	for(int i = 0; i < n; i++){
		stdev += (avg-(double)v[i]) * (avg-(double)v[i]);
	}
	stdev /= (double)n;
	return stdev;
}

vector<vector<int>> all_words_dist(vector<string> &words){
	int n = words.size();
	vector<vector<int>> res(n, vector<int>(242, 0));
	for(int i = 0; i < n; i++){
		/*
		if(i % 100 == 0){
			cout << (double)i /  (double)n * 100 << '%' << endl;
		}
		*/
		for(int j = 0; j < n; j++){
			int diff = diff_two_strings(words[i], words[j]);
			res[i][diff]++;
		}
	}
	return res;
}

int guess(vector<string> &allPossibleWords, vector<vector<int>> &words_dist){
  	double min_stdev = 99999.0;
  	int guess_idx = 0;
  	for(unsigned int i = 0; i < allPossibleWords.size(); i++){
  		double stdev = standard_deviation(words_dist[i]);
  		if(stdev < min_stdev){
  			min_stdev = stdev;
  			guess_idx = i;
  		}
  	}
  	vector<string> tmp;
  	string raw_result;
  	cout << "best guess :" << allPossibleWords[guess_idx] << endl;
  	cout << "input result" << endl;
  	cin >> raw_result;
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
  	for(auto word : allPossibleWords){
  		if(diff_two_strings(word, allPossibleWords[guess_idx]) == result){
  			tmp.push_back(word);
  		}
  	}
  	allPossibleWords.clear();
  	allPossibleWords = tmp;
  	words_dist = all_words_dist(allPossibleWords);
  	if(result == 242){
  		cout << "Congratulations!" << endl;
  		return 0;
  	}
  	return 1;
}


void init(vector<vector<int>> &words_dist){
	// vector<vector<int>> words_dist(allPossibleWords.size(), vector<int>(242));
	ifstream dists;
	dists.open("/Users/ryanovovo/Documents/wordle-solver/all_words_dist.txt");
	if(dists.is_open()){
		/*
		for(int i = 0; i < words_dist.size(); i++){
			for(int j = 0; j < words_dist[i].size(); j++){
				dists << words_dist[i][j] << ' ';
			}
			dists << "\n";
		}
		*/
		string line;
		for(int i = 0; getline(dists, line); i++){
			string tmp;
			int it = 0;
			for(auto ch : line){
				if(ch == ' '){
					words_dist[i][it] = stoi(tmp);
					it++;
					tmp.clear();
				}
				else{
					tmp.push_back(ch);
				}
			}
		}
	}
	dists.close();
	return;
}

int main () {
	
 	string line;
 	vector<string> allPossibleWords;
  	ifstream words ("/Users/ryanovovo/Documents/wordle-solver/words.txt");
  	if(words.is_open()){
  		while(getline(words, line)){
  			allPossibleWords.push_back(line);
  		}
  		words.close();
  	}
  	else{
  		cout << "Unable to open words!" << endl;
  	}
  	cout << "start" << endl;
  	auto allPossibleWords_cpy = allPossibleWords;
  	while(true){
  		vector<vector<int>> words_dist(allPossibleWords.size(), vector<int>(242));
  		allPossibleWords_cpy = allPossibleWords;
  		init(words_dist);
  		while(guess(allPossibleWords_cpy, words_dist));
  	}
  	
}