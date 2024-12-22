#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d11_1.h>
#include <wrl/client.h>

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

		bool enableMSAA = true;
		UINT msaaQuality_ = 4;
		static inline constexpr UINT MSAA_LEVEL = 4;
	public:
		LeaDevice(LeaWindow& window);

		void Clean();
	private:
		void InitDevice();

		void CreateSwapChain();

		void CreateDebugger();

		void CreateRenderTargetView();

		void CreateDepthStencil();

		void BindRenderTargets();
	};
}