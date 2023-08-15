#include <windows.h>
#include <dwmapi.h>
#include <d3d11.h>

#include "gui.hpp"
#include "os_util.hpp"

#include "imgui\imgui.h"
#include "imgui\imgui_impl_dx11.h"
#include "imgui\imgui_impl_win32.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT msg, WPARAM w_param, 
                                                             LPARAM l_param);
                                                             
bool                        esp_checked             = false;
HWND                        main_window             = NULL; 
ID3D11Device                *g_pd3dDevice           = NULL;
ID3D11DeviceContext         *g_pd3dDeviceContext    = NULL;
IDXGISwapChain              *g_pSwapChain           = NULL;
ID3D11RenderTargetView      *g_mainRenderTargetView = NULL;

                                                                                       
LRESULT __stdcall window_procedure(HWND window, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if(ImGui_ImplWin32_WndProcHandler(window, msg, w_param, l_param))
        return true;

    switch (msg)
    {
        std::cout << "NEW MESSAGE BRO: " << msg;
        case WM_SIZE:
            if(g_pd3dDevice != NULL && w_param != SIZE_MINIMIZED)
            {
                clean_up_render_target();
                g_pSwapChain->ResizeBuffers(0, (UINT) LOWORD(l_param), (UINT) HIWORD(l_param), 
                                            DXGI_FORMAT_UNKNOWN, 0);
                create_render_target();
            }
            return 0;
            
        case WM_SYSCOMMAND:
            if((w_param & 0xfff0) == SC_KEYMENU) 
                return 0;
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, msg, w_param, l_param);
}


void clean_up_device_d3d()
{
    clean_up_render_target();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}


void clean_up_render_target()
{           
    if(g_mainRenderTargetView) 
    {
        g_mainRenderTargetView->Release(); 
        g_mainRenderTargetView = NULL;
    }
}


bool create_device_d3d(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, 
                                                      D3D_FEATURE_LEVEL_10_0,};
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, 
                                                NULL, createDeviceFlags, featureLevelArray, 2, 
                                                D3D11_SDK_VERSION, &sd, &g_pSwapChain, 
                                                &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);                                               
    if (res == DXGI_ERROR_UNSUPPORTED) 
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, 
                                            featureLevelArray, 2, D3D11_SDK_VERSION, &sd,
                                            &g_pSwapChain, &g_pd3dDevice, &featureLevel, 
                                            &g_pd3dDeviceContext);

    if (res != S_OK)
        return false;


    create_render_target();
    return true;
}


void create_render_target()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

 
void create_window(HINSTANCE &instance, INT cmd_show)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"Calculator Class";
    
    RegisterClassExW(&wc);

    main_window = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_LAYERED,
                                        wc.lpszClassName, 
                                        L"Calculator",
                                        WS_POPUP,
                                        0,
                                        0,
                                        1720,
                                        900,
                                        nullptr,
                                        nullptr, 
                                        wc.hInstance,
                                        nullptr);
                                        
    SetLayeredWindowAttributes(main_window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
    
    {
        RECT client_area{};
        GetClientRect(main_window, &client_area);
        
        RECT window_area{};
        GetWindowRect(main_window, &window_area);
        
        POINT diff{};
        ClientToScreen(main_window, &diff);
        
        const MARGINS margins
        {
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right, 
            client_area.bottom
        };
        
        DwmExtendFrameIntoClientArea(main_window, &margins);
    }   
    
    ShowWindow(main_window, cmd_show);
	UpdateWindow(main_window);
}


int get_esp_state()
{
    return 0;
}


bool render_frame_begin()
{
    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32 backend.
    MSG msg;
    while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            return false;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::GetIO().MouseDown[0] = GetAsyncKeyState(VK_LBUTTON);
    
    return true;
}


void render_frame_end()
{
    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync   
    
}


void render_frame_gui()
{   
    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::SetNextWindowSize(ImVec2(500, 500));
    ImGui::Begin("Quake Cheat Client", NULL, 
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
                 ImGuiWindowFlags_NoScrollbar);                     

    std::string entity_name = "GWASBALL";
    ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), 17.f, 
		                                        ImVec2(200.0f, 
												200.0f), 
												ImColor(0, 0, 240, 220), 
												(entity_name).c_str(), 0,
												0.0f, 0);
    ImGui::Text("--- ESP ON ---");
    ImGui::End();

}


bool render_init()
{
    // Initialize Direct3D
    if(!create_device_d3d(main_window))
    {
        clean_up_device_d3d();
        return 0;
    }

    IMGUI_CHECKVERSION();
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(main_window);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);    
    return 1;
}


void set_gui_background()
{
    SetWindowLongA(main_window, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED | 
                                             WS_EX_NOACTIVATE);
}


void set_gui_foreground()
{
    SetWindowLongA(main_window, GWL_EXSTYLE, 0);
}


void set_render_window_top()
{
    HWND hwnd_target_window = FindWindowA(NULL, "Quake Champions CLIENT 1.18.RETAIL.115489/115970");
    SetWindowPos(hwnd_target_window, main_window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);    
}


int set_target_window(const std::string &window_name)
{
    main_window = FindWindowA(NULL, window_name.c_str());
    SetLayeredWindowAttributes(main_window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
    {
        RECT client_area{};
        GetClientRect(main_window, &client_area);
        
        RECT window_area{};
        GetWindowRect(main_window, &window_area);
        
        POINT diff{};
        ClientToScreen(main_window, &diff);
        
        const MARGINS margins
        {
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right, 
            client_area.bottom
        };
        
        DwmExtendFrameIntoClientArea(main_window, &margins);
    }   
    ShowWindow(main_window, 1);
	UpdateWindow(main_window);
    return 1;
}

