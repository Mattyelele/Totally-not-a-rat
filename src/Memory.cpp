#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>

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
