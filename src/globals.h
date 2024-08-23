#include <vector>
#ifndef GLOBALS_H // header guards
#define GLOBALS_H

inline bool keepShifting = false;
inline bool showMenu = true;
inline HHOOK hKeyboardHook;
inline std::atomic<bool> autoClickerRunning{ false };
inline static bool checkboxState = false;
inline static bool shiftcheckboxState = false;
inline int openKey = VK_F2; // Default to F2

#endif