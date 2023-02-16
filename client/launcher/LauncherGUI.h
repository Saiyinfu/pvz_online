#pragma once
#include <windef.h>


struct GuiType
{
	LPWSTR* pArgv;
	int nArgc;
	HANDLE hUiThread;
	HANDLE hRefreshProcessesThread;
};

VOID InitUi(GuiType& gui);

VOID UninitUi(GuiType& gui);