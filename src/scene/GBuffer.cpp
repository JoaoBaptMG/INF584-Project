#include "GBuffer.hpp"

#include "wrappers/glException.hpp"
#include "resources/Cache.hpp"

using namespace scene;

GBuffer::GBuffer(glfw::Size size) : width(size.width), height(size.height)
{
    // Assign all textures
    colorTexture.assign(0, gl::InternalFormat::RGBA, size.width, size.height);
    depthTexture.assign(0, gl::InternalFormat::Depth32f, size.width, size.height);
    normalTexture.assign(0, gl::InternalFormat::RG16f, size.width, size.height);
    specShinyTexture.assign(0, gl::InternalFormat::RG16f, size.width, size.height);

    colorTexture.setName("G-Buffer Color Texture");
    depthTexture.setName("G-Buffer Depth Texture");
    normalTexture.setName("G-Buffer Normal Texture");
    specShinyTexture.setName("G-Buffer Specular/Shininess Texture");

    for (auto texture : { &colorTexture, &depthTexture, &normalTexture, &specShinyTexture })
    {
        texture->setMagFilter(gl::MagFilter::Linear);
        texture->setMinFilter(gl::MinFilter::Linear);
    }

    // Attach them
    framebuffer.attach(gl::ColorAttachment(0), colorTexture);
    framebuffer.attach(gl::ColorAttachment(1), normalTexture);
    framebuffer.attach(gl::ColorAttachment(2), specShinyTexture);
    framebuffer.attach(gl::DepthAttachment, depthTexture);
    framebuffer.setDrawBuffers(gl::ColorAttachment(0), 
        gl::ColorAttachment(1), gl::ColorAttachment(2));
    framebuffer.setName("G-Buffer Framebuffer");

    gbufferProgram = cache::loadProgram({ "resources/shaders/gbuffer.vert", "resources/shaders/gbuffer.frag" });
}

void GBuffer::begin()
{
    framebuffer.bind();
    glViewport(0, 0, (GLsizei)width, (GLsizei)height); gl::checkError();
    glClearColor(0.0, 0.0, 0.0, 0.0); gl::checkError();
    glClearDepth(1.0); gl::checkError();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GBuffer::end()
{
    gl::Framebuffer::bindDefault();
}

void scene::GBuffer::setParams(gl::Program& program, const glm::mat4& projection) const
{
    // Set the draw parameters
    colorTexture.bindTo(1);
    program.setUniform("ColorTexture", 1);
    depthTexture.bindTo(2);
    program.setUniform("DepthTexture", 2);
    normalTexture.bindTo(3);
    program.setUniform("NormalTexture", 3);
    specShinyTexture.bindTo(4);
    program.setUniform("SpecularShininessTexture", 4);

    // Set the parameters required for reconstructing the position
    program.setUniform("ScreenSize", glm::vec2(width, height));
    program.setUniform("InverseProjection", glm::inverse(projection));
}
