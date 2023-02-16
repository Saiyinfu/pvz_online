// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include "GUI.h"
#include "Utils.h"

GuiType g_gui;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    FormatOutputDebugString(L"Enter Function");
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxW(NULL, L"Hello", L"", MB_OK);
        InitUi(g_gui);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        UninitUi(g_gui);
        break;
    }
    return TRUE;
}

