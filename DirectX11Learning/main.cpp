#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include <memory>

#include "terrain_app.hpp"
#include "box_app.hpp"
#include "shapes_app.hpp"
#include "pyramide_app.hpp"
#include "skull_app.hpp"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    std::unique_ptr<lea::App> app = std::make_unique<lea::SkullApp>();
    try {
        app->Run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        Sleep(100000);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}