#pragma once
#include <windows.h>
#include <iostream>

extern CRITICAL_SECTION cs;
extern HANDLE hStartSignal, hContinueSignal, hRemoveEvent;
extern HANDLE* hFinishEvents;
extern HANDLE* hTerminateSignals;
extern int* arr;
extern volatile int dim;
extern volatile int rem;

DWORD WINAPI marker(LPVOID number);
