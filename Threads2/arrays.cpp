#include "arrays.h"
#include <algorithm>

void initArr(std::vector<int>& arr) {
	std::fill(arr.begin(), arr.end(), 0);
}

bool tryMarkElem(std::vector<int>& arr, int id) {
	for (int& x : arr) {
		if (x == id) x = 0;
	}
}

