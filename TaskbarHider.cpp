#include "src/taskbarManager.h"
#include "libs/IniParser.h"

using namespace std;

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
        char path[MAX_PATH];
        GetModuleFileName(nullptr, path, MAX_PATH);
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

int main() {
    HWND hWnd = GetConsoleWindow();

#if defined _DEBUG
    ShowWindow(hWnd, SW_SHOW);
#else
    ShowWindow(hWnd, SW_HIDE);
#endif

    SetRussianLang();

    INIData();

    auto future = std::async(std::launch::async, checkMousePosUpdate);

    HWND bridge = GetTaskbarCompositionBridge();
    if (!bridge) {
        std::cout << "DesktopWindowContentBridge not found" << std::endl;
        return 0;
    }
}

#ifdef _WIN32
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return main();
}
#endif