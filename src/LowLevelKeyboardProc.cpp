#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <d3d11.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <tlhelp32.h>

#include "globals.h"
#include <imgui.h>

// LowLevelKeyboardProc function for global hotkey detection

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool hotkeyPressed = false;
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (p->vkCode == openKey) {
                hotkeyPressed = true;
            }
            if (hotkeyPressed && p->vkCode == 'Q') {
                autoClickerRunning.store(false);
                checkboxState = false;
            }
        }

        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (p->vkCode == openKey) {
                hotkeyPressed = false;

                // Toggle overlay visibility
                HWND window = FindWindow(L"Overlay Menu Class", L"Overlay Menu");
                if (window) {
                    LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
                    if (exStyle == -1) {
                        // Error handling for GetWindowLong
                        std::cerr << "Failed to get window style" << std::endl;
                        return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
                    }

                    ImGuiIO& io = ImGui::GetIO();
                    if (showMenu && io.WantCaptureKeyboard) {
                        // Hide the window
                        exStyle |= WS_EX_TRANSPARENT;
                        if (!SetWindowLong(window, GWL_EXSTYLE, exStyle)) {
                            // Error handling for SetWindowLong
                            std::cerr << "Failed to set window style" << std::endl;
                        }
                        if (!SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(0), LWA_ALPHA)) {
                            // Error handling for SetLayeredWindowAttributes
                            std::cerr << "Failed to set layered window attributes" << std::endl;
                        }
                        SetForegroundWindow(window);
                        ImGui::SetNextFrameWantCaptureKeyboard(true);
                    }
                    else {
                        // Show the window
                        exStyle &= ~WS_EX_TRANSPARENT;
                        if (!SetWindowLong(window, GWL_EXSTYLE, exStyle)) {
                            // Error handling for SetWindowLong
                            std::cerr << "Failed to set window style" << std::endl;
                        }
                        if (!SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA)) {
                            // Error handling for SetLayeredWindowAttributes
                            std::cerr << "Failed to set layered window attributes" << std::endl;
                        }
                        SetForegroundWindow(window);
                        ImGui::SetNextFrameWantCaptureKeyboard(false);
                    }
                    showMenu = !showMenu; // Toggle the state
                }
            }
        }
    }

    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}
