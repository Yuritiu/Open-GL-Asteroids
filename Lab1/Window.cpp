#include "Window.h"
#include <stdexcept>

Window::Window(const std::string& title, int width, int height, Uint32 flags)
    : sdlWindow(nullptr), width(width), height(height) {
    sdlWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (!sdlWindow) {
        throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
    }
}

Window::~Window() {
    SDL_DestroyWindow(sdlWindow);
}

void Window::setTitle(const std::string& title)
{
    SDL_SetWindowTitle(sdlWindow, title.c_str());
}