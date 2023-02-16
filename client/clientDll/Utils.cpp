#include "pch.h"
#include "Utils.h"
#include <string>
#include <TlHelp32.h>

VOID FormatOutputDebugStringW(std::wstring fmt, ...) {
	va_list va = nullptr;
	va_start(va, fmt);
	WCHAR str[2048];
	vswprintf_s(str, fmt.c_str(), va);
	OutputDebugStringW(str);
}

VOID StreamOutputDebugStringW(std::wstringstream stream) {
	OutputDebugStringW(stream.str().c_str());
}

// 以PVOID数组提供指针集
BOOL ReadOffsetsPointerMemoryArr(PVOID pDest, size_t nRead, PVOID baseAddr, size_t nOffset, PVOID* offsets) {
	PVOID lastPtr = baseAddr;
	__try {
		for (size_t i = 0; i < nOffset; ++i) {
			lastPtr = (PVOID)(*((size_t*)lastPtr) + (size_t)offsets[i]);
		}
		memcpy(pDest, lastPtr, nRead);
		return TRUE;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
		return FALSE;
	}
}

// 以std::vector提供指针集
BOOL ReadOffsetPointerMemoryVec(PVOID pDest, size_t nRead, PVOID baseAddr, std::vector<PVOID> offsets) {
	return ReadOffsetsPointerMemoryArr(pDest, nRead, baseAddr, offsets.size(), offsets.data());
}

BOOL WriteOffsetsPointerMemoryArr(PVOID pSrc, size_t nWrite, PVOID base_addr, size_t nOffset, PVOID* offsets) {
	PVOID lastPtr = base_addr;
	__try {
		for (size_t i = 0; i < nOffset; ++i) {
			lastPtr = (PVOID)(*((size_t*)lastPtr) + (size_t)offsets[i]);
		}
		memcpy(lastPtr, pSrc, nWrite);
		return TRUE;
	}
	__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION) {
		return FALSE;
	}
}

// 以std::vector提供指针集
BOOL WriteOffsetsPointerMemoryVec(PVOID pSrc, size_t nWrite, PVOID base_addr, std::vector<PVOID> offsets) {
	return WriteOffsetsPointerMemoryArr(pSrc, nWrite, base_addr, offsets.size(), offsets.data());
}

std::wstring string2wstring(std::string str) {
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
	WCHAR* wstrbuf = new WCHAR[len + 1];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wstrbuf, len);
	wstrbuf[len] = L'\0';
	std::wstring res(wstrbuf);
	delete[] wstrbuf;
	return res;
}

std::wstring utf82wstring(std::string str) {
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), NULL, 0);
	WCHAR* wstrbuf = new WCHAR[len + 1];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), wstrbuf, len);
	wstrbuf[len] = L'\0';
	std::wstring res(wstrbuf);
	delete[] wstrbuf;
	return res;
}

std::string wstring2string(std::wstring wstr) {
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* strbuf = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), strbuf, len, NULL, NULL);
	strbuf[len] = '\0';
	std::string res(strbuf);
	delete[] strbuf;
	return res;
}

std::string wstring2utf8(std::wstring wstr) {
	int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* strbuf = new char[len + 1];
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), strbuf, len, NULL, NULL);
	strbuf[len] = '\0';
	std::string res(strbuf);
	delete[] strbuf;
	return res;
}

DWORD GetMainThreadId() {
	DWORD dwMainThreadID = 0;
	ULONGLONG ullMinCreateTime = MAXULONGLONG;

	DWORD dwProcID = GetCurrentProcessId();

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap != INVALID_HANDLE_VALUE) {
		THREADENTRY32 th32{ };
		th32.dwSize = sizeof(THREADENTRY32);
		BOOL bOK = TRUE;
		for (bOK = Thread32First(hThreadSnap, &th32); bOK;
			bOK = Thread32Next(hThreadSnap, &th32)) {
			if (th32.th32OwnerProcessID == dwProcID) {
				HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION,
					TRUE, th32.th32ThreadID);
				if (hThread) {
					FILETIME afTimes[4] = { 0 };
					if (GetThreadTimes(hThread,
						&afTimes[0], &afTimes[1], &afTimes[2], &afTimes[3])) {
						ULONGLONG ullTest = ULONGLONG(afTimes[0].dwLowDateTime) + (ULONGLONG(afTimes[0].dwHighDateTime) << 32);
						if (ullTest && ullTest < ullMinCreateTime) {
							ullMinCreateTime = ullTest;
							dwMainThreadID = th32.th32ThreadID;
						}
					}
					CloseHandle(hThread);
				}
			}
		}
		CloseHandle(hThreadSnap);
	}
	return dwMainThreadID;
}