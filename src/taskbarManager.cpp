#include "./taskbarManager.h"
#include "..\libs\iniParser.h"

bool isTaskbarHidden = true;
int mouseAboveMs = 5000;
int screenPercentToHide = 50;

POINT pt;
chrono::steady_clock::time_point mouseAboveStartTime;
bool isMouseAbove = false;

static std::atomic_bool transitionInProgress = false;
static std::mutex transitionMutex;

HWND GetTaskbarWindow() {
    return FindWindowW(L"Shell_TrayWnd", NULL);
}

HWND GetTaskbarReBar() {
    HWND taskbar = GetTaskbarWindow();
    if (!taskbar) return NULL;
    return FindWindowExW(taskbar, NULL, L"ReBarWindow32", NULL);
}

HWND GetTaskbarCompositionBridge() {
    HWND taskbar = GetTaskbarWindow();
    if (!taskbar) return NULL;
    return taskbar;
    //return FindWindowExW(taskbar, NULL, L"Windows.UI.Composition.DesktopWindowContentBridge", NULL);
}

void HideWindowSimple(HWND hwnd) {
    if (hwnd && IsWindow(hwnd)) ShowWindow(hwnd, SW_HIDE);
}

void ShowWindowSimple(HWND hwnd) {
    if (hwnd && IsWindow(hwnd)) ShowWindow(hwnd, SW_SHOW);
}

void HideTaskbar() {
    auto bar = GetTaskbarCompositionBridge();
    if (bar) {
        HideWindowSimple(bar);
        isTaskbarHidden = true;
    }
}

void ShowTaskbar() {
    auto bar = GetTaskbarCompositionBridge();
    if (bar) {
        ShowWindowSimple(bar);
        isTaskbarHidden = false;
    }
}

void smoothTransitionTransparency(HWND hWnd, BYTE targetAlpha, int durationMs) {
    if (!hWnd || !IsWindow(hWnd)) return;

    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (!(style & WS_EX_LAYERED)) {
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
    }

    BYTE startAlpha = 255;
    DWORD flags = 0;
    COLORREF colorKey = 0;
    GetLayeredWindowAttributes(hWnd, &colorKey, &startAlpha, &flags);

    const int refreshRate = 60;
    const float frameDuration = 1000.0f / refreshRate;
    int steps = max(1, durationMs / static_cast<int>(frameDuration));
    float increment = static_cast<float>(targetAlpha - startAlpha) / steps;

    LARGE_INTEGER frequency, startTime;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startTime);

    for (int i = 1; i <= steps; ++i) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        float elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;

        float newAlphaF = startAlpha + increment * i;
        BYTE newAlpha = static_cast<BYTE>(std::clamp(newAlphaF, 0.0f, 255.0f));

        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), newAlpha, LWA_ALPHA);

        while (elapsedMs < frameDuration * i) {
            QueryPerformanceCounter(&currentTime);
            elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
    }

    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), targetAlpha, LWA_ALPHA);
}

void checkMousePosUpdate() {
    static bool hideTriggered = false;
    static std::future<void> transitionFuture;
    static bool isHiding = false;

    while (true) {
        int screenHeight = GetSystemMetrics(SM_CYSCREEN) -
            GetSystemMetrics(SM_CYSCREEN) * (screenPercentToHide / 100.0f);

        GetCursorPos(&pt);
        bool currentlyAbove = (pt.y < screenHeight);

        if (currentlyAbove && !isMouseAbove) {
            isMouseAbove = true;
            mouseAboveStartTime = std::chrono::steady_clock::now();
            hideTriggered = false;
        }
        else if (!currentlyAbove && isMouseAbove) {
            isMouseAbove = false;
            hideTriggered = false;

            if (isTaskbarHidden || isHiding) {
                auto bar = GetTaskbarCompositionBridge();
                if (transitionInProgress && transitionFuture.valid()) {
                    transitionFuture.wait();
                    transitionInProgress = false;
                }

                ShowTaskbar();
                if (bar) {
                    std::lock_guard<std::mutex> lock(transitionMutex);
                    std::async(std::launch::async, smoothTransitionTransparency, bar, 255, 100);
                }

                isTaskbarHidden = false;
                isHiding = false;
            }
        }

        if (transitionInProgress && transitionFuture.valid() &&
            transitionFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            transitionInProgress = false;
            isHiding = false;

            if (!isTaskbarHidden) HideTaskbar();
        }

        if (isMouseAbove && !isTaskbarHidden && !hideTriggered && !isHiding) {
            auto currentTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - mouseAboveStartTime).count();

            if (duration >= mouseAboveMs - 10) {
                hideTriggered = true;
                isHiding = true;
                auto bar = GetTaskbarCompositionBridge();
                if (bar) {
                    std::lock_guard<std::mutex> lock(transitionMutex);
                    transitionFuture = std::async(std::launch::async, smoothTransitionTransparency, bar, 0, 100);
                    transitionInProgress = true;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void SetRussianLang()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
}

std::string getDirectory(const std::string& fullPath) {
    size_t pos = fullPath.find_last_of("/\\");
    if (pos == std::string::npos) {
        return "";
    }
    return fullPath.substr(0, pos);
}

bool GetBoolFromIni(const std::string& value) {
    std::string lowerValue = value;
    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);

    if (lowerValue == "true" || lowerValue == "1" || lowerValue == "yes" ||
        lowerValue == "on" || lowerValue == "enabled") {
        return true;
    }

    if (lowerValue == "false" || lowerValue == "0" || lowerValue == "no" ||
        lowerValue == "off" || lowerValue == "disabled") {
        return false;
    }

    return false;
}

void INIData() {
    IniParser parser;

    try {
        wchar_t wpath[MAX_PATH];
        GetModuleFileNameW(nullptr, wpath, MAX_PATH);

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
        std::string path(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, &path[0], size_needed, NULL, NULL);

        if (!path.empty() && path.back() == '\0') path.pop_back();
        std::cout << "Текущий путь к исполняемому файлу: " << getDirectory(path) << std::endl;

        parser.createIniFile(getDirectory(path));

        parser.parseFromFile("settings.ini");

        mouseAboveMs = std::stoi(parser.getValue("Taskbar", "mouseAboveMs"));
        screenPercentToHide = std::stoi(parser.getValue("Taskbar", "screenPercentToHide"));

        if (screenPercentToHide < 0 || screenPercentToHide > 100) {
            cout << "[ERROR] screenPercentToHide value (0-100): " << screenPercentToHide << endl;
            screenPercentToHide = 50;
        }

        if (mouseAboveMs < 0) {
            cout << "[ERROR] mouseAboveMs value (0-99999): " << mouseAboveMs << endl;
            mouseAboveMs = 5000;
        }

        cout << "mouseAboveMs: " << mouseAboveMs << endl;
        cout << "screenPercentToHide: " << screenPercentToHide << endl;

        parser.saveToFile("settings.ini");
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }
}
