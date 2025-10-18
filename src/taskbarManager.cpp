#include "./taskbarManager.h"

bool isTaskbarHidden = true;
int mouseAboveMs = 5000;
int screenPercentToHide = 50;

POINT pt;
chrono::steady_clock::time_point mouseAboveStartTime;
bool isMouseAbove = false;

static bool transitionInProgress = false;

HWND GetTaskbarWindow() {
    return FindWindow("Shell_TrayWnd", NULL);
}

HWND GetTaskbarReBar() {
    HWND taskbar = GetTaskbarWindow();
    if (!taskbar) return NULL;
    return FindWindowEx(taskbar, NULL, "ReBarWindow32", NULL);
}

HWND GetTaskbarCompositionBridge() {
    HWND taskbar = GetTaskbarWindow();
    if (!taskbar) return NULL;
    return FindWindowEx(taskbar, NULL, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
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
        //cout << "Taskbar скрыт" << endl;
    }
}

void ShowTaskbar() {
    auto bar = GetTaskbarCompositionBridge();
    if (bar) {
        ShowWindowSimple(bar);
        isTaskbarHidden = false;
        //cout << "Taskbar показан" << endl;
    }
}

void smoothTransitionTransparency(HWND hWnd, BYTE targetAlpha, int durationMs) {
    if (!hWnd) return;

    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    if (!(style & WS_EX_LAYERED)) SetWindowLongPtr(hWnd, GWL_EXSTYLE, style | WS_EX_LAYERED);

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

        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), newAlpha, LWA_ALPHA | LWA_COLORKEY);

        while (elapsedMs < frameDuration * i) {
            QueryPerformanceCounter(&currentTime);
            elapsedMs = (currentTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
    }

    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), targetAlpha, LWA_ALPHA | LWA_COLORKEY);
}

void checkMousePosUpdate() {
    static bool hideTriggered = false;
    static std::future<void> transitionFuture;
    static bool transitionInProgress = false;
    static bool isHiding = false;
    int screenHeight = GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYSCREEN) * (screenPercentToHide / 100.0f);

    while (true) {
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
                std::async(std::launch::async, smoothTransitionTransparency, bar, 255, 100);
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
                transitionFuture = std::async(std::launch::async, smoothTransitionTransparency, bar, 0, 100);
                transitionInProgress = true;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void SetRussianLang()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
}
