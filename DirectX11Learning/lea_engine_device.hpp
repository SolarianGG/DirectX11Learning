#pragma once

#include <d3d11_1.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <vector>

#include <d3dx11effect.h>
#include <d3dxGlobal.h>
#include <functional>

#include "lea_engine_utils.hpp"
#include "lea_window.hpp"

namespace lea {

	using Microsoft::WRL::ComPtr;

	class LeaDevice {

		ComPtr<ID3D11Device> device_;
		ComPtr<ID3D11DeviceContext> context_;
		ComPtr<IDXGISwapChain> swapChain_;

		ComPtr<ID3D11Debug> debugger_;

		ComPtr<ID3D11RenderTargetView> renderTargetView_;

		ComPtr<ID3D11Texture2D> depthStencilBuffer_;
		ComPtr<ID3D11DepthStencilView> depthStencilView_;

		LeaWindow& window_;

		bool enableMSAA = false;
		UINT msaaQuality_ = 4;
		static inline constexpr UINT MSAA_LEVEL = 4;
	public:
		LeaDevice(LeaWindow& window);

		ID3D11Device* Device() { return device_.Get(); }
		ID3D11DeviceContext* Context() { return context_.Get(); }
		IDXGISwapChain* SwapChain() { return swapChain_.Get(); }
		ID3D11RenderTargetView* RenderTargetView() { return renderTargetView_.Get(); }
		ID3D11DepthStencilView* DepthStencilView() { return depthStencilView_.Get(); }

		ID3DX11Effect* CreateEffect(const WCHAR* szFileName);

		void Clean();
	private:
		std::vector<ComPtr<IDXGIAdapter>> GetAdapters();

		void PrintInfoAboutAdapters(const std::vector<ComPtr<IDXGIAdapter>>& vAdapters);

		void InitDevice();

		void CreateSwapChain();

		void CreateDebugger();

		void CreateRenderTargetView();

		void CreateDepthStencil();

		void BindRenderTargets();

		void SetupViewport();
	};
}