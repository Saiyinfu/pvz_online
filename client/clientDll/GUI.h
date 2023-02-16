#pragma once
#include <thread>

struct GuiType
{
	HWND hPvzWindow;
	HANDLE hUiThread;
	HWND hClientWindow;
};

VOID InitUi(GuiType& gui);

VOID UninitUi(GuiType& gui);