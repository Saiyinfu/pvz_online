#pragma once

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <tlhelp32.h>
#include <vector>

struct PvzInstance
{
    HANDLE hProcess;
    DWORD dwPid;
    HWND hwnd;
    BOOL isInjected;
};

void ErrorMsgBox(LPCWSTR text);
BOOL InjectDllToRemoteProcess(HANDLE hProcess, LPCWSTR fullDllPath);
BOOL EjectDllFromRemoteProcess(HANDLE hProcess, LPCWSTR dllName);
BOOL GetPvzIfInjected(IN HANDLE hProcess, IN LPCWSTR dllName, OUT MODULEENTRY32W* moduleEntry);
std::vector<PvzInstance> GetPvzInstances();