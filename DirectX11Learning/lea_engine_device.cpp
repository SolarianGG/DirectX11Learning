#include "lea_engine_device.hpp"

#include <stdexcept>
#include <cassert>
#include <iostream>
#include <string>

#include <d3dcompiler.h>
#include <DirectXColors.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include "DXHelper.hpp"
#include "lea_timer.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Effects11d.lib")

using Microsoft::WRL::ComPtr;

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
    SetupViewport();
}

void lea::LeaDevice::Clean()
{

    if (context_)
    {
        context_->ClearState();
        context_->Flush();
    }
}

std::vector<ComPtr<IDXGIAdapter>> lea::LeaDevice::GetAdapters()
{
    ComPtr<IDXGIFactory> pFactory = nullptr;

    DX::ThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(pFactory.GetAddressOf())));
    std::vector<ComPtr<IDXGIAdapter>> vAdapters;
    UINT i = 0;
    ComPtr<IDXGIAdapter> pAdapter;
    while (pFactory->EnumAdapters(i, pAdapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(std::move(pAdapter));
        ++i;
    }
    return vAdapters;
}

void lea::LeaDevice::PrintInfoAboutAdapters(const std::vector<ComPtr<IDXGIAdapter>>& vAdapters)
{
    for (const auto& adapter : vAdapters)
    {
        DXGI_ADAPTER_DESC adapterDesc{};
        DX::ThrowIfFailed(adapter->GetDesc(&adapterDesc));

        std::wstring info;
        info += L"Adaptor name: " + std::wstring(adapterDesc.Description) + L"\n";
        info += L"VRAM: " + std::to_wstring(adapterDesc.DedicatedVideoMemory / (1024 * 1024)) + L" MB\n";
        info += L"System memory: " + std::to_wstring(adapterDesc.DedicatedSystemMemory / (1024 * 1024)) + L" MB\n";
        info += L"Common memory: " + std::to_wstring(adapterDesc.SharedSystemMemory / (1024 * 1024)) + L" MB\n";
        info += L"ID of device: " + std::to_wstring(adapterDesc.DeviceId) + L"\n";
        info += L"vendor ID: " + std::to_wstring(adapterDesc.VendorId) + L"\n";
        info += L"UUID: " + std::to_wstring(adapterDesc.AdapterLuid.LowPart) + L"-" + std::to_wstring(adapterDesc.AdapterLuid.HighPart) + L"\n";

        UINT i = 0;
        ComPtr<IDXGIOutput> pOutput;
        while (adapter->EnumOutputs(i, pOutput.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_OUTPUT_DESC outputDesc{};
            pOutput->GetDesc(&outputDesc);

            info += L"Device name: " + std::wstring(outputDesc.DeviceName) + L"\n";
            info += L"Rotation: " + std::to_wstring(outputDesc.Rotation) + L"\n";
            info += L"Attached to desktop: " + std::to_wstring(outputDesc.AttachedToDesktop) + L"\n";
            info += L"Desktop Coordinates: " +
                std::to_wstring(outputDesc.DesktopCoordinates.left) + L" " +
                std::to_wstring(outputDesc.DesktopCoordinates.right) + L" " +
                std::to_wstring(outputDesc.DesktopCoordinates.top) + L" " +
                std::to_wstring(outputDesc.DesktopCoordinates.bottom) + L"\n";

            UINT numModes = 0;
            DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

            DX::ThrowIfFailed(pOutput->GetDisplayModeList(format, 0, &numModes, nullptr));
            if (numModes > 0)
            {
                std::vector<DXGI_MODE_DESC> modeList(numModes);
                DX::ThrowIfFailed(pOutput->GetDisplayModeList(format, 0, &numModes, modeList.data()));

                info += L"Supported Display Modes:\n";
                for (const auto& mode : modeList)
                {
                    info += L"  Resolution: " + std::to_wstring(mode.Width) + L"x" + std::to_wstring(mode.Height) +
                        L", Refresh Rate: " + std::to_wstring(mode.RefreshRate.Numerator / mode.RefreshRate.Denominator) + L" Hz\n";
                }
            }
            else
            {
                info += L"No display modes found for DXGI_FORMAT_R8G8B8A8_UNORM.\n";
            }

            ++i;
        }
        OutputDebugString(info.c_str());
    }
}

void lea::LeaDevice::InitDevice()
{
    // DirectX
    auto vAdapters = GetAdapters();
   
#if defined(DEBUG) || defined(_DEBUG)
    PrintInfoAboutAdapters(vAdapters);
#endif

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

    D3D_FEATURE_LEVEL featureLevels = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL featureLevel;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // vAdapters[1].Get() && D3D_DRIVER_TYPE_UNKNOWN for nvidia card
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

    DXGI_SWAP_CHAIN_DESC sd{};
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

    assert(dxgiFactory.Get() != nullptr);
    DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(device_.Get(), &sd, swapChain_.GetAddressOf()));

    DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(window_.HWND(), DXGI_MWA_NO_WINDOW_CHANGES));
    DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(window_.HWND(), DXGI_MWA_NO_ALT_ENTER));
}

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
    D3D11_TEXTURE2D_DESC depthStencilDesc{};
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

inline void lea::LeaDevice::SetupViewport()
{
    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(window_.Width());
    vp.Height = static_cast<float>(window_.Height());
    vp.TopLeftX = 0.f;
    vp.TopLeftY = 0.f;
    vp.MinDepth = 0.f;
    vp.MaxDepth = 1.f;
    context_->RSSetViewports(1, &vp);

    debugger_->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
}

ID3D11ShaderResourceView* lea::LeaDevice::CreateTexture(std::wstring_view texture_file_name)
{
    ID3D11ShaderResourceView* shaderResourceView;
    if (texture_file_name.ends_with(L".dds"))
    {
        OutputDebugString(L"DDS texture has been provided");
        DX::ThrowIfFailed(CreateDDSTextureFromFile(device_.Get(), texture_file_name.data(), nullptr, &shaderResourceView));
    }
    else {
        OutputDebugString(L"NOT DDS texture has been provided");
        DX::ThrowIfFailed(CreateWICTextureFromFile(device_.Get(), texture_file_name.data(), nullptr, &shaderResourceView));
    }
    assert(shaderResourceView != nullptr);
    return shaderResourceView;
}

ID3DX11Effect* lea::LeaDevice::CreateEffect(const WCHAR* szFileName)
{
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)

    dwShaderFlags |= D3DCOMPILE_DEBUG;

    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif


    ComPtr<ID3DBlob> pErrorBlob;
    ComPtr<ID3DBlob> ppBlobout;
    HRESULT hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 0, "fx_5_0",
        dwShaderFlags, 0, ppBlobout.GetAddressOf(), pErrorBlob.GetAddressOf());
    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
        }
  
    }

    ID3DX11Effect* effect = nullptr;
    DX::ThrowIfFailed(
    D3DX11CreateEffectFromMemory(ppBlobout->GetBufferPointer(), ppBlobout->GetBufferSize(), 0, device_.Get(), &effect));
    return effect;
}

