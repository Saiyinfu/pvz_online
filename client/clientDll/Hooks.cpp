#include "pch.h"
#include <polyhook2/ZydisDisassembler.hpp>
#include <polyhook2/Detour/x86Detour.hpp>

#include "Consts.h"
#include "Communication.h"
#include "Utils.h"


std::vector<PLH::x86Detour*> detourVector{};

static uint64_t origPlacePlant = NULL;
static PLH::ZydisDisassembler *disasm;

const int NOTHOOKED = 0;
const int HOOKED = 1;
const int UNHOOKED = 2;
static int hookStatus = NOTHOOKED;

void sendRemoteLogI(LPCWSTR wstr) {
	Communication::SendLog(wstr, LogLevel::INFO);
}

__declspec(naked) DWORD __stdcall hookPlacePlant(PVOID game_obj, int row, int plantType, int a4) {
	volatile int line;
	__asm {
		push   ebp
		mov    ebp, esp
		sub    esp, __LOCAL_SIZE
		mov    line, eax;
	}
	WCHAR text[2048];
	wsprintfW(text, L"Place Plant: \ngame_obj=%p, line=%d, row=%d, type=%d, origPlacePlant=%x", game_obj, line, row, plantType, (uint32_t)origPlacePlant);
	//MessageBoxW(NULL, text, L"", MB_OK);
	sendRemoteLogI(text);
	OutputDebugStringW(text);
	// DWORD retval;
	PVOID orig;
	orig = (PVOID)origPlacePlant;
	__asm {
		mov eax, line
		push a4
		push plantType
		push row
		push game_obj
		call orig //种植
		mov esp, ebp
		pop ebp
		ret 16
	}
}

static uint64_t origGenerateZombie = NULL;
__declspec(naked) int __stdcall hookGenerateZombie(int zombieType, int line) {
	int game_obj;
	int reg_ebx;
	__asm {
		push   ebp
		mov    ebp, esp
		sub    esp, __LOCAL_SIZE
		mov    game_obj, eax
		mov    reg_ebx, ebx
	}
	WCHAR text[2048];
	if (reg_ebx < 0) {
		wsprintfW(text, L"Generate Zombie: \ngame_obj=%p, line=%d, zombieType=%d, ebx=%x(%d) 预先生成，不计", (PVOID)game_obj, line, zombieType, reg_ebx, reg_ebx);
	}
	else {
		wsprintfW(text, L"Generate Zombie: \ngame_obj=%p, line=%d, zombieType=%d, ebx=%x(%d)", (PVOID)game_obj, line, zombieType, reg_ebx, reg_ebx);
	}
	//MessageBoxW(NULL, text, L"", MB_OK);
	sendRemoteLogI(text);
	OutputDebugStringW(text);
	PVOID orig;
	orig = (PVOID)origGenerateZombie;
	__asm {
		mov ecx, 0
		mov eax, game_obj
		push line
		push zombieType
		call orig
		mov esp, ebp
		pop ebp
		ret 8
	}
}

VOID HookEntry() {
	if (hookStatus == NOTHOOKED) {
		disasm = new PLH::ZydisDisassembler(PLH::Mode::x86);
		PLH::x86Detour* detour = new PLH::x86Detour((const uint64_t)fpPlacePlant, (const uint64_t)(&hookPlacePlant), &origPlacePlant, *disasm);
		detourVector.push_back(detour);
		detour = new PLH::x86Detour((const uint64_t)fpGenerateZombie, (const uint64_t)(&hookGenerateZombie), &origGenerateZombie, *disasm);
		detourVector.push_back(detour);

		for (size_t i = 0; i < detourVector.size(); i++) {
			detourVector[i]->hook();
		}
		hookStatus = HOOKED;
	}
	else if (hookStatus == UNHOOKED) {
		for (size_t i = 0; i < detourVector.size(); i++) {
			detourVector[i]->reHook();
		}
		hookStatus = HOOKED;
	}
	
	
}

VOID HookLeave() {
	if (hookStatus != HOOKED) return;
	for (size_t i = 0; i < detourVector.size(); i++) {
		detourVector[i]->unHook();
	}
	hookStatus = UNHOOKED;
}