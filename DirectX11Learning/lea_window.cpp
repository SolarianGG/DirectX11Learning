#include "lea_window.hpp"

#include <stdexcept>

#include "lea_timer.hpp"


namespace {
    constexpr auto class_name = TEXT("WINCLASS1");

    HINSTANCE hInstanceMain;

    HWND mainWindowHandle;

    lea::LeaWindow *gwnd = 0;

    LRESULT CALLBACK WndProc(::HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return gwnd->MsgProc(hWnd, message, wParam, lParam);
    }
}

LRESULT lea::LeaWindow::MsgProc(::HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // TODO: messages handle
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_ACTIVATE: {
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            isActive_ = false;
            TIMER.Stop();
        }
        else
        {
            isActive_ = true;
            TIMER.Start();
        }
        return 0;
    } break;
    case WM_ENTERSIZEMOVE: {
        isActive_ = false;
        TIMER.Stop();
        return 0;
    } break;
    case WM_EXITSIZEMOVE: {
        isActive_ = true;
        TIMER.Start();
        return 0;
    } break;
    case WM_PAINT: {
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
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

lea::LeaWindow::LeaWindow(HINSTANCE hInstance, int width, int height)
    : hinstance_(hInstance)
{
    gwnd = this;
    LeaCreateWindow(width, height);
}

int lea::LeaWindow::Width() const
{
    RECT rc;
    GetClientRect(hwnd_, &rc);

    return rc.right - rc.left;
}

int lea::LeaWindow::Height() const
{
    RECT rc;
    GetClientRect(hwnd_, &rc);

    return rc.bottom - rc.top;
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

void lea::LeaWindow::SetTitle(std::wstring_view title)
{
    SetWindowText(hwnd_, title.data());
}

void lea::LeaWindow::LeaCreateWindow(int width, int height)
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


    RECT rc = { 0, 0, width, height };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    if (!(hwnd_ = CreateWindowExW(0, class_name, L"Direct3D 11 Tutorial 2: Rendering a Triangle",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hinstance_,
        nullptr
    )))
    {
        throw std::runtime_error("Failed to create window");
    }
    
    ShowWindow(hwnd_, 10);

    mainWindowHandle = hwnd_;
}
