#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

#include <string_view>

namespace lea {

	class LeaWindow {

		HWND hwnd_;
		HINSTANCE hinstance_;

		MSG msg_{ 0 };

		bool isActive_ = true;
	public:
		LeaWindow(HINSTANCE hInstance, int width, int height);

		HWND HWND() const { return hwnd_; }

		int Width() const;

		int Height() const;

		float AspectRatio() const { return static_cast<float>(Width()) / Height(); }

		bool ShouldClose();

		void SetTitle(std::wstring_view title);

		bool IsActive() const { return isActive_; }

		LRESULT MsgProc(::HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		void LeaCreateWindow(int width, int height);
	};

}