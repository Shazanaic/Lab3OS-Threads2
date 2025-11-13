#include <iostream>
#include "head.h"
#include "marker.h"

CRITICAL_SECTION cs;
HANDLE hStartSignal = nullptr, hContinueSignal = nullptr, hRemoveEvent = nullptr;
HANDLE* hFinishEvents = nullptr;
HANDLE* hTerminateSignals = nullptr;
HANDLE* hMarkrs = nullptr;
int* arr = nullptr;
volatile int dim = 0;
volatile int rem = 0;

int main() {
    int dimTmp;
    std::cout << "Enter the dimension of array: ";
    std::cin >> dimTmp;
    dim = dimTmp;

    arr = new int[dim] {};

    int n;
    std::cout << "Enter the number of marker threads: ";
    std::cin >> n;

    InitializeCriticalSection(&cs);

    DWORD* IDMarkrs = new DWORD[n];
    hMarkrs = new HANDLE[n]{};
    hTerminateSignals = new HANDLE[n]{};
    hFinishEvents = new HANDLE[n]{};

    if (!(hStartSignal = CreateEventA(NULL, TRUE, FALSE, NULL)) ||
        !(hContinueSignal = CreateEventA(NULL, TRUE, FALSE, NULL)) ||
        !(hRemoveEvent = CreateEventA(NULL, TRUE, FALSE, NULL))) {
        std::cerr << "Failed to create control events.\n";
        return 1;
    }

    for (int i = 0; i < n; i++) {
        hMarkrs[i] = CreateThread(NULL, 0, marker, reinterpret_cast<LPVOID>(i + 1), 0, &IDMarkrs[i]);
        if (hMarkrs[i] == NULL) {
            std::cerr << "Failed to create marker(" << (i + 1) << ").\n";
            return 1;
        }

        hTerminateSignals[i] = CreateEventA(NULL, TRUE, FALSE, NULL);
        hFinishEvents[i] = CreateEventA(NULL, TRUE, FALSE, NULL);

        if (!hTerminateSignals[i] || !hFinishEvents[i]) {
            std::cerr << "Failed to create event for marker(" << (i + 1) << ").\n";
            return 1;
        }
    }

    SetEvent(hStartSignal);

    int iter = n;
    while (iter > 0) {
        WaitForMultipleObjects(n, hFinishEvents, TRUE, INFINITE);
        EnterCriticalSection(&cs);

        std::cout << "Current array:\n";
        for (int i = 0; i < dim; i++)
            std::cout << arr[i] << " ";
        std::cout << "\n";
        LeaveCriticalSection(&cs);

        int forDelete;
        std::cout << "\nEnter the number of marker thread to delete: ";
        std::cin >> forDelete;

        if (forDelete < 1 || forDelete > n || !hMarkrs[forDelete - 1]) {
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
        std::cout << "Marker(" << forDelete << ") stopped successfully. Current Array:\n";
        for (int i = 0; i < dim; i++)
            std::cout << arr[i] << " ";
        std::cout << "\n";
        LeaveCriticalSection(&cs);

        ResetEvent(hRemoveEvent);
        PulseEvent(hContinueSignal);
    }

    std::cout << "All threads are finished.\nCurrent array:\n";
    for (int i = 0; i < dim; i++)
        std::cout << arr[i] << " ";
    std::cout << "\n";

    for (int i = 0; i < n; i++) {
        if (hMarkrs[i]) CloseHandle(hMarkrs[i]);
        if (hTerminateSignals[i]) CloseHandle(hTerminateSignals[i]);
        if (hFinishEvents[i]) CloseHandle(hFinishEvents[i]);
    }

    if (hStartSignal) CloseHandle(hStartSignal);
    if (hContinueSignal) CloseHandle(hContinueSignal);
    if (hRemoveEvent) CloseHandle(hRemoveEvent);

    DeleteCriticalSection(&cs);
    delete[] arr;
    delete[] IDMarkrs;
    delete[] hMarkrs;
    delete[] hTerminateSignals;
    delete[] hFinishEvents;

    return 0;
}
