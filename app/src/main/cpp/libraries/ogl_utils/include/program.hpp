#pragma once

#include <iostream>

namespace bnb::oep
{
    class program
    {
    public:
        program(const char* name, const char* vertex_shader_code, const char* fragmant_shader_code);
        ~program();

        void use() const;
        void unuse() const;

        unsigned int handle() const { return m_handle; }

    private:
        unsigned int m_handle;
    };
}
