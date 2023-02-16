#include "launcher.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <algorithm>
#include <shellapi.h>

#include "LauncherGUI.h"
#include "defs.h"

std::vector<HANDLE> hPvzProcesses;

BOOL CALLBACK _EnumWindowsFunc(HWND hwnd, LPARAM lParam) {
	std::vector<PvzInstance>* pvz_processes = reinterpret_cast<decltype(pvz_processes)>(lParam);
	WCHAR windowText[1024];
	if (!GetWindowTextW(hwnd, windowText, sizeof(windowText) / 2)) {
		return TRUE;
	}
	WCHAR windowClass[1024];
	if (!GetClassNameW(hwnd, windowClass, sizeof(windowClass) / 2)) {
		return TRUE;
	}
	if (lstrcmpW(windowText, L"Plants vs. Zombies") == 0 &&
		lstrcmpW(windowClass, L"MainWindow") == 0) {
		DWORD dwPid = -1;
		GetWindowThreadProcessId(hwnd, &dwPid);
		PvzInstance ins;
		ins.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		ins.dwPid = dwPid;
		ins.hwnd = hwnd;
		ins.isInjected = GetPvzIfInjected(ins.hProcess, DLL_NAME, NULL);
		pvz_processes->push_back(ins);
	}
	return TRUE;
}

std::vector<PvzInstance> GetPvzInstances() {
	std::vector<PvzInstance>* pvz_processes = new std::vector<PvzInstance>();
	EnumWindows(_EnumWindowsFunc, reinterpret_cast<LPARAM>(pvz_processes));
	std::sort(pvz_processes->begin(), pvz_processes->end(), [](PvzInstance a, PvzInstance b) {
		return a.dwPid < b.dwPid;
		});
	return *pvz_processes;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR pCmdLine, int nCmdShow) {

	BOOL isEject = FALSE, isKill = FALSE;
	int nArgc = 0;
	LPWSTR *pArgv = CommandLineToArgvW(pCmdLine, &nArgc);
	for (int i = 0; i < nArgc; i++) {
		if(!lstrcmpW(pArgv[i], L"--eject")) {
			isEject = TRUE;
		}
		if (!lstrcmpW(pArgv[i], L"--kill")) {
			isKill = TRUE;
		}
	}

	GuiType gui;
	InitUi(gui);
	WaitForSingleObject(gui.hUiThread, INFINITE);

	/*EnumWindows(_EnumWindowsFunc, 0);
	if (hPvzProcesses.size() > 1) {
		int res = MessageBoxW(NULL, L"打开了多个PVZ进程，是否继续", L"提示", MB_OKCANCEL);
		if (res == IDCANCEL) {
			ExitProcess(0);
		}
	}
	for (size_t i = 0; i < hPvzProcesses.size(); i++) {
		HANDLE hPvzProcess = hPvzProcesses[i];
		if (isEject) {
			EjectDllFromRemoteProcess(hPvzProcess, L"clientDll.dll");
		}
		if (!InjectDllToRemoteProcess(hPvzProcess, L"E:\\pvz_online\\client\\Debug\\clientDll.dll"))
		{
			MessageBoxW(NULL, L"Dll注入失败", L"", MB_OK);
			ExitProcess(0);
		}
	}*/
}

void ErrorMsgBox(LPCWSTR text) {
	MessageBoxW(NULL, text, L"Error", MB_OK);
}

BOOL InjectDllToRemoteProcess(HANDLE hProcess, LPCWSTR fullDllPath) {
	size_t pathLen = lstrlenW(fullDllPath) + 1;
	LPVOID remoteDllPath = VirtualAllocEx(hProcess, NULL, pathLen * 2, MEM_COMMIT, PAGE_READWRITE);
	if (remoteDllPath == NULL) {
		ErrorMsgBox(L"远程内存申请失败");
		return FALSE;
	}
	SIZE_T writtenBytes = -1;
	WriteProcessMemory(hProcess, remoteDllPath, fullDllPath, pathLen * 2, &writtenBytes);
	if (writtenBytes != pathLen * 2) {
		ErrorMsgBox(L"远程内存申请长度不足");
		return FALSE;
	}
	LPVOID lpLoadLibrary = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "LoadLibraryW");
	HANDLE hThread = CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpLoadLibrary, 
		remoteDllPath, 0, NULL, NULL);
	if (hThread == NULL) {
		ErrorMsgBox(L"远程线程创建失败(InjectDllToRemoteProcess)");
	}
	return TRUE;
}

BOOL GetPvzIfInjected(IN HANDLE hProcess, IN LPCWSTR dllName, OUT MODULEENTRY32W *moduleEntry) {
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot;
	HMODULE hModule = NULL;
	MODULEENTRY32W me = { sizeof(me) };

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(hProcess));
	bMore = Module32FirstW(hSnapshot, &me);
	for (; bMore; bMore = Module32NextW(hSnapshot, &me)) {
		if (!lstrcmpW(me.szModule, dllName)) {
			bFound = TRUE;
			break;
		}
	}
	CloseHandle(hSnapshot);
	if (bFound && moduleEntry)
	{
		memcpy(moduleEntry, &me, sizeof(me));
	}
	return bFound;
}

BOOL EjectDllFromRemoteProcess(HANDLE hProcess, LPCWSTR dllName) {
	
	MODULEENTRY32W me;
	if (!GetPvzIfInjected(hProcess, dllName, &me))
	{
		return FALSE;
	}

	LPVOID lpFreeLibrary = GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "FreeLibrary");
	HANDLE hThread = CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpFreeLibrary, me.modBaseAddr, 0, NULL, NULL);
	if (hThread == NULL) {
		ErrorMsgBox(L"远程线程创建失败(EjectDllFromRemoteProcess)");
		return FALSE;
	}
	return TRUE;
}

