#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <d3d11.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <tlhelp32.h>
#include <streambuf>

//my headers
#include "ConfigManager.h"
#include "keys.h"
#include "autoclicker.h"
#include "globals.h"
#include "LowLevelKeyboardProc.h"
#include "discordgamecore.h"
#include "console.h"
#include "Modules/ServerBrowser.h"
#include "Modules/Serverinfo.h"
#include "Modules/Memory.h"

//imgui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

// Function prototypes
void RightClick();
void AutoClicker(std::atomic<bool>& running);
void Plsworkservers();

// Global variables for DirectX and ImGui
ID3D11Device* device = nullptr;
ID3D11DeviceContext* device_context = nullptr;
IDXGISwapChain* swap_chain = nullptr;
ID3D11RenderTargetView* render_target_view = nullptr;

static char detailtext[128] = "";
static char statetext[128] = "";
static const char* imgitems[] = {"20220827_102222", "spongebob_no"};
static int selectedItem = 0;

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

void UpdateDiscordPresence(const std::string& state, const std::string& details, const std::string imgId);
void InitDiscord();
void RunCallbacks();

Console console;

// Custom stream buffer to capture output
class ConsoleStreamBuf : public std::streambuf {
public:
    ConsoleStreamBuf(Console& console) : console(console) {}

protected:
    virtual int overflow(int c) override {
        if (c == '\n') {
            console.AddLog("%s", buffer.c_str());
            buffer.clear();
        }
        else {
            buffer += static_cast<char>(c);
        }
        return c;
    }

private:
    Console& console;
    std::string buffer;
};

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

    Console console;
    ConsoleStreamBuf consoleStreamBuf(console);

    // Redirect std::cout and std::cerr
    std::streambuf* origCoutBuf = std::cout.rdbuf(&consoleStreamBuf);
    std::streambuf* origCerrBuf = std::cerr.rdbuf(&consoleStreamBuf);

    InitDiscord();
    

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

        static bool show_console = true;
        console.Draw("Console", &show_console);

        // Inside your ImGui rendering loop
        if (showMenu) {
            SetForegroundWindow(window);
            ImGui::SetNextFrameWantCaptureKeyboard(true);
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

            if (ImGui::Button("")) {

            }
            ImGui::End();
            ImGui::Begin("discord");

            ImGui::InputText("Details", detailtext, IM_ARRAYSIZE(detailtext));
            ImGui::InputText("State", statetext, IM_ARRAYSIZE(statetext));
            ImGui::Combo("Select LargeImage", &selectedItem, imgitems, IM_ARRAYSIZE(imgitems));

            if (ImGui::Button("Set Discord Presence")) {
                UpdateDiscordPresence(statetext, detailtext, imgitems[selectedItem]);
            }

            ImGui::End();

            Plsworkservers();
        }   

        

        ImGui::Render();

        const float color[4]{ 0.f, 0.f, 0.f, 0.f };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1U, 0U);
        RunCallbacks();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    render_target_view->Release();
    swap_chain->Release();
    device_context->Release();
    device->Release();

    //UnhookWindowsHookEx(hKeyboardHook);

    return 0;
}
