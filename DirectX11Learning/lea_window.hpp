#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>


namespace lea {

	class LeaWindow {

		HWND hwnd_;
		HINSTANCE hinstance_;

		MSG msg_{ 0 };

		int width_, height_;
	public:
		LeaWindow(HINSTANCE hInstance, int width, int height);

		HWND HWND() const { return hwnd_; }

		int Width() const { return width_; }

		int Height() const { return height_; }

		bool ShouldClose();

	private:
		void LeaCreateWindow();
	};

}