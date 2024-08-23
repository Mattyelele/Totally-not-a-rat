#include <windows.h>
#include <thread>
#include "globals.h"

// Function to simulate a right mouse click
void RightClick() {
    if (keepShifting) {
        keybd_event(VK_SHIFT, 0, 0, 0);
    }
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

// Auto-clicker thread function
void AutoClicker(std::atomic<bool>& running) {
    while (running.load()) {
        RightClick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}