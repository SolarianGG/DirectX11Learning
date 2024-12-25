#pragma once

#include "lea_window.hpp"
#include "lea_engine_device.hpp"

namespace lea {

	class App {
		
	public:
		static inline constexpr auto WIDTH = 1024;
		static inline constexpr auto HEIGHT = 768;

		App();
		App(const App& other) = delete;
		App& operator=(const App& other) = delete;

		App(App&& other) noexcept = delete;
		App& operator=(App&& other) noexcept = delete;
		virtual ~App();
		void Run();

	protected:
		LeaWindow window_;
		LeaDevice device_;

		bool isAppPaused_ = false;

		virtual void Init() = 0;

		void CalculateFrameStats();

		virtual void PollEvents() = 0;

		virtual void UpdateScene(float deltaTime) = 0;

		virtual void DrawScene() = 0;
	};
}