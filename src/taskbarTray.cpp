#include "taskbarTray.h"
#include <shellapi.h>
#include "taskbarManager.h"

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_RELOAD 1002

static NOTIFYICONDATAW nid = {};
static HMENU hMenu = NULL;
static HWND hWndTray = nullptr;
static HWND hWndMain = nullptr;

static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        }
        else if (lParam == WM_LBUTTONDBLCLK) {
            if (hWndMain) ShowWindow(hWndMain, SW_SHOW);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
            RemoveTray();
            PostQuitMessage(0);
            break;
        case ID_TRAY_RELOAD:
            INIData();
            break;
        }
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void InitTray(HINSTANCE hInst, HWND mainWnd)
{
    hWndMain = mainWnd;

    const wchar_t CLASS_NAME[] = L"TaskbarTrayClass";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    hWndTray = CreateWindowW(CLASS_NAME, L"Tray Window", 0,
        0, 0, 0, 0, NULL, NULL, hInst, NULL);

    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWndTray;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(NULL, MAKEINTRESOURCEW(IDI_APPLICATION));
    wcscpy_s(nid.szTip, L"Taskbar Manager");

    Shell_NotifyIconW(NIM_ADD, &nid);

    hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_RELOAD, L"Reload Settings");
}

void RemoveTray()
{
    Shell_NotifyIconW(NIM_DELETE, &nid);
    if (hMenu) DestroyMenu(hMenu);
    if (hWndTray) DestroyWindow(hWndTray);
}
