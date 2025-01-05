#pragma once


#include <Windows.h>
#include <string_view>

#include <SDL2/SDL.h>

namespace lea {

	struct LeaEvent {
		enum Type {
			TypeUndefined,
			TypePressed,
			TypeReleased,
			TypeQuit,
			TypeMouseMotion,
			TypeMouseDown,
			TypeMouseUp
		} type = TypeUndefined;

		enum Key {
			KeyUndefined,
			KeyLeft,
			KeyRight,
			KeyUp,
			KeyDown,
			KeyEnter,
			KeyEscape,
			KeySpace,
			KeyLeftMouse,
			KeyRightMouse,
			KeyMiddleMouse,
			KeyOne,
			KeyTwo,
			KeyThree,
			KeyFour,
			KeyFive,
			KeySix,
			KeySeven,
			KeyEight,
			KeyNine,
			KeyZero
		} key = KeyUndefined;

		int mouse_x = 0;
		int mouse_y = 0;
	};


	class LeaWindow {
		HWND hwnd_;

		SDL_Window* window_ = nullptr;

		bool isActive_ = true;

		SDL_Event e_;
	public:
		LeaWindow(int width, int height);

		LeaWindow(const LeaWindow& other) = delete;
		LeaWindow& operator=(const LeaWindow& other) = delete;

		LeaWindow(LeaWindow&& other) noexcept = delete;
		LeaWindow& operator=(LeaWindow&& other) noexcept = delete;

		~LeaWindow();

		HWND HWND() const { return hwnd_; }

		SDL_Window* SDL_WINDOW() const { return window_; }

		SDL_Event SDL_EVENT() const { return e_; }

		int Width() const;

		int Height() const;

		float AspectRatio() const { return static_cast<float>(Width()) / Height(); }

		bool ShouldClose();

		void SetTitle(std::wstring_view title);

		bool IsActive() const { return isActive_; }

		lea::LeaEvent GetCurrentEvent();

	private:
		void LeaCreateWindow(int width, int height);

		Uint32 mouse_button_state_ = 0;
		lea::LeaEvent::Key MapSDLKeyToLeaKey(SDL_Keycode sdl_key) const;
		lea::LeaEvent::Key MapMouseButtonToLeaKey(Uint8 button) const;
	};

}