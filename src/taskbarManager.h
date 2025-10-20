#pragma once
#include "windows.h"
#include <future>
#include <chrono>
#include <iostream>
#include <atomic>
#include <mutex>
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

std::string getDirectory(const std::string& fullPath);

bool GetBoolFromIni(const std::string& value);

void INIData();
