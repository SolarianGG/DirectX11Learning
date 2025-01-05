#include "lea_window.hpp"

#include <stdexcept>
#include <SDL2/SDL_syswm.h>

#include "lea_timer.hpp"

lea::LeaWindow::LeaWindow(int width, int height)
{
    LeaCreateWindow(width, height);
}

lea::LeaWindow::~LeaWindow()
{
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

int lea::LeaWindow::Width() const
{
    int width;
    SDL_GetWindowSize(window_, &width, nullptr);
    return width;
}

int lea::LeaWindow::Height() const
{
    int height;
    SDL_GetWindowSize(window_, nullptr, &height);
    return height;
}

bool lea::LeaWindow::ShouldClose()
{
    if (SDL_PollEvent(&e_))
    {
        if (e_.type == SDL_QUIT)
        {
            return true;
        }

        if (e_.type == SDL_WINDOWEVENT)
        {
            if (e_.window.event == SDL_WINDOWEVENT_MINIMIZED)
            {
                isActive_ = false;
                TIMER.Stop();
            }
            else if (e_.window.event == SDL_WINDOWEVENT_RESTORED)
            {
                isActive_ = true;
                TIMER.Start();
            }
        }
    }
    return false;
}

void lea::LeaWindow::SetTitle(std::wstring_view title)
{
    std::string titleStr(title.begin(), title.end());
    SDL_SetWindowTitle(window_, titleStr.c_str());
}

lea::LeaEvent lea::LeaWindow::GetCurrentEvent()
{
    LeaEvent event;

    mouse_button_state_ = SDL_GetMouseState(&event.mouse_x, &event.mouse_y);

    switch (e_.type) {
    case SDL_MOUSEMOTION:
        event.type = LeaEvent::TypeMouseMotion;
        if (mouse_button_state_ & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            event.key = LeaEvent::KeyLeftMouse;
        }
        else if (mouse_button_state_ & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            event.key = LeaEvent::KeyRightMouse;
        }
        else if (mouse_button_state_ & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
            event.key = LeaEvent::KeyMiddleMouse;
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        event.type = LeaEvent::TypeMouseDown;
        event.key = MapMouseButtonToLeaKey(e_.button.button);
        break;

    case SDL_MOUSEBUTTONUP:
        event.type = LeaEvent::TypeMouseUp;
        event.key = MapMouseButtonToLeaKey(e_.button.button);
        break;

    case SDL_KEYDOWN:
        event.type = LeaEvent::TypePressed;
        event.key = MapSDLKeyToLeaKey(e_.key.keysym.sym);
        break;

    case SDL_KEYUP:
        event.type = LeaEvent::TypeReleased;
        event.key = MapSDLKeyToLeaKey(e_.key.keysym.sym);
        break;

    default:
        event.type = LeaEvent::TypeUndefined;
        break;
    }

    return event;
}

void lea::LeaWindow::LeaCreateWindow(int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("Failed to init SDL\n");
    }

    window_ = SDL_CreateWindow("DirectX + SDL", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("Failed to create window\n");
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window_, &wmInfo)) {
        SDL_Log("Failed to get WMInfo: %s", SDL_GetError());
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("Failed to get WMInfo");
    }

    hwnd_ = wmInfo.info.win.window;
}

inline lea::LeaEvent::Key lea::LeaWindow::MapSDLKeyToLeaKey(SDL_Keycode sdl_key) const
{
    using namespace lea;
    switch (sdl_key) {
    case SDLK_w: case SDLK_UP:    return LeaEvent::KeyUp;
    case SDLK_s: case SDLK_DOWN:  return LeaEvent::KeyDown;
    case SDLK_a: case SDLK_LEFT:  return LeaEvent::KeyLeft;
    case SDLK_d: case SDLK_RIGHT: return LeaEvent::KeyRight;
    case SDLK_RETURN:             return LeaEvent::KeyEnter;
    case SDLK_ESCAPE:             return LeaEvent::KeyEscape;
    case SDLK_SPACE:              return LeaEvent::KeySpace;
    case SDLK_1:              return LeaEvent::KeyOne;
    case SDLK_2:              return LeaEvent::KeyTwo;
    case SDLK_3:              return LeaEvent::KeyThree;
    case SDLK_4:              return LeaEvent::KeyFour;
    case SDLK_5:              return LeaEvent::KeyFive;
    case SDLK_6:              return LeaEvent::KeySix;
    case SDLK_7:              return LeaEvent::KeySeven;
    case SDLK_8:              return LeaEvent::KeyEight;
    case SDLK_9:              return LeaEvent::KeyNine;
    case SDLK_0:              return LeaEvent::KeyZero;
    default:                      return LeaEvent::KeyUndefined;
    }
}

inline lea::LeaEvent::Key lea::LeaWindow::MapMouseButtonToLeaKey(Uint8 button) const
{
    using namespace lea;
    switch (button) {
    case SDL_BUTTON_LEFT:   return LeaEvent::KeyLeftMouse;
    case SDL_BUTTON_RIGHT:  return LeaEvent::KeyRightMouse;
    case SDL_BUTTON_MIDDLE: return LeaEvent::KeyMiddleMouse;
    default:                return LeaEvent::KeyUndefined;
    }
}
