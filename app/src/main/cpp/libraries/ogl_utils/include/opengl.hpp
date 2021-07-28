#pragma once

#include <GLES3/gl3.h>

#include "singleton.hpp"

namespace bnb::gl
{
    enum class mali_gpu_family
    {
        generic, // all not listed
        bifrost,
    };

    enum class adreno_gpu_family
    {
        generic,
        _61x,
    };

    class context_info : public bnb::singleton<context_info>
    {
    public:
        struct caps_t
        {
            GLint max_texture_size{1024};
            bool has_rgba16f{false};
        } mutable caps;

        struct quirks_t
        {
            bool msaa_bug{false}; // Adreno (TM) 616, 615, 612 crashes on rendering into MSAA RG16F target
        } quirks;

        std::pair<int, int> gl_version{};

    public:
        context_info();
        virtual ~context_info() = default;

        void check_error(const char* file, int line);

    private:
        bool is_rgba16f_available();

        const char* error_code_to_string(GLenum error_code) const;
        void on_error(GLenum error_code, const char* file, int line);
    };

} // namespace bnb::gl

#define GL_CHECK_ERROR() bnb::gl::context_info::instance().check_error(__FILE__, __LINE__)
#define GL_CALL(FUNC) [&]() {FUNC; GL_CHECK_ERROR(); }()

#define BNB_GL_INIT() ((void) 0)
#define BNB_GL_START_GROUP(name) ((void) 0)
#define BNB_GL_END_GROUP() ((void) 0)
#define BNB_GL_LABEL(obj, name) ((void) 0)

#define BNB_GL_SCOPE(name) ((void) 0)
