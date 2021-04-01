#include <glad/glad.h>
#include "wrappers/glException.hpp"

#include <iostream>

void enableOpenGLErrorHandler()
{
    glEnable(GL_DEBUG_OUTPUT); gl::checkError();
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); gl::checkError();

    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam)
        {
            const char* strSource = "";
            switch (source)
            {
            case GL_DEBUG_SOURCE_API: strSource = "API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: strSource = "window system"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: strSource = "shader compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY: strSource = "third party"; break;
            case GL_DEBUG_SOURCE_APPLICATION: strSource = "application"; break;
            case GL_DEBUG_SOURCE_OTHER: strSource = "other"; break;
            }

            const char* strType = "";
            switch (type)
            {
            case GL_DEBUG_TYPE_ERROR: strType = "error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: strType = "deprecated behavior"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: strType = "undefined behavior"; break;
            case GL_DEBUG_TYPE_PORTABILITY: strType = "portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE: strType = "performance"; break;
            case GL_DEBUG_TYPE_MARKER: strType = "marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP: strType = "push group"; break;
            case GL_DEBUG_TYPE_POP_GROUP: strType = "pop group"; break;
            case GL_DEBUG_TYPE_OTHER: strType = "other"; break;
            }

            const char* strSeverity = "";
            switch (severity)
            {
            case GL_DEBUG_SEVERITY_HIGH: strSeverity = "HIGH"; break;
            case GL_DEBUG_SEVERITY_MEDIUM: strSeverity = "MEDIUM"; break;
            case GL_DEBUG_SEVERITY_LOW: strSeverity = "LOW"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: strSeverity = "notification"; break;
            }

            std::cout << '[' << strSeverity << "] " << strType << " (" << strSource << "): ";
            std::cout << std::string_view(message, length) << std::endl;
        }, nullptr);
}
