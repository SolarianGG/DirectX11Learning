#include "app.hpp"

#include "lea_timer.hpp"

#include <sstream>

lea::App::App(HINSTANCE hInstance)
	: window_(hInstance, WIDTH, HEIGHT), device_(window_)
{

}

lea::App::~App()
{
	device_.Clean();
}

void lea::App::CalculateFrameStats()
{
	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	if ((TIMER.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frameCnt);
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << "Main:" << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		window_.SetTitle(outs.str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void lea::App::UpdateScene(float deltaTime)
{

}

void lea::App::DrawScene()
{
	device_.Draw();
}

void lea::App::Run()
{
	TIMER.Reset();

	while (!window_.ShouldClose())
	{
		TIMER.Tick();
		if (!isAppPaused_ && window_.IsActive())
		{
			CalculateFrameStats();
			// TODO: Poll Events
			UpdateScene(TIMER.DeltaTime());
			DrawScene();
		}
		else
		{
			Sleep(100);
		}
	}
}
