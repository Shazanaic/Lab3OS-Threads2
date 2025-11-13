#pragma once
#include <windows.h>
#include <iostream>

extern CRITICAL_SECTION cs;
extern HANDLE hStartSignal, hContinueSignal, hRemoveEvent;
extern HANDLE* hFinishEvents;
extern int* arr;
extern volatile int dim;
extern volatile int rem;