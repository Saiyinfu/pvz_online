#include "pch.h"
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <d3d9.h>
#include <atomic>
#include <winuser.h>

#include "GUI.h"

#include "Utils.h"
#include "Values.h"
#include "Functions.h"
#include "Consts.h"
#include "Hooks.h"
#include "Communication.h"

using std::string;
using std::vector;

#pragma comment(lib, "d3d9.lib")

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};
static std::atomic<BOOL>        is_done = FALSE;



// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


VOID DebugWindowImpl(GuiType &gui) {
    bool is_pop = true;
    PVOID game_obj;
    ImGui::Begin("调试");
    if (Values::GetGameObjAddr(game_obj) && game_obj != NULL) {
        {
            int sun;
            static bool set_sun_result = false;
            if (Values::GetSunValue(sun)) {
                ImGui::Text("阳光数：%d", sun);
                if (ImGui::InputInt("阳光数", &sun, 10)) {
                    set_sun_result = Values::SetSunValue(sun);
                }
                ImGui::Text(set_sun_result ? "成功设置阳光" : "未成功设置阳光");
            }
            else {
                ImGui::Text("无法获取阳光数");
            }
        }
        ImGui::Separator();
        {
            int zombie_num = 0;
            static int line = 2, type = 0;
            if (Values::GetZombieNum(zombie_num)) {
                static int current_item = 0;
                ImGui::Text("僵尸数：%d", zombie_num);
                vector<char*> items;
                for (int i = 0; i < zombie_num; i++) {
                    char* text = (char*)_malloca(256);
                    int id = -1;
                    Values::GetZombieId(i, id);
                    sprintf_s(text, 256, "index=%d, id=%d", i, id);
                    items.push_back(text);
                }
                ImGui::ListBox("僵尸列表", &current_item, 
                    items.data(), zombie_num);
                for (int i = 0; i < zombie_num; i++) {
                    _freea(items[i]);
                }
            }
            ImGui::SliderInt("生成行数", &line, 0, 4);
            ImGui::SliderInt("生成种类", &type, 0, 32);
            ImGui::Text("选定僵尸类型：%s", (type < sizeof(ZOMBIE_NAMES) / sizeof(ZOMBIE_NAMES[0])) ? ZOMBIE_NAMES[type] : "未知");
            if (ImGui::Button("出僵尸")) {
                Functions::GenerateZombie(line, type);
            }
        }
        ImGui::Separator();
        {
            int plant_num = 0;
            static int line = 0, row = 0, type = 0;
            if (Values::GetPlantNum(plant_num)) {
                ImGui::Text("植物数：%d", plant_num);
            }
            ImGui::SliderInt("种植行数", &line, 0, 4);
            ImGui::SliderInt("种植列数", &row, 0, 8);
            ImGui::SliderInt("植物种类", &type, 0, 47);
            if (ImGui::Button("种植")) {
                Functions::PlacePlant(line, row, type);
            }
        }
        
        
    }
    else {
        ImGui::Text("游戏对象未创建");
    }
    ImGui::Separator();
    {
        if (ImGui::Button("Send hello to server")) {
            Communication::SendLog(L"Hello", LogLevel::INFO);
        }
    }
    ImGui::Separator();
    {
        if (ImGui::Button("Hook")) {
            HookEntry();
        }
        if (ImGui::Button("Unhook")) {
            HookLeave();
        }
    }
    ImGui::End();
}

VOID MyGuiImpl(GuiType &gui) {
    static bool is_debug = false;
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;

    ImGui::Begin("客户端", 0, flags);
    {
        ImGui::SetWindowPos(ImVec2(0, 0));
        RECT rect;
        GetClientRect(gui.hClientWindow, &rect);
        ImGui::SetWindowSize(ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top)));
        ImGui::Checkbox("Debug", &is_debug);

        {
            std::string user_name;
            if (Values::GetPvzUserName(user_name)) {
                ImGui::Text("用户名:%s", user_name.c_str());
            }
            else {
                ImGui::Text("无法获取用户名");
            }
        }
        {
            static char url[1024];
            static int status;
            ImGui::InputText("服务器地址", url, sizeof(url));
            if (ImGui::Button("连接")) {
                Communication::ConnectServer(url);
            }
            ImGui::Text("连接状态：%s", Communication::isConnected() ? "已连接" : "未连接");
            ImGui::Text("玩家Id: %d", Communication::GetPlayerId());
        }
    }
    ImGui::End();
    if (is_debug) {
        DebugWindowImpl(gui);
    }
}


DWORD WINAPI UiThreadFunction(LPARAM lpGui) {
    

    GuiType* gui = reinterpret_cast<GuiType *>(lpGui);
    
    // OutputDebugStringW(L"Enter " __FUNCTION__);
    FormatOutputDebugString(L"Enter Function");
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX9 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

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

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    auto simfang_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\simfang.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    gui->hClientWindow = hwnd;

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

        MyGuiImpl(*gui);

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
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    FormatOutputDebugString(L"Leave Function");
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

BOOL CALLBACK _EnumWindowsFunc(HWND hwnd, LPARAM lParam) {
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
        if (dwPid == GetCurrentProcessId())
        {
            *reinterpret_cast<HWND*>(lParam) = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

VOID InitUi(GuiType &gui) {
    FormatOutputDebugString(L"Enter Function");
    HWND hwnd = 0;
    EnumWindows(_EnumWindowsFunc, reinterpret_cast<LPARAM>(&hwnd));
    gui.hUiThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UiThreadFunction, (LPVOID)&gui, 0, 0);
    gui.hPvzWindow = hwnd;
}

VOID UninitUi(GuiType &gui) {
    FormatOutputDebugString(L"Enter Function");
    //is_done = TRUE;
    PostMessageW(gui.hClientWindow, WM_QUIT, NULL, NULL);
    DWORD dwRes = WaitForSingleObject(gui.hUiThread, 5000);
    if (dwRes == WAIT_TIMEOUT)
    {
        OutputDebugStringW(L"WaitForSingleObject return WAIT_TIMEOUT");
        TerminateThread(gui.hUiThread, 0);
    }
}