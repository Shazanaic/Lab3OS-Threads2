#include <iostream>
#include "head.h"
#include "marker.h"

CRITICAL_SECTION cs;
HANDLE hStartSignal, hContinueSignal, hRemoveEvent;
HANDLE* hFinishEvents, *hTerminateSignals;
int* arr;
volatile int dim;
volatile int rem;

int main() {
	int dimTmp;
	std::cout << "Enter the dimension of array: "; std::cin >> dimTmp;
	dim = dimTmp;

	arr = new int[dim] {};

	int n;
	std::cout << "Enter the number of marker threads: "; std::cin >> n;

	InitializeCriticalSection(&cs);

	DWORD* IDMarkrs = new DWORD[n];
	HANDLE* hMarkrs = new HANDLE[n];
	hTerminateSignals = new HANDLE[n];
	hFinishEvents = new HANDLE[n];

	hStartSignal = CreateEventA(NULL, TRUE, FALSE, NULL);
	hContinueSignal = CreateEventA(NULL, TRUE, FALSE, NULL);
	hRemoveEvent = CreateEventA(NULL, TRUE, FALSE, NULL);

	for (int i = 0; i < n; i++) {
		hMarkrs[i] = CreateThread(NULL, 0, marker, reinterpret_cast<LPVOID>(i + 1), 0, &IDMarkrs[i]);
		if (hMarkrs[i] == NULL) {
			std::cerr << "Failed to create marker(" << (i + 1) << ").\n";
			return 1;
		}
		
		hTerminateSignals[i] = CreateEventA(NULL, TRUE, FALSE, NULL);
		hFinishEvents[i] = CreateEventA(NULL, TRUE, FALSE, NULL);
	}

	SetEvent(hStartSignal);

	int iter = n;
	while (iter > 0) {
		WaitForMultipleObjects(n, hFinishEvents, true, INFINITE);
		EnterCriticalSection(&cs);

		std::cout << "Current array: \n";
		for (int i = 0; i < dim; i++) {
			std::cout << arr[i] << " ";
		}
		std::cout << "\n";
		LeaveCriticalSection(&cs);

		int forDelete;
		std::cout << "\nEnter the number of marker thread to delete: "; std::cin >> forDelete;
		if (forDelete < 1 || forDelete > n || hMarkrs[forDelete - 1] == NULL) {
			std::cout << "\nInvalid number. Try again\n";
			SetEvent(hContinueSignal);
			continue;
		}

		rem = forDelete;
		PulseEvent(hRemoveEvent);
		WaitForSingleObject(hMarkrs[forDelete - 1], INFINITE);
		CloseHandle(hMarkrs[forDelete - 1]);
		hMarkrs[forDelete - 1] = NULL;

		iter--;

		EnterCriticalSection(&cs);
		std::cout << "Marker(" << forDelete << ") stopped successfully. Current Array: \n";
		for (int i = 0; i < dim; i++)
			std::cout << arr[i] << " ";
		std::cout << "\n";
		
		LeaveCriticalSection(&cs);
		ResetEvent(hRemoveEvent);
		PulseEvent(hContinueSignal);
	}

	std::cout << "All threads are finished.\nCurrent array: \n";
	for (int i = 0; i < dim; i++)
		std::cout << arr[i] << " ";
	std::cout << "\n";

	for (int i = 0; i < n; i++) {
		if (hMarkrs[i]) CloseHandle(hMarkrs[i]);
		CloseHandle(hTerminateSignals[i]);
		CloseHandle(hFinishEvents[i]);
	}

	CloseHandle(hStartSignal);
	CloseHandle(hContinueSignal);
	CloseHandle(hRemoveEvent);
	DeleteCriticalSection(&cs);

	return 0;
}