#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>
#include <vector>
#include <psapi.h>

#pragma comment(lib, "Psapi.lib")



void GetProssIdByName(const std::wstring& processName, DWORD& processId) {
    processId = 0;  // Initialize processId to 0
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);
        if (Process32FirstW(snap, &entry)) {
            do {
                if (processName == entry.szExeFile) {
                    processId = entry.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &entry));
        }
        CloseHandle(snap);
    }
}

uintptr_t GetModuleBaseAddress(HANDLE hProcess, const std::wstring& moduleName) {
    MODULEINFO modInfo = { 0 };
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t))) {
                if (moduleName == szModName) {
                    if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                        return reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
                    }
                }
            }
        }
    }

    return 0;
}

HANDLE GetProcessHandle(const std::wstring& processName) {
    HANDLE hProcess = NULL;
    DWORD processID = 0;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Take a snapshot of all processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create toolhelp snapshot" << std::endl;
        return NULL;
    }

    // Retrieve information about the first process
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (processName == pe32.szExeFile) {
                processID = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (processID == 0) {
        std::cerr << "Process not found" << std::endl;
        return NULL;
    }

    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process" << std::endl;
        return NULL;
    }

    return hProcess;
}


void WriteMem(std::wstring processName, LPCVOID Address, byte value) {
    DWORD processId;
    GetProssIdByName(processName, processId);

    if (processId == 0) {
        std::wcerr << L"Failed to find process" << std::endl;
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (hProcess == NULL) {
        std::wcerr << L"Failed to open process" << std::endl;
        return;
    }

    SIZE_T bytesWritten;

    WriteProcessMemory(hProcess, (LPVOID)Address, &value, sizeof(value), &bytesWritten);

    CloseHandle(hProcess);
    return;
}

void WriteToProcessMemory(HANDLE hProcess, uintptr_t address, const std::vector<BYTE>& data) {
    SIZE_T bytesWritten;
    if (!WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), data.data(), data.size(), &bytesWritten)) {
        std::cerr << "Failed to write process memory" << std::endl;
    }
}

void WriteByteArray(std::wstring processName, LPCVOID Address, BYTE array[]) {
    DWORD processId;
    GetProssIdByName(processName, processId);

    if (processId == 0) {
        std::wcerr << L"Failed to find process" << std::endl;
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
    if (hProcess == NULL) {
        std::wcerr << L"Failed to open process" << std::endl;
        return;
    }

    SIZE_T byteArraySize = sizeof(array);
    SIZE_T bytesWritten;

    WriteProcessMemory(hProcess, (LPVOID)Address, &array, sizeof(array), &bytesWritten);

    CloseHandle(hProcess);
    return;
}
