#include "marker.h"
const int SleepTime = 5;

DWORD WINAPI marker(LPVOID number) {
	WaitForSingleObject(hStartSignal, INFINITE);
	int n = reinterpret_cast<int>(number);
	srand(n);

	int mark = 0;
	int* marks = new int[dim] {};

	while (true) {
		int tmp = rand();
		int idx = tmp % dim;

		EnterCriticalSection(&cs);
		if (arr[idx] == 0) {
			Sleep(SleepTime);
			arr[idx] = n;
			marks[idx] = 1;
			mark++;
			Sleep(SleepTime);
			LeaveCriticalSection(&cs);
		}
		else {
			std::cout << "Number of thread: " << n << "\n";
			std::cout << "Number of marked elem-s: " << mark << "\n";
			std::cout << "Index of unmarkable element: " << idx << "\n\n";
			LeaveCriticalSection(&cs);

			SetEvent(hFinishEvents[n - 1]);
			WaitForSingleObject(hRemoveEvent, INFINITE);

			if (rem == n) {
				EnterCriticalSection(&cs);
				for (int i = 0; i < dim; i++) {
					if (marks[i] == 1)
						arr[i] = 0;
				}
				LeaveCriticalSection(&cs);
				delete[] marks;
				SetEvent(hFinishEvents[n - 1]);
				return 0;
			}
			else {
				ResetEvent(hFinishEvents[n - 1]);
				WaitForSingleObject(hContinueSignal, INFINITE);
			}
		}
	}
}