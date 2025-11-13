#include "gtest/gtest.h"
#include "marker.h"
#include "head.h"
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <random>
#include <iostream>

const int SLEEP_BASIC_MS = 500;
const int SLEEP_STRESS_MS = 1500;

void initial(int dimension, int numThreads) {
    std::cout << "\nINIT (" << numThreads << " threads, dim=" << dimension << ")\n";

    dim = dimension;
    arr = new int[dim] {};

    InitializeCriticalSection(&cs);

    hStartSignal = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    hContinueSignal = CreateEventA(nullptr, TRUE, FALSE, nullptr);
    hRemoveEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);

    hFinishEvents = new HANDLE[numThreads]{};
    for (int i = 0; i < numThreads; i++)
        hFinishEvents[i] = CreateEventA(nullptr, TRUE, FALSE, nullptr);
}

void cleanup(int numThreads) {
    if (arr) {
        delete[] arr;
        arr = nullptr;
    }

    if (hFinishEvents) {
        for (int i = 0; i < numThreads; i++) {
            if (hFinishEvents[i])
                CloseHandle(hFinishEvents[i]);
        }
        delete[] hFinishEvents;
        hFinishEvents = nullptr;
    }

    if (hStartSignal) { CloseHandle(hStartSignal); hStartSignal = nullptr; }
    if (hContinueSignal) { CloseHandle(hContinueSignal); hContinueSignal = nullptr; }
    if (hRemoveEvent) { CloseHandle(hRemoveEvent); hRemoveEvent = nullptr; }

    DeleteCriticalSection(&cs);
}

std::vector<HANDLE> createMarkerThreads(int numThreads) {
    std::vector<HANDLE> handles(numThreads, nullptr);
    std::vector<DWORD> ids(numThreads, 0);

    for (int i = 0; i < numThreads; i++) {
        handles[i] = CreateThread(nullptr, 0, marker, reinterpret_cast<LPVOID>(i + 1), 0, &ids[i]);
        if (!handles[i])
            throw std::runtime_error("Failed to create marker thread");
        std::cout << "Thread " << (i + 1) << " created.\n";
    }
    return handles;
}

void removeThreads(std::vector<HANDLE>& handles) {
    for (size_t i = 0; i < handles.size(); i++) {
        if (!handles[i]) continue;

        std::cout << "Waiting for thread " << (i + 1) << " to finish marking...\n";
        WaitForSingleObject(hFinishEvents[i], INFINITE);

        rem = static_cast<int>(i + 1);
        std::cout << "Removing thread " << rem << "\n";
        SetEvent(hRemoveEvent);

        WaitForSingleObject(handles[i], INFINITE);
        CloseHandle(handles[i]);
        handles[i] = nullptr;

        ResetEvent(hRemoveEvent);
        SetEvent(hContinueSignal);
        ResetEvent(hFinishEvents[i]);
    }
}

void removeThreadsRandom(std::vector<HANDLE>& handles) {
    std::vector<int> order(handles.size());
    std::iota(order.begin(), order.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(order.begin(), order.end(), g);

    for (int idx : order) {
        if (!handles[idx]) continue;

        std::cout << "Randomly removing thread " << (idx + 1) << "\n";
        WaitForSingleObject(hFinishEvents[idx], INFINITE);

        rem = idx + 1;
        SetEvent(hRemoveEvent);

        WaitForSingleObject(handles[idx], INFINITE);
        CloseHandle(handles[idx]);
        handles[idx] = nullptr;

        ResetEvent(hRemoveEvent);
        SetEvent(hContinueSignal);
        ResetEvent(hFinishEvents[idx]);
    }
}

TEST(MarkerTests, Marking) {
    constexpr int NUM_THREADS = 3;
    constexpr int ARRAY_SIZE = 10;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_BASIC_MS));

    int countMarked = 0;
    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        if (arr[i] != 0) countMarked++;
    LeaveCriticalSection(&cs);

    EXPECT_GT(countMarked, 0);

    removeThreads(handles);
    cleanup(NUM_THREADS);
}

