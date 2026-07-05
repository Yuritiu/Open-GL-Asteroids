#include "OpenGLContext.h"
#include <stdexcept>

OpenGLContext::OpenGLContext(SDL_Window* window) {
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        throw std::runtime_error("Failed to create OpenGL context: " + std::string(SDL_GetError()));
    }

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW: " + std::string(reinterpret_cast<const char*>(glewGetErrorString(error))));
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

OpenGLContext::~OpenGLContext() {
    SDL_GL_DeleteContext(glContext);
}

void OpenGLContext::swapBuffers(SDL_Window* window) {
    SDL_GL_SwapWindow(window);
}
