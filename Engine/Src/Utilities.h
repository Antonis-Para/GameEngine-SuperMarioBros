#pragma once
#include <string>
#include <vector>
#include <math.h>
#include <sstream>
#include <iostream>

using namespace std;

vector <string> splitString(string text, string delimiter);
int customRound(double number);
string standarizeSize(string text, int size);

struct pointShow {
    int points;
    int x;
    int y;
    int final_y;
};