TEST(MarkerTests, SingleThreadRemoval) {
    constexpr int NUM_THREADS = 2;
    constexpr int ARRAY_SIZE = 5;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_BASIC_MS));

    rem = 1;
    SetEvent(hRemoveEvent);
    WaitForSingleObject(handles[0], INFINITE);
    CloseHandle(handles[0]);
    handles[0] = nullptr;
    ResetEvent(hRemoveEvent);
    SetEvent(hContinueSignal);

    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_BASIC_MS));

    int countMarked = 0;
    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        if (arr[i] != 0) countMarked++;
    LeaveCriticalSection(&cs);

    EXPECT_GT(countMarked, 0);

    rem = 2;
    SetEvent(hRemoveEvent);
    WaitForSingleObject(handles[1], INFINITE);
    CloseHandle(handles[1]);
    handles[1] = nullptr;
    ResetEvent(hRemoveEvent);

    cleanup(NUM_THREADS);
}

TEST(MarkerTests, MultipleRemovals) {
    constexpr int NUM_THREADS = 3;
    constexpr int ARRAY_SIZE = 10;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_BASIC_MS));

    removeThreads(handles);

    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(arr[i], 0);
    LeaveCriticalSection(&cs);

    cleanup(NUM_THREADS);
}

TEST(MarkerTests, StressTest20x50) {
    constexpr int NUM_THREADS = 20;
    constexpr int ARRAY_SIZE = 50;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_STRESS_MS));

    removeThreads(handles);

    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(arr[i], 0);
    LeaveCriticalSection(&cs);

    cleanup(NUM_THREADS);
}

TEST(MarkerTests, StressTest50x100) {
    constexpr int NUM_THREADS = 50;
    constexpr int ARRAY_SIZE = 100;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_STRESS_MS));

    removeThreadsRandom(handles);

    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        EXPECT_EQ(arr[i], 0);
    LeaveCriticalSection(&cs);

    cleanup(NUM_THREADS);
}

TEST(MarkerTests, SingleThread) {
    const int NUM_THREADS = 3;
    const int ARRAY_SIZE = 10;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);
    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    rem = 2;
    SetEvent(hRemoveEvent);
    WaitForSingleObject(handles[1], INFINITE);
    CloseHandle(handles[1]);
    handles[1] = nullptr;
    ResetEvent(hRemoveEvent);
    SetEvent(hContinueSignal);

    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        EXPECT_NE(arr[i], 2) << "Marker 2 was not fully cleared at index " << i;
    }
    LeaveCriticalSection(&cs);

    removeThreads(handles);
    cleanup(NUM_THREADS);
}

TEST(MarkerTests, SmallArrayManyThreads) {
    const int NUM_THREADS = 4;
    const int ARRAY_SIZE = 2;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles = createMarkerThreads(NUM_THREADS);

    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int marked = 0;
    EnterCriticalSection(&cs);
    for (int i = 0; i < ARRAY_SIZE; i++)
        if (arr[i] != 0) marked++;
    LeaveCriticalSection(&cs);
    EXPECT_GT(marked, 0);

    removeThreads(handles);
    cleanup(NUM_THREADS);
}

TEST(MarkerTests, Reinit) {
    const int NUM_THREADS = 3;
    const int ARRAY_SIZE = 6;

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles1 = createMarkerThreads(NUM_THREADS);
    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    removeThreads(handles1);
    cleanup(NUM_THREADS);

    initial(ARRAY_SIZE, NUM_THREADS);
    auto handles2 = createMarkerThreads(NUM_THREADS);
    SetEvent(hStartSignal);
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    removeThreads(handles2);
    cleanup(NUM_THREADS);

    SUCCEED() << "Reinitialization successful, no crashes detected.";
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "=== RUNNING TESTS ===\n";
    int result = RUN_ALL_TESTS();
    std::cout << "=== ALL TESTS DONE ===\n";
    return result;
}
