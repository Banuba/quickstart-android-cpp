// #include <bnb/utils/singleton.hpp>

#ifndef BNB_WRITE_LOG_MESSAGE
    #define WRITE_LOG_MESSAGE(severity, message) std::cout << #severity << message << std::endl
#else
    #define WRITE_LOG_MESSAGE(severity, message) BNB_WRITE_LOG_MESSAGE(severity) << message
#endif

#ifndef BNB_WRITE_LOG_MESSAGE_WITH_LOGGER
    #define WRITE_LOG_MESSAGE_WITH_LOGGER(logger, severity, message) std::cout << #severity << message << std::endl
#else
    #define WRITE_LOG_MESSAGE_WITH_LOGGER(logger, severity, message) BNB_WRITE_LOG_MESSAGE_WITH_LOGGER(logger, severity) << message
#endif

#include <iostream>
#include <string>
#include "opengl.hpp"

using namespace bnb;

gl::context_info::context_info()
{
    /* Context data queries */
    auto& [major, minor] = gl_version;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &caps.max_texture_size);
    caps.has_rgba16f = is_rgba16f_available();
}

bool gl::context_info::is_rgba16f_available()
{
    return true;
}

const char* gl::context_info::error_code_to_string(GLenum error_code) const
{
    switch (error_code) {
        // clang-format off
        case GL_INVALID_ENUM:                  return "INVALID_ENUM";
        case GL_INVALID_VALUE:                 return "INVALID_VALUE";
        case GL_INVALID_OPERATION:             return "INVALID_OPERATION";

#ifdef BNB_GL
        case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW";
#endif

        case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "UNKNOWN";
            // clang-format on
    }
}

void gl::context_info::on_error(GLenum error_code, const char* file, int line)
{
    WRITE_LOG_MESSAGE(
        warning, "glGetError: " << error_code_to_string(error_code) << " | " << file << " (" << line << ") ");
}

void gl::context_info::check_error(const char* file, int line)
{
    GLenum error_code;
    while ((error_code = glGetError()) != GL_NO_ERROR) {
        context_info::instance().on_error(error_code, file, line);
    }
}