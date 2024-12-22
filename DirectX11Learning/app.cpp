#include "app.hpp"

lea::App::App(HINSTANCE hInstance)
	: window_(hInstance, WIDTH, HEIGHT), device_(window_)
{

}

lea::App::~App()
{
	device_.Clean();
}

void lea::App::Run()
{
	while (!window_.ShouldClose())
	{
		// TODO: Poll Events
		// TODO: Updage logic
		// TODO: Draw Frame
	}
}
