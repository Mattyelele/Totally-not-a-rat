#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <d3d11.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <tlhelp32.h>

//my headers
#include "ConfigManager.h"
#include "keys.h"

//imgui
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

// Declare hotkey variable
int openKey = VK_F2; // Default to F2

// Function prototypes
void RightClick();
void AutoClicker(std::atomic<bool>& running);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool showMenu = true;
HHOOK hKeyboardHook;
std::atomic<bool> autoClickerRunning{ false };
bool keepShifting = false;
static bool checkboxState = false;
static bool shiftcheckboxState = false;

bool IsAnotherInstanceRunning() {
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"Totally not a rat");

    if (hMutex == NULL) {
        return true; // Error creating mutex
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Mutex already exists, another instance is running
        CloseHandle(hMutex);
        return true;
    }

    // Mutex does not exist, this is the first instance
    return false;
}

// Function to simulate a right mouse click
void RightClick() {
    if (keepShifting) {
        keybd_event(VK_SHIFT, 0, 0, 0);
    }
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

// Auto-clicker thread function
void AutoClicker(std::atomic<bool>& running) {
    while (running.load()) {
        RightClick();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
}

// LowLevelKeyboardProc function for global hotkey detection
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    static bool hotkeyPressed = false;

    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (p->vkCode == openKey) {
            hotkeyPressed = true;
        }
        if (hotkeyPressed && p->vkCode == 'Q') {
            autoClickerRunning.store(false);
            checkboxState = false;
        }
        if (p->vkCode == openKey) {
            showMenu = !showMenu;
            HWND window = FindWindow(L"Overlay Menu Class", L"Overlay Menu");
            if (window) {
                LONG exStyle = GetWindowLong(window, GWL_EXSTYLE);
                if (showMenu) {
                    exStyle &= ~WS_EX_TRANSPARENT;
                }
                else {
                    exStyle |= WS_EX_TRANSPARENT;
                }
                SetWindowLong(window, GWL_EXSTYLE, exStyle);
                SetLayeredWindowAttributes(window, RGB(0, 0, 0), showMenu ? BYTE(255) : BYTE(0), LWA_ALPHA);
                if (showMenu) {
                    SetForegroundWindow(window);
                }
            }
        }
    }
    if (nCode == HC_ACTION && (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (p->vkCode == openKey) {
            hotkeyPressed = false;
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

// Window procedure function
LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam)) {
        return true;
    }

    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
    if (IsAnotherInstanceRunning()) {
        MessageBox(NULL, L"The overlay is already running.", L"Overlay Check", MB_OK | MB_ICONWARNING);
        return 0; // Exit the application
    }

    // Load configuration
    if (!ConfigManager::LoadConfig(openKey)) {
        std::cerr << "Failed to load configuration. Using default hotkey." << std::endl;
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"Overlay Menu Class";

    RegisterClassExW(&wc);

    const HWND window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE, wc.lpszClassName, L"Overlay Menu", WS_POPUP, 0, 0, 1920, 1080, nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(window, &client_area);

        RECT window_area{};
        GetWindowRect(window, &window_area);

        POINT diff{};
        ClientToScreen(window, &diff);

        const MARGINS margins{
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom
        };

        DwmExtendFrameIntoClientArea(window, &margins);
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U;
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = window;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, levels, 2U, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &level, &device_context);

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer) {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else {
        return 1;
    }

    ShowWindow(window, cmd_show);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, device_context);

    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, instance, 0);

    bool running = true;

    while (running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                running = false;
            }
        }

        if (!running) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Inside your ImGui rendering loop
        if (showMenu) {
            ImGui::Begin("Menu");
            ImGui::Text("Press %s to hide this menu", ImGui::GetKeyName(VirtualKeyToImGuiKey(openKey)));
            if (ImGui::Button("Exit")) {
                exit(0);
            }
            if (ImGui::Checkbox("Enable Auto Clicker", &checkboxState)) {
                if (checkboxState) {
                    autoClickerRunning.store(true);
                    std::thread(AutoClicker, std::ref(autoClickerRunning)).detach();
                }
                else {
                    autoClickerRunning.store(false);
                }
            }
            if (ImGui::Checkbox("Keep shifting", &shiftcheckboxState)) {
                keepShifting = shiftcheckboxState;
            }
            ImGui::End();
        }

        ImGui::Render();

        const float color[4]{ 0.f, 0.f, 0.f, 0.f };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1U, 0U);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    render_target_view->Release();
    swap_chain->Release();
    device_context->Release();
    device->Release();

    UnhookWindowsHookEx(hKeyboardHook);

    return 0;
}
