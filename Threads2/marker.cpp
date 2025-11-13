#include "marker.h"
#include <vector>
#include <iostream>
#include <windows.h>

const int SLEEP_TIME_MS = 5;

class Marker {
public:
    Marker(int id)
        : n(id),
        markCount(0),
        marks(dim, 0)
    {
        srand(n);
    }

    DWORD run() {
        WaitForSingleObject(hStartSignal, INFINITE);

        while (true) {
            int tmp = rand();
            int idx = tmp % dim;

            EnterCriticalSection(&cs);
            if (arr[idx] == 0) {
                Sleep(SLEEP_TIME_MS);
                arr[idx] = n;
                marks[idx] = 1;
                markCount++;
                Sleep(SLEEP_TIME_MS);
                LeaveCriticalSection(&cs);
            }
            else {
                report(idx);
                LeaveCriticalSection(&cs);

                SetEvent(hFinishEvents[n - 1]);
                WaitForSingleObject(hRemoveEvent, INFINITE);

                if (rem == n) {
                    cleanup();
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

private:
    int n;
    int markCount;
    std::vector<int> marks;

    void report(int idx) const {
        std::cout << "Number of thread: " << n << "\n";
        std::cout << "Number of marked elem-s: " << markCount << "\n";
        std::cout << "Index of unmarkable element: " << idx << "\n\n";
    }

    void cleanup() {
        EnterCriticalSection(&cs);
        for (int i = 0; i < dim; ++i) {
            if (marks[i] == 1)
                arr[i] = 0;
        }
        LeaveCriticalSection(&cs);
    }
};

DWORD WINAPI marker(LPVOID number) {
    int n = reinterpret_cast<int>(number);
    Marker worker(n);
    return worker.run();
}
