#include "lea_window.hpp"

#include <stdexcept>

namespace {
    constexpr auto class_name = TEXT("WINCLASS1");

    HINSTANCE hInstanceMain;

    HWND mainWindowHandle;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // TODO: messages handle

        switch (message)
        {
        case WM_ACTIVATE: {
            return 0;
        } break;
        case WM_CLOSE: {
            int result = MessageBox(hWnd,
                TEXT("Are you sure that you want to close the window?"),
                TEXT("Close?"), MB_YESNO | MB_ICONQUESTION);

            return result == IDYES ? DefWindowProc(hWnd, message, wParam, lParam) : 0;
        } break;
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        } break;
        default: break;
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

}

lea::LeaWindow::LeaWindow(HINSTANCE hInstance, int width, int height)
    : hinstance_(hInstance), width_(width), height_(height)
{
    LeaCreateWindow();
}

bool lea::LeaWindow::ShouldClose()
{
    if (msg_.message != WM_QUIT)
    {
        if (PeekMessage(&msg_, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg_);

            DispatchMessage(&msg_);
        }
        
        return false;
    }
    return true;
}

void lea::LeaWindow::LeaCreateWindow()
{
    WNDCLASSEX winclass{
        sizeof(WNDCLASSEX),
        CS_VREDRAW | CS_HREDRAW | CS_OWNDC | CS_DBLCLKS,
        WndProc,
        0,
        0,
        hinstance_,
        LoadIcon(hinstance_, IDI_APPLICATION),
        LoadCursor(hinstance_, IDC_ARROW),
        static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)),
        NULL,
        class_name,
        LoadIcon(hinstance_, IDI_APPLICATION)
    };

    hInstanceMain = hinstance_;

    if (!RegisterClassExW(&winclass))
        throw std::runtime_error("Failed to register window");

    if (!(hwnd_ = CreateWindowExW(
        0, class_name, TEXT("Direct X Learning"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, width_, height_,
        nullptr, nullptr, hinstance_, nullptr
    )))
    {
        throw std::runtime_error("Failed to create window");
    }
    
    mainWindowHandle = hwnd_;
}
