// Author: Jake Rieger
// Created: 1/13/2025.
//

#include "Types.hpp"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace x {
    class Scene;
    class Camera;

    struct RenderTargets {
        ComPtr<ID3D11RenderTargetView> mainRTV;
        ComPtr<ID3D11ShaderResourceView> mainSRV;

        // G-Buffer components for deferred lighting
        struct GBuffer {
            ComPtr<ID3D11RenderTargetView> albedoRTV;    // R8G8B8A8_UNORM
            ComPtr<ID3D11RenderTargetView> normalRTV;    // R10G10B10A2_UNORM
            ComPtr<ID3D11RenderTargetView> materialRTV;  // R8G8B8A8_UNORM (Roughness, Metallic, AO)
            ComPtr<ID3D11RenderTargetView> emissiveRTV;  // R16G16B16A16_FLOAT (HDR emission)
            ComPtr<ID3D11DepthStencilView> depthDSV;     // D24_UNORM_S8_UINT

            // Corresponding SRVs for reading in lighting pass
            ComPtr<ID3D11ShaderResourceView> albedoSRV;
            ComPtr<ID3D11ShaderResourceView> normalSRV;
            ComPtr<ID3D11ShaderResourceView> materialSRV;
            ComPtr<ID3D11ShaderResourceView> emissiveSRV;
            ComPtr<ID3D11ShaderResourceView> depthSRV;
        } gBuffer;

        struct ShadowMaps {
            ComPtr<ID3D11DepthStencilView> cascadeDSV[4];
            ComPtr<ID3D11ShaderResourceView> cascadeSRV[4];
        } shadowMaps;

        struct GlobalIllumination {
            ComPtr<ID3D11RenderTargetView> lightProbeRTV;  // For light probe GI
            ComPtr<ID3D11ShaderResourceView> lightProbeSRV;
            ComPtr<ID3D11RenderTargetView> irradianceRTV;  // For irradiance caching
            ComPtr<ID3D11ShaderResourceView> irradianceSRV;
        } gi;

        struct PostProcess {
            ComPtr<ID3D11RenderTargetView> bloomRTV;  // For HDR bloom
            ComPtr<ID3D11ShaderResourceView> bloomSRV;
            ComPtr<ID3D11RenderTargetView> ldrTargetRTV;  // Final LDR output
            ComPtr<ID3D11ShaderResourceView> ldrTargetSRV;
        } postProcess;
    };

    class RenderPipeline {
    public:
        void Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
            CreateRenderTargetViews();
            CreateSamplers();
            CreateConstantBuffers();
            InitializeShadowMapping();
            InitializeGI();
            InitializePostProcess();
        }

        void Render(/*Scene* scene, Camera* camera*/);

    private:
        void RenderShadowMaps();
        void RenderGBuffer();
        void UpdateGI();
        void RenderLighting();
        void RenderTransparentObjects();
        void PostProcess();

        void CreateRenderTargetViews();
        void CreateSamplers();
        void CreateConstantBuffers();
        void InitializeShadowMapping();
        void InitializeGI();
        void InitializePostProcess();
    };

    struct DisplayMode {
        DXGI_MODE_DESC mode;
        str description;

        bool operator==(const DisplayMode& other) const {
            return mode.Width == other.mode.Width && mode.Height == other.mode.Height &&
                   mode.RefreshRate.Numerator == other.mode.RefreshRate.Numerator &&
                   mode.RefreshRate.Denominator == other.mode.RefreshRate.Denominator;
        }
    };

    class GameEngine {
    public:
        GameEngine();
        ~GameEngine();

        bool Initialize(HINSTANCE hInstance,
                        int nCmdShow,
                        int windowWidth  = 1280,
                        int windowHeight = 720);
        void Run();

        bool SetResolution(u32 width, u32 height);
        bool SetDisplayMode(const DisplayMode& mode);
        bool ToggleFullscreen();

        const vector<DisplayMode>& GetSupportedDisplayModes() const {
            return _supportedModes;
        }
        DisplayMode GetCurrentDisplayMode() const {
            return _currentMode;
        }

    private:
        bool InitializeWindow(HINSTANCE hInstance, int nCmdShow);
        bool InitializeDirectX();
        void Update();
        void Render();
        void Cleanup();

        // Window event callbacks
        void OnResize(u32 width, u32 height);
        void OnKeyDown(u32 keycode);

        HWND _hwnd;
        int _width, _height;
        str _title;

        ComPtr<ID3D11Device> _device;
        ComPtr<ID3D11DeviceContext> _context;
        ComPtr<IDXGISwapChain> _swapChain;
        ComPtr<ID3D11RenderTargetView> _renderTargetView;

        ComPtr<IDXGIFactory> _factory;
        ComPtr<IDXGIAdapter> _adapter;
        ComPtr<IDXGIOutput> _output;

        vector<DisplayMode> _supportedModes;
        DisplayMode _currentMode;
        bool _fullscreen;
        RECT _clientRect;

        bool EnumerateDisplayModes();
        bool CreateDeviceAndSwapChain();
        bool ResizeSwapChain(u32 width, u32 height);
        void StoreWindowRect();

        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };

    bool GameEngine::EnumerateDisplayModes() {
        u32 numModes       = 0;
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
        HRESULT hr         = _output->GetDisplayModeList(format, 0, &numModes, None);
        if (FAILED(hr)) { return false; }

        vector<DXGI_MODE_DESC> modes(numModes);
        hr = _output->GetDisplayModeList(format, 0, &numModes, modes.data());
        if (FAILED(hr)) { return false; }

        _supportedModes.clear();
        for (const auto& mode : modes) {
            if (mode.RefreshRate.Denominator == 0) continue;
            f32 refreshRate = CAST<f32>(mode.RefreshRate.Numerator) / mode.RefreshRate.Denominator;
            if (refreshRate < 30.0f || refreshRate > 360.0f) continue;

            DisplayMode displayMode;
            displayMode.mode = mode;

            char desc[128];
            sprintf_s(desc, "%dx%d @%.1fHz", mode.Width, mode.Height, refreshRate);
            displayMode.description = desc;
            _supportedModes.push_back(displayMode);
        }

        return !_supportedModes.empty();
    }

    bool GameEngine::CreateDeviceAndSwapChain() {
        HRESULT hr =
          CreateDXGIFactory(__uuidof(IDXGIFactory), RCAST<void**>(_factory.GetAddressOf()));
        if (FAILED(hr)) { return false; }

        hr = _factory->EnumAdapters(0, _adapter.GetAddressOf());
        if (FAILED(hr)) { return false; }

        hr = _adapter->EnumOutputs(0, _output.GetAddressOf());
        if (FAILED(hr)) { return false; }

        if (!EnumerateDisplayModes()) { return false; }

        DXGI_OUTPUT_DESC outputDesc = {};
        _output->GetDesc(&outputDesc);
        _width  = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
        _height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

        DXGI_SWAP_CHAIN_DESC scd               = {};
        scd.BufferCount                        = 2;  // Double buffering
        scd.BufferDesc.Width                   = _width;
        scd.BufferDesc.Height                  = _height;
        scd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.RefreshRate.Numerator   = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow                       = _hwnd;
        scd.SampleDesc.Count                   = 1;
        scd.SampleDesc.Quality                 = 0;
        scd.Windowed                           = TRUE;
        scd.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};

        hr = D3D11CreateDeviceAndSwapChain(None,
                                           D3D_DRIVER_TYPE_HARDWARE,
                                           None,
                                           D3D11_CREATE_DEVICE_DEBUG,
                                           featureLevels,
                                           1,
                                           D3D11_SDK_VERSION,
                                           &scd,
                                           &_swapChain,
                                           &_device,
                                           None,
                                           &_context);

        if (FAILED(hr)) { return false; }

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateRenderTargetView(backBuffer.Get(), None, &_renderTargetView);
        if (FAILED(hr)) { return false; }

        return true;
    }

    bool GameEngine::ResizeSwapChain(u32 width, u32 height) {
        if (!_swapChain || width == 0 || height == 0) { return false; }

        _context->OMSetRenderTargets(0, None, None);
        _renderTargetView.Reset();

        HRESULT hr = _swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) { return false; }

        ComPtr<ID3D11Texture2D> backBuffer;
        hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
        if (FAILED(hr)) { return false; }

        hr = _device->CreateRenderTargetView(backBuffer.Get(), None, &_renderTargetView);
        if (FAILED(hr)) { return false; }

        D3D11_VIEWPORT viewport = {};
        viewport.Width          = CAST<f32>(width);
        viewport.Height         = CAST<f32>(height);
        viewport.MinDepth       = 0.0f;
        viewport.MaxDepth       = 1.0f;
        _context->RSSetViewports(1, &viewport);

        _width  = width;
        _height = height;

        return true;
    }

    void GameEngine::StoreWindowRect() {
        GetWindowRect(_hwnd, &_clientRect);
    }

    bool GameEngine::SetResolution(u32 width, u32 height) {
        for (const auto& mode : _supportedModes) {
            if (mode.mode.Width == width && mode.mode.Height == height) {
                return SetDisplayMode(mode);
            }
        }
        return false;
    }

    bool GameEngine::SetDisplayMode(const DisplayMode& mode) {
        const auto it = std::find(_supportedModes.begin(), _supportedModes.end(), mode);
        if (it == _supportedModes.end()) return false;

        _currentMode = mode;
        return ResizeSwapChain(mode.mode.Width, mode.mode.Height);
    }

    bool GameEngine::ToggleFullscreen() {
        BOOL fullscreen = FALSE;
        _swapChain->GetFullscreenState(&fullscreen, None);

        if (!fullscreen) { StoreWindowRect(); }

        HRESULT hr = _swapChain->SetFullscreenState(!fullscreen, None);
        if (FAILED(hr)) { return false; }

        if (fullscreen) {
            // Restore windowed mode
            SetWindowLongPtr(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
            SetWindowPos(_hwnd,
                         HWND_TOP,
                         _clientRect.left,
                         _clientRect.top,
                         _clientRect.right - _clientRect.left,
                         _clientRect.bottom - _clientRect.top,
                         SWP_FRAMECHANGED);
        } else {
            // Remove window border in fullscreen
            SetWindowLongPtr(_hwnd, GWL_STYLE, WS_POPUP);
            SetWindowPos(_hwnd,
                         HWND_TOP,
                         0,
                         0,
                         _currentMode.mode.Width,
                         _currentMode.mode.Height,
                         SWP_FRAMECHANGED);
        }

        _fullscreen = !fullscreen;
        return true;
    }

    GameEngine::GameEngine()
        : _hwnd(None), _width(1280), _height(720), _title("XenDX | Testbed"), _currentMode({}),
          _fullscreen(true), _clientRect({}) {}

    GameEngine::~GameEngine() {
        Cleanup();
    }

    bool
    GameEngine::Initialize(HINSTANCE hInstance, int nCmdShow, int windowWidth, int windowHeight) {
        _width  = windowWidth;
        _height = windowHeight;
        if (!InitializeWindow(hInstance, nCmdShow)) { return false; }
        if (!InitializeDirectX()) return false;
        return true;
    }

    void GameEngine::Run() {
        MSG msg = {};
        while (msg.message != WM_QUIT) {
            if (PeekMessageA(&msg, None, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            } else {
                Update();
                Render();
            }
        }
    }

    bool GameEngine::InitializeWindow(HINSTANCE hInstance, int nCmdShow) {
        WNDCLASSEXA wc   = {};
        wc.cbSize        = sizeof(WNDCLASSEXA);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = hInstance;
        wc.hCursor       = LoadCursorA(None, IDC_ARROW);
        wc.lpszClassName = "GameEngineClass";
        RegisterClassExA(&wc);

        DWORD style   = WS_OVERLAPPEDWINDOW;
        DWORD exStyle = WS_EX_APPWINDOW;

        RECT clientRect = {0, 0, _width, _height};
        AdjustWindowRectEx(&clientRect, style, false, exStyle);
        RECT workArea;
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &workArea, 0);
        const i32 x = workArea.left +
                      (workArea.right - workArea.left - (clientRect.right - clientRect.left)) / 2;
        const i32 y = workArea.top +
                      (workArea.bottom - workArea.top - (clientRect.bottom - clientRect.top)) / 2;

        _hwnd = CreateWindowExA(exStyle,
                                wc.lpszClassName,
                                _title.c_str(),
                                style,
                                x,
                                y,
                                clientRect.right - clientRect.left,
                                clientRect.bottom - clientRect.top,
                                None,
                                None,
                                hInstance,
                                this);

        if (!_hwnd) { return false; }

        ShowWindow(_hwnd, nCmdShow);
        UpdateWindow(_hwnd);

        return true;
    }

    bool GameEngine::InitializeDirectX() {
        return CreateDeviceAndSwapChain();
    }  // namespace x

    void GameEngine::Update() {
        // Game logic
    }

    void GameEngine::Render() {
        constexpr f32 clearColor[4] = {0.0f, 0.2f, 0.4f, 1.0f};
        _context->ClearRenderTargetView(_renderTargetView.Get(), clearColor);
        std::ignore = _swapChain->Present(1, 0);
    }

    void GameEngine::Cleanup() {
        if (_context) _context->ClearState();
    }

    void GameEngine::OnResize(u32 width, u32 height) {
        if (width == _width && height == _height) { return; }
        ResizeSwapChain(width, height);
    }

    void GameEngine::OnKeyDown(u32 keycode) {
        if (keycode == VK_ESCAPE) {
            PostQuitMessage(0);
            return;
        }

        if (keycode == VK_F11) { ToggleFullscreen(); }
    }

    LRESULT GameEngine::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            case WM_SIZE: {
                const u32 width  = (u32)LOWORD(lParam);
                const u32 height = (u32)HIWORD(lParam);
                if (width == 0 || height == 0) { return 0; }
                OnResize(width, height);
            }
                return 0;

            case WM_KEYDOWN:
                OnKeyDown((u32)wParam);

            default:
                break;
        }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }

    LRESULT GameEngine::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        GameEngine* engine = None;

        if (msg == WM_CREATE) {
            const auto pCreate = RCAST<CREATESTRUCTA*>(lParam);
            engine             = CAST<GameEngine*>(
              pCreate->lpCreateParams);  // I don't think you need reinterpret cast here
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, RCAST<LONG_PTR>(engine));
        } else {
            engine = RCAST<GameEngine*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (engine) { return engine->HandleMessage(hwnd, msg, wParam, lParam); }

        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}  // namespace x

int main() {
    x::GameEngine engine;
    if (!engine.Initialize(GetModuleHandleA(x::None), SW_SHOWMAXIMIZED, 1280, 720)) {
        return EXIT_FAILURE;
    }
    engine.Run();

    return 0;
}