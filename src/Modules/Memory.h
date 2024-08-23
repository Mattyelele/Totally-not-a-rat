#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <vector>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")

#ifndef MEMORY_H
#define MEMORY_H

HANDLE GetProcessHandle(const std::wstring& processName);
uintptr_t GetModuleBaseAddress(HANDLE hProcess, const std::wstring& moduleName);
void WriteToProcessMemory(HANDLE hProcess, uintptr_t address, const std::vector<BYTE>& data);

#endif // MEMORY_H