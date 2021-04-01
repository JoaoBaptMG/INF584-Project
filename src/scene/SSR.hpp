#pragma once

#include "resources/Texture.hpp"
#include "resources/Framebuffer.hpp"
#include "wrappers/glfw.hpp"
#include "resources/Program.hpp"
#include "GBuffer.hpp"

namespace scene
{
    class SSR final
    {
        gl::Texture2D ssrTexcoord;
        gl::Texture2D ssrVisibility;
        gl::Framebuffer ssrFramebuffer;
        std::shared_ptr<gl::Program> ssrProgram;
        std::size_t width, height;

    public:
        SSR(const glfw::Size& size);

        void drawSSR(const GBuffer& gbuffer, const glm::mat4& projection);
        void clearSSR();
        void setTextureParam(gl::Program& program) const;
    };
}
