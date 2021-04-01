#include "SSR.hpp"

#include "Scene.hpp"
#include "resources/Cache.hpp"
#include <optional>

using namespace scene;

SSR::SSR(const glfw::Size& size) : width(size.width), height(size.height)
{
    ssrTexcoord.assign(0, gl::InternalFormat::RG16i, (GLsizei)width, (GLsizei)height);
    ssrTexcoord.setName("SSR Texcoord Texture");

    ssrVisibility.assign(0, gl::InternalFormat::R32f, (GLsizei)width, (GLsizei)height);
    ssrVisibility.setName("SSR Visibility Texture");

    for (auto texture : { &ssrTexcoord, &ssrVisibility })
    {
        texture->setMagFilter(gl::MagFilter::Nearest);
        texture->setMinFilter(gl::MinFilter::Nearest);
    }

    ssrFramebuffer.attach(gl::ColorAttachment(0), ssrTexcoord);
    ssrFramebuffer.attach(gl::ColorAttachment(1), ssrVisibility);
    ssrFramebuffer.setDrawBuffers(gl::ColorAttachment(0), gl::ColorAttachment(1));
    ssrFramebuffer.setName("SSR Framebuffer");

    ssrProgram = cache::loadProgram({ "resources/shaders/fullScreenQuad.vert", "resources/shaders/ssr.frag" });
}

void SSR::drawSSR(const GBuffer& gbuffer, const glm::mat4& projection)
{
    // Draw SSR
    clearSSR();

    ssrProgram->use();
    gbuffer.setParams(*ssrProgram, projection);
    ssrProgram->setUniform("Projection", projection);
    Scene::drawFullScreenQuad();
}

void SSR::clearSSR()
{
    ssrFramebuffer.bind();
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); gl::checkError();
    const GLint values[] = { -1, -1 };
    const float fval = 0.0f;
    glClearBufferiv(GL_COLOR, 0, values);
    glClearBufferfv(GL_COLOR, 1, &fval);
}

void SSR::setTextureParam(gl::Program& program) const
{
    ssrTexcoord.bindTo(7);
    program.setUniform("SSRTexcoordTexture", 7);
    ssrVisibility.bindTo(8);
    program.setUniform("SSRVisibilityTexture", 8);
}
