#pragma once

#include <bnb/common_types.h>

#include "program.hpp"

#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <mutex>

namespace bnb {
    struct orient_format
    {
        bnb_image_orientation_t orientation;
        bool is_y_flip;
    };

    struct data_t
    {
        using type = uint8_t[];
        using pointer = uint8_t*;
        using uptr = std::unique_ptr<type, std::function<void(pointer)>>;
        uptr data;
        size_t size;
    };

    class ort_frame_surface_handler;

    class offscreen_render_target
    {
    public:
        offscreen_render_target(uint32_t width, uint32_t height);

        ~offscreen_render_target();

        void init();

        void deinit();

        void surface_changed(int32_t width, int32_t height);

        void activate_context();

        void deactivate_context();

        void prepare_rendering();

        void orient_image(orient_format orient);

        data_t read_current_buffer();

        int get_current_buffer_texture();

    private:
        void create_context();

        void generate_texture(GLuint &texture);

        void prepare_post_processing_rendering();

        void delete_textures();

        uint32_t m_width;
        uint32_t m_height;

        GLuint m_framebuffer{0};
        GLuint m_post_processing_framebuffer{0};
        GLuint m_offscreen_render_texture{0};
        GLuint m_offscreen_post_processuing_render_texture{0};

        GLuint m_active_texture{0};

        EGLDisplay m_display;
        EGLSurface m_surface;
        EGLContext m_context;

        std::unique_ptr<oep::program> m_program;
        std::unique_ptr<ort_frame_surface_handler> m_frame_surface_handler;

        std::once_flag m_init_flag;
        std::once_flag m_deinit_flag;
    };
} // bnb
