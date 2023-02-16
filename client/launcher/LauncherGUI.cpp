#include <Windows.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <d3d9.h>
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9tex.h>
#include <winuser.h>

#include "LauncherGUI.h"
#include <vector>
#include <string>
#include "defs.h"
#include "launcher.h"
#include "resource.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/d3dx9.lib")

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static BOOL                     is_done = FALSE;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::vector<PvzInstance> g_PvzInstances;
HANDLE g_hPvzInstancesMutex;

DWORD WINAPI UiThreadFunction(LPARAM lpGui) {


    GuiType* gui = reinterpret_cast<GuiType*>(lpGui);

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Pvz Injector", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"注入器", WS_OVERLAPPEDWINDOW, 100, 100, 630, 700, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 0;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    //加载图片
    PDIRECT3DTEXTURE9 pPicTexture;
    HRESULT hr = D3DXCreateTextureFromResourceW(g_pd3dDevice, NULL, MAKEINTRESOURCEW(IDR_JPEG1), &pPicTexture);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //auto default_font = io.Fonts->AddFontDefault();
    //auto arial_font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 12.0f);
    auto simfang_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\simfang.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    int pvz_process_item = 0;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
    // bool done = false;
    while (!is_done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                is_done = true;
        }
        if (is_done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            ImGuiWindowFlags window_flag = 0;
            window_flag |= ImGuiWindowFlags_NoMove;
            window_flag |= ImGuiWindowFlags_NoResize;

            ImGui::Begin("PVZ注入工具", 0, window_flag);                          // Create a window called "Hello, world!" and append into it.
            ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
            {
                RECT rect = { };
                GetClientRect(hwnd, &rect);
                ImGui::SetWindowSize(ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top)));
            }
            ImGui::Text("技术宅拯救世界！\nTECH OTAKUS SAVE THE WORLD!");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            
            {
                //刷新PVZ进程列表
                for (DWORD dwRes = 0; dwRes == WAIT_TIMEOUT; dwRes = WaitForSingleObject(g_hPvzInstancesMutex, 20));
                char** pvz_items = nullptr;
                size_t nIns = g_PvzInstances.size();
                if (nIns > 0) pvz_items = (PCHAR*)_malloca(g_PvzInstances.size() * sizeof(PCHAR));
                for (size_t i = 0; i < nIns; ++i) {
                    pvz_items[i] = (char*)_malloca(0x20);
                    sprintf_s(pvz_items[i], 0x20, "%d(%s)", g_PvzInstances[i].dwPid, g_PvzInstances[i].isInjected ? "Injected" : "Not injected");
                }
                ImGui::ListBox("PVZ进程", &pvz_process_item, pvz_items, nIns);
                for (size_t i = 0; i < nIns; ++i) {
                }
                ReleaseMutex(g_hPvzInstancesMutex);
            }
            //ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            WCHAR dllPath[MAX_PATH * 2];
            GetCurrentDirectoryW(sizeof(dllPath) / 2, dllPath);
            lstrcatW(dllPath, L"\\" DLL_NAME);
            CHAR _dllPath[MAX_PATH * 2];
            WideCharToMultiByte(CP_UTF8, 0, dllPath, sizeof(dllPath), _dllPath, sizeof(_dllPath), NULL, NULL);
            auto dllFileIsExist = [dllPath]()->bool {
                DWORD dwAttrib = GetFileAttributesW(dllPath);
                return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
                    !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
            }();
            ImGui::TextColored(
                dllFileIsExist ? ImVec4(0xFF, 0xFF, 0xFF, 0xFF) : ImVec4(0xFF, 0, 0, 0xFF),
                "%s(%s)", _dllPath, dllFileIsExist ? "存在" : "不存在"
            );
            if (g_PvzInstances.size() > 0)
            {
                if (ImGui::Button("注入DLL")) {
                    InjectDllToRemoteProcess(g_PvzInstances[pvz_process_item].hProcess, dllPath);
                }
                ImGui::SameLine();
                if (ImGui::Button("杀进程")) {
                    TerminateProcess(g_PvzInstances[pvz_process_item].hProcess, 0);
                }
                ImGui::SameLine();
                if (ImGui::Button("卸载DLL")) {
                    EjectDllFromRemoteProcess(g_PvzInstances[pvz_process_item].hProcess, DLL_NAME);
                }
                ImGui::TextColored(ImVec4(0xFF, 0, 0, 0xFF), "不建议卸载Dll，那样可能会导致PVZ内存泄漏");
            }
            //显示图片
            {
                
                if (hr == D3D_OK) {
                    D3DSURFACE_DESC desc;
                    pPicTexture->GetLevelDesc(0, &desc);
                    ImGui::Image((ImTextureID)pPicTexture, ImVec2(400, 656));
                    ImGui::Text("size = %d * %d", 400, 656);
                }
                else if (hr == D3DERR_INVALIDCALL) {
                    ImGui::Text("D3DERR_INVALIDCALL");
                }
                else if (hr == D3DXERR_INVALIDDATA)
                {
                    ImGui::Text("D3DXERR_INVALIDDATA");
                }
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }


        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();

        //Release 
        
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


DWORD WINAPI RefreshProcessesThread(LPARAM lParam) {
    while (!is_done)
    {
        for (DWORD dwRes = 0; dwRes == WAIT_TIMEOUT; dwRes = WaitForSingleObject(g_hPvzInstancesMutex, 10));
        g_PvzInstances = GetPvzInstances();
        ReleaseMutex(g_hPvzInstancesMutex);
        Sleep(100);
    }
    return 0;
}

//BOOL CALLBACK _EnumWindowsFunc2(HWND hwnd, LPARAM lParam) {
//    WCHAR windowText[1024];
//    if (!GetWindowTextW(hwnd, windowText, sizeof(windowText) / 2)) {
//        return TRUE;
//    }
//    WCHAR windowClass[1024];
//    if (!GetClassNameW(hwnd, windowClass, sizeof(windowClass) / 2)) {
//        return TRUE;
//    }
//    if (lstrcmpW(windowText, L"Plants vs. Zombies") == 0 &&
//        lstrcmpW(windowClass, L"MainWindow") == 0) {
//        DWORD dwPid = -1;
//        GetWindowThreadProcessId(hwnd, &dwPid);
//        if (dwPid == GetCurrentProcessId())
//        {
//            *reinterpret_cast<HWND*>(lParam) = hwnd;
//            return FALSE;
//        }
//    }
//    return TRUE;
//}

VOID InitUi(GuiType& gui) {
    HWND hwnd = 0;
    g_PvzInstances = GetPvzInstances();
    g_hPvzInstancesMutex = CreateMutexW(NULL, FALSE, L"PvzInstancesMutex");
    gui.hUiThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UiThreadFunction, 0, 0, 0);
    gui.hRefreshProcessesThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RefreshProcessesThread, 0, 0, 0);
}

VOID UninitUi(GuiType& gui) {
    is_done = TRUE;
    DWORD dwRes = WaitForSingleObject(gui.hUiThread, 500);
    if (dwRes == WAIT_TIMEOUT)
    {
        OutputDebugStringW(L"WaitForSingleObject return WAIT_TIMEOUT");
    }
    dwRes = WaitForSingleObject(gui.hRefreshProcessesThread, 500);
    if (dwRes == WAIT_TIMEOUT)
    {
        OutputDebugStringW(L"WaitForSingleObject return WAIT_TIMEOUT");
    }
    CloseHandle(g_hPvzInstancesMutex);
}