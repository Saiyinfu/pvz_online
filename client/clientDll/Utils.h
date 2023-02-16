#include "pch.h"
#include <strsafe.h>
#include <sstream>

VOID FormatOutputDebugStringW(std::wstring fmt, ...);
#define FormatOutputDebugString FormatOutputDebugStringW

VOID StreamOutputDebugStringW(std::wstringstream stream);

// 以PVOID数组提供指针集
BOOL ReadOffsetsPointerMemoryArr(PVOID pDest, size_t nRead, PVOID baseAddr, size_t nOffset, PVOID* offsets);

// 以PVOID数组提供指针集
template<size_t Size, typename T>
inline BOOL ReadOffsetsPointerMemoryArr(T& dest, PVOID baseAddr, PVOID(&offsets)[Size]) {
	return ReadOffsetsPointerMemoryArr((PVOID)(&dest), sizeof(T), baseAddr, Size, offsets);
}

// 以std::vector提供指针集
BOOL ReadOffsetPointerMemoryVec(PVOID pDest, size_t nRead, PVOID baseAddr, std::vector<PVOID> offsets = { });

// 以std::vector提供指针集
template<typename T>
BOOL ReadOffsetPointerMemoryVec(T& dest, PVOID baseAddr, std::vector<PVOID> offsets) {
	return ReadOffsetsPointerMemoryArr((PVOID)(&dest), sizeof(T), baseAddr, offsets.size(), offsets.data());
}

// 以PVOID数组提供指针集
BOOL WriteOffsetsPointerMemoryArr(PVOID pSrc, size_t nWrite, PVOID base_addr, size_t nOffset, PVOID* offsets);

// 以PVOID数组提供指针集
template<size_t Size, typename T>
inline BOOL WriteOffsetsPointerMemoryArr(T& src, PVOID baseAddr, PVOID(&offsets)[Size]) {
	return WriteOffsetsPointerMemoryArr((PVOID)(&src), sizeof(T), baseAddr, Size, offsets);
}

// 以std::vector提供指针集
BOOL WriteOffsetsPointerMemoryVec(PVOID pSrc, size_t nWrite, PVOID base_addr, std::vector<PVOID> offsets);

// 以std::vector提供指针集
template<typename T>
BOOL WriteOffsetsPointerMemoryVec(T& src, PVOID baseAddr, std::vector<PVOID> offsets) {
	return WriteOffsetsPointerMemoryVec((PVOID)(&src), sizeof(T), baseAddr, offsets.size(), offsets.data());
}

std::wstring string2wstring(std::string str);
std::wstring utf82wstring(std::string str);
std::string wstring2string(std::wstring wstr);
std::string wstring2utf8(std::wstring wstr);

DWORD GetMainThreadId();