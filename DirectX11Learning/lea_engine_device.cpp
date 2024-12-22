#include "lea_engine_device.hpp"

#include <stdexcept>
#include <cassert>

using Microsoft::WRL::ComPtr;

#include "DXHelper.hpp"

lea::LeaDevice::LeaDevice(LeaWindow& window)
    : window_(window)
{
    InitDevice();
    CreateSwapChain();
#if defined(DEBUG) || defined(_DEBUG)
    CreateDebugger();
#endif
    CreateRenderTargetView();
    CreateDepthStencil();
    BindRenderTargets();
}

void lea::LeaDevice::Clean()
{
    context_->ClearState();
}

void lea::LeaDevice::InitDevice()
{
    // DirectX

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

    D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL featureLevel;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    DX::ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, &featureLevels, 1,
        D3D11_SDK_VERSION, device_.GetAddressOf(), &featureLevel, context_.GetAddressOf()));

    if ((featureLevel != D3D_FEATURE_LEVEL_11_0))
    {
        throw std::runtime_error("Direct3D feature level unsupported");
    }
}

void lea::LeaDevice::CreateSwapChain()
{
    DX::ThrowIfFailed(device_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, MSAA_LEVEL, &msaaQuality_));

    assert(msaaQuality_ > 0 || !enableMSAA);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = window_.Width();
    sd.BufferDesc.Height = window_.Height();
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    if (enableMSAA && msaaQuality_ > 0) {
        sd.SampleDesc.Count = MSAA_LEVEL;
        sd.SampleDesc.Quality = msaaQuality_ - 1;

    }
    else {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window_.HWND();
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;
    sd.Windowed = true;
    
    ComPtr<IDXGIDevice> dxgiDevice;
    DX::ThrowIfFailed(device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(dxgiDevice.GetAddressOf())));

    ComPtr<IDXGIAdapter> dxgiAdapter;
    DX::ThrowIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(dxgiAdapter.GetAddressOf())));

    ComPtr<IDXGIFactory> dxgiFactory;
    DX::ThrowIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(dxgiFactory.GetAddressOf())));

    assert(dxgiFactory != nullptr);
    DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(device_.Get(), &sd, swapChain_.GetAddressOf()));
}

// NOTE: PRACTICE USING DEBUGGER | ASK ChatGPT
void lea::LeaDevice::CreateDebugger()
{
    DX::ThrowIfFailed(device_->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(debugger_.GetAddressOf())));
}

void lea::LeaDevice::CreateRenderTargetView()
{
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf())));
    assert(backBuffer != nullptr);
    DX::ThrowIfFailed(device_->CreateRenderTargetView(backBuffer.Get(), 0, renderTargetView_.GetAddressOf()));

}

void lea::LeaDevice::CreateDepthStencil()
{

    D3D11_TEXTURE2D_DESC depthStencilDesc;
    ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
    depthStencilDesc.Width = window_.Width();
    depthStencilDesc.Height = window_.Height();
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    if (enableMSAA && msaaQuality_ > 0)
    {
        depthStencilDesc.SampleDesc.Count = MSAA_LEVEL;
        depthStencilDesc.SampleDesc.Quality = msaaQuality_ - 1;
    }
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    DX::ThrowIfFailed(device_->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer_.GetAddressOf()));

    DX::ThrowIfFailed(device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, depthStencilView_.GetAddressOf()));
}

inline void lea::LeaDevice::BindRenderTargets()
{
    ID3D11RenderTargetView* const targets[1] = { renderTargetView_.Get() };
    context_->OMSetRenderTargets(1, targets, depthStencilView_.Get());
}
