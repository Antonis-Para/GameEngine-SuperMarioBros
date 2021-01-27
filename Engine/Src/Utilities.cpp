#include "Utilities.h"

using namespace std;

vector <string> splitString(string text, string delimiter) {
	string token = "";
	int pos = 0;
	vector<string> results;

	while ((pos = text.find(delimiter)) != string::npos) {
		token = text.substr(0, pos);
		results.push_back(token);
		text.erase(0, pos + delimiter.length());
	}
	results.push_back(text);

	return results;
}

int customRound(double number) {
	int base = floor(number);

	if (number - base >= 0.4f)
		return ceil(number);
	else
		return floor(number);
}