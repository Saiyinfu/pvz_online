#include "pch.h"
#include "Functions.h"
#include "Utils.h"
#include "Values.h"
#include "Consts.h"

BOOL Functions::GenerateZombie(int line, int zombie_type) {
	DWORD dwMainThId = GetMainThreadId();
	HANDLE hMainTh = OpenThread(THREAD_ALL_ACCESS, FALSE, dwMainThId);
	if (!hMainTh) return FALSE;
	SuspendThread(hMainTh);

    int _line = line, _zombie_type = zombie_type;
	PVOID game_obj;
	if (!Values::GetGameObjAddr(game_obj) || game_obj == NULL) {
		ResumeThread(hMainTh);
		CloseHandle(hMainTh);
		return FALSE;
	}
	__asm {
		mov ecx, 0
		/*push ebx
		mov ebx, 0*/
		mov eax, game_obj
		push _line
		push _zombie_type
		call fpGenerateZombie
		//pop ebx
	}
	ResumeThread(hMainTh);
	CloseHandle(hMainTh);
	return TRUE;
}

BOOL Functions::PlacePlant(int line, int row, int plantType) {
	DWORD dwMainThId = GetMainThreadId();
	HANDLE hMainTh = OpenThread(THREAD_ALL_ACCESS, FALSE, dwMainThId);
	if (!hMainTh) return FALSE;
	SuspendThread(hMainTh);

	PVOID game_obj;
	if (!Values::GetGameObjAddr(game_obj) || game_obj == NULL) {
		ResumeThread(hMainTh);
		CloseHandle(hMainTh);
		return FALSE;
	}
	__asm {
		mov eax, line
		push -1
		push plantType
		push row
		push game_obj
		call fpPlacePlant //种植
	}
	ResumeThread(hMainTh);
	CloseHandle(hMainTh);
	return TRUE;
}
