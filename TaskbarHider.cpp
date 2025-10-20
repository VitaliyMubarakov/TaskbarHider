#include "src/taskbarManager.h"
#include "src/taskbarTray.h"

using namespace std;

bool IsAlreadyRunning(const wchar_t* mutexName = L"MyTaskbarManagerMutex")
{
    HANDLE hMutex = CreateMutexW(NULL, TRUE, mutexName);

    if (hMutex == NULL)
        return false;

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return true;
    }
    return false;
}

#define ID_BUTTON_CLOSE 2001

int main() {
    if (IsAlreadyRunning()) {
        MessageBoxW(NULL, L"Программа уже запущена!", L"Taskbar Manager", MB_OK | MB_ICONWARNING);
        return 0;
    }

    SetRussianLang();
    INIData();

    const wchar_t CLASS_NAME[] = L"TaskbarMainWindow";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hWndMain = CreateWindowW(CLASS_NAME, L"Taskbar Manager", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 80,
        NULL, NULL, GetModuleHandle(NULL), NULL);

    HWND hWndClose = CreateWindowW(
        L"BUTTON",
        L"Закрыть",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        30, 20, 120, 30,
        hWndMain,
        (HMENU)ID_BUTTON_CLOSE,
        GetModuleHandle(NULL),
        NULL
    );

    InitTray(GetModuleHandle(NULL), hWndMain);

    std::thread(checkMousePosUpdate).detach();

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_COMMAND && LOWORD(msg.wParam) == ID_BUTTON_CLOSE) {
            RemoveTray();
            PostQuitMessage(0);
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    RemoveTray();
    return 0;
}

#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main();
}
#endif