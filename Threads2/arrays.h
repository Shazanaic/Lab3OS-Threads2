#pragma once
#include <vector>

void initArr(std::vector<int>& arr);
bool tryMarkElem(std::vector<int>& arr, size_t index, int id);
void clearMarks(std::vector<int>& arr, int id);
int countMarks(const std::vector<int>& arr, int id);