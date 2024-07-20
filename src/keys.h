#include <imgui.h>

ImGuiKey VirtualKeyToImGuiKey(int vk) {
    switch (vk) {
    case VK_BACK: return ImGuiKey_Backspace;
    case VK_TAB: return ImGuiKey_Tab;
    case VK_RETURN: return ImGuiKey_Enter;
    case VK_SHIFT: return ImGuiKey_LeftShift;
    case VK_CONTROL: return ImGuiKey_LeftCtrl;
    case VK_MENU: return ImGuiKey_LeftAlt;
    case VK_PAUSE: return ImGuiKey_Pause;
    case VK_CAPITAL: return ImGuiKey_CapsLock;
    case VK_ESCAPE: return ImGuiKey_Escape;
    case VK_SPACE: return ImGuiKey_Space;
    case VK_PRIOR: return ImGuiKey_PageUp;
    case VK_NEXT: return ImGuiKey_PageDown;
    case VK_END: return ImGuiKey_End;
    case VK_HOME: return ImGuiKey_Home;
    case VK_LEFT: return ImGuiKey_LeftArrow;
    case VK_UP: return ImGuiKey_UpArrow;
    case VK_RIGHT: return ImGuiKey_RightArrow;
    case VK_DOWN: return ImGuiKey_DownArrow;
    case VK_INSERT: return ImGuiKey_Insert;
    case VK_DELETE: return ImGuiKey_Delete;
    case VK_NUMPAD0: return ImGuiKey_Keypad0;
    case VK_NUMPAD1: return ImGuiKey_Keypad1;
    case VK_NUMPAD2: return ImGuiKey_Keypad2;
    case VK_NUMPAD3: return ImGuiKey_Keypad3;
    case VK_NUMPAD4: return ImGuiKey_Keypad4;
    case VK_NUMPAD5: return ImGuiKey_Keypad5;
    case VK_NUMPAD6: return ImGuiKey_Keypad6;
    case VK_NUMPAD7: return ImGuiKey_Keypad7;
    case VK_NUMPAD8: return ImGuiKey_Keypad8;
    case VK_NUMPAD9: return ImGuiKey_Keypad9;
    case VK_F1: return ImGuiKey_F1;
    case VK_F2: return ImGuiKey_F2;
    case VK_F3: return ImGuiKey_F3;
    case VK_F4: return ImGuiKey_F4;
    case VK_F5: return ImGuiKey_F5;
    case VK_F6: return ImGuiKey_F6;
    case VK_F7: return ImGuiKey_F7;
    case VK_F8: return ImGuiKey_F8;
    case VK_F9: return ImGuiKey_F9;
    case VK_F10: return ImGuiKey_F10;
    case VK_F11: return ImGuiKey_F11;
    case VK_F12: return ImGuiKey_F12;
        // Add more cases as needed
    default: return ImGuiKey_None; // Unknown key
    }
}
