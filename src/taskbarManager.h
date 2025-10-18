#pragma once
#include "windows.h"
#include <future>
#include <chrono>
#include <iostream>
using namespace std;

extern bool isTaskbarHidden;
extern int mouseAboveMs;
extern int screenPercentToHide;

HWND GetTaskbarWindow();
HWND GetTaskbarReBar();
HWND GetTaskbarCompositionBridge();

void HideTaskbar();
void ShowTaskbar();

void smoothTransitionTransparency(HWND hWnd, BYTE targetAlpha, int durationMs);

void checkMousePosUpdate();

void SetRussianLang();
