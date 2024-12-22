#pragma once

#include "lea_window.hpp"
#include "lea_engine_device.hpp"

namespace lea {


	class App {
		LeaWindow window_;
		LeaDevice device_;

	public:
		static inline constexpr auto WIDTH = 800;
		static inline constexpr auto HEIGHT = 600;

		App(HINSTANCE hInstance);
		App(const App& other) = delete;
		App& operator=(const App& other) = delete;

		App(App&& other) noexcept = delete;
		App& operator=(App&& other) noexcept = delete;
		~App();
		void Run();
	};
}