#pragma once

#include <cstdint>
#include <memory>
#include "wrappers/glfw.hpp"
#include "resources/Mesh.hpp"
#include "resources/Program.hpp"
#include "resources/Framebuffer.hpp"
#include "Lighting.hpp"

namespace scene
{
    class GBuffer final
    {
        // Includes all the parameters for a GBuffer
        gl::Framebuffer framebuffer;
        gl::Texture2D colorTexture;
        gl::Texture2D depthTexture;
        gl::Texture2D normalTexture;
        gl::Texture2D specShinyTexture;
        std::size_t width, height;
        std::shared_ptr<gl::Program> gbufferProgram;

    public:
        GBuffer(glfw::Size size);
        ~GBuffer() {}

        void begin();
        void end();

        auto& getDrawProgram() const { return *gbufferProgram; }
        void setParams(gl::Program& program, const glm::mat4& projection) const;
    };
}