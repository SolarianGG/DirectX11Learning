#include "app.hpp"

#include <stdexcept>
#include <iostream>
#include <cstdlib>



int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    lea::App app{ hInstance };
    try {
        app.Run();
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        Sleep(100000);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}