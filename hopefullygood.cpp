#include <iostream>
#include <fstream>
#include <ostream>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include <cstring>
#include <set>
#include <sstream>
#include <stdexcept>

using namespace std;

class node {
public:
	vector<string> current; // Current value as we go through the file
	vector<int> currentint; // Similar items; allows for all types of property values in files to be accepted (dumping ground)
	int featureIndex;// index of the split feature
	double threshold; // initial prediction value
	int label; // leaf node, each individual item on a branch of decision tree
	node* left;
	node* right;
	node() :
		featureIndex(-1), threshold(0.0), label(-1), left(nullptr), right(nullptr) {

	}

};/*
void tree(fstream file1) {
	string line; // string for the while function below to track final line
	while (getline(file1, line)) {

	}

}*/
struct seismic {
	vector<double> features;
	int label;
};
double entropy(const vector<seismic>& data, int index, double split) {
	vector<seismic> left, right;
	for (const auto& dp : data) {
		if (dp.features[index] <= split)
			left.push_back(dp);
		else
			right.push_back(dp);
	}

	// Count yes labels on each side
	auto calcEntropy = [](const vector<seismic>& subset) {
		if (subset.empty()) return 0.0;
		int ones = 0;
		for (const auto& d : subset)
			if (d.label == 1) ones++;
		double p = (double)ones / subset.size();
		if (p == 0 || p == 1) return 0.0;
		return -p * log2(p) - (1 - p) * log2(1 - p);
		};

	double leftEntropy = calcEntropy(left);
	double rightEntropy = calcEntropy(right);

	// Weighted entropy (information gain = parent entropy - this)
	return -((double)left.size() / data.size() * leftEntropy +
		(double)right.size() / data.size() * rightEntropy);
}
pair<int, double> bestSplit(const vector<seismic>& data) {
	int bestfeatureindex = -1;
	double bestthreshold = 0.0;
	double bestinfogain = -numeric_limits<double>::infinity();
	for (size_t i = 0; i < data[0].features.size(); i++) {
		set<double> levels;
		for (const auto& row:data) {
			levels.insert(row.features[i]);
		
		}
		for (const auto& row : levels) {
			double info = entropy(data, i, row);
			if (info > bestinfogain) {
				bestinfogain = info;
				bestfeatureindex = i;
				bestthreshold = row;
			}
		}

	}
	return { bestfeatureindex, bestthreshold };
	

}
class tree {
public:
	tree() :root(nullptr) {

	}
	void train(const vector<seismic>& data, int levels) { // seismic data records features and labels, levels = levels of tree
		root = build(data, levels);
	}
	int predict(const seismic& data) const {
		node* leaf = root;
		while (leaf->left && leaf->right ) {
			if (data.features[leaf->featureIndex] <= leaf->threshold) {
				leaf = leaf->left;
			}
			else {
				leaf = leaf->right;
			}
		}
		return leaf->label;
	}
private:
	node* root;
	node* build(const vector<seismic>& data, int levels) {
		node* newNode = new node();
		if (levels == 0 || data.empty()) {
			newNode->label = mostcommonlabel(data);
			return newNode;
		}
		pair <int, double> best;
		best = bestSplit(data);

		cout << "Split: feature=" << best.first << " threshold=" << best.second << " datasize=" << data.size() << endl;
		if (best.first == -1) {
			newNode->label = mostcommonlabel(data);
			return newNode;
		}
		vector<seismic> leftside;
		vector<seismic> rightside;
		for (const auto& i : data) {
			if (i.features[best.first] <= best.second) {  
				leftside.push_back(i);
			}
			else {
				rightside.push_back(i);
			}
		}
		newNode->featureIndex = best.first;
		newNode->threshold = best.second;
		newNode->left = build(leftside, levels - 1);
		newNode->right = build(rightside, levels - 1);
		return newNode;
	}
	int mostcommonlabel(const vector<seismic>& data) {
		map<int, int> counting;
		for (const auto& i: data) {
			counting[i.label]++;

		}
		int maxcount = 0;
		int mostcommonlabel = -1;
		for (const auto& i: counting) {
			if (i.second > maxcount) {
				maxcount = i.second;
				mostcommonlabel = i.first;

			}
			
		}
		return mostcommonlabel;
	}
};
string trim(const string& str) {
	size_t first = str.find_first_not_of(" \r\t");
	if (first == string::npos) {
		return "";
	}
	size_t last = str.find_last_not_of(" \r\t");
	return str.substr(first, last - first + 1);
}

vector<seismic> readfile(const string& file) {
	ifstream file1(file);
	if (!file1.is_open()) {
		cout << "Failed to open file!" << endl;
		return {};
	}

	// Map text categories to numbers
	map<string, double> encoder = {
		{"Sunny", 0}, {"Overcast", 1}, {"Rain", 2},
		{"Hot", 0}, {"Mild", 1}, {"Cool", 2},
		{"High", 0}, {"Normal", 1},
		{"Weak", 0}, {"Strong", 1},
		{"No", 0}, {"Yes", 1}
	};

	vector<seismic> data;
	string line;

	getline(file1, line); // skip header

	while (getline(file1, line)) {
		seismic seisdata;
		string value;
		istringstream iss(line);
		bool first = true;
		while (getline(iss, value, ',')) {
			value = trim(value);
			if (first) { first = false; continue; } // skip the row number column
			if (encoder.count(value)) {
				seisdata.features.push_back(encoder[value]);
			}
			else {
				try {
					seisdata.features.push_back(stod(value));
				}
				catch (...) {
					seisdata.features.push_back(0.0);
				}
			}
		}
		// Last feature is the label (Yes/No), pull it out
		seisdata.label = static_cast<int>(seisdata.features.back());
		seisdata.features.pop_back();
		data.push_back(seisdata);
	}
	return data;
}
int main() {
	
	/*fstream file1;
	file1.open("enter name.txt", ios::in);
	if (file1.is_open()) {
		tree thing;
		thing.train(file1, 3);
		seismic sample = file1[0];
		int prediction = thing.predict(sample);
		cout << prediction << endl;
	}
	file1.close();*/
	vector<seismic> data = readfile("classification.csv");
	tree tree;
	tree.train(data, 3);
	for (int i = 0; i < data.size(); i++) {
		int prediction = tree.predict(data[i]);
		cout << "Sample " << i + 1 << ": predicted=" << prediction << " actual=" << data[i].label << endl;
	}
	return 0;

}
