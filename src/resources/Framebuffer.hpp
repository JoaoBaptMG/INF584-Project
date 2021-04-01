#pragma once

#include <glad/glad.h>
#include <algorithm>

#include "Texture.hpp"
#include "Renderbuffer.hpp"
#include "util/is_one_of.hpp"
#include "wrappers/glException.hpp"

namespace gl
{
    enum class FramebufferStatus : GLenum
    {
        Complete = GL_FRAMEBUFFER_COMPLETE,
        Undefined = GL_FRAMEBUFFER_UNDEFINED,
        Unsupported = GL_FRAMEBUFFER_UNSUPPORTED,
        IncompleteAttachment = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
        IncompleteMissingAttachment = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
        IncompleteDrawBuffer = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
        IncompleteReadBuffer = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
        IncompleteMultisample = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        IncompleteLayerTargets = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS,
    };

    class Attachment final
    {
        GLenum attachment;

    public:
        constexpr Attachment(GLenum attachment) : attachment(attachment) {}
        friend class Framebuffer;
    };

    static constexpr Attachment ColorAttachment(GLenum i) { return GL_COLOR_ATTACHMENT0 + i; }
    static constexpr Attachment DepthAttachment = GL_DEPTH_ATTACHMENT;
    static constexpr Attachment StencilAttachment = GL_STENCIL_ATTACHMENT;
    static constexpr Attachment DepthStencilAttachment = GL_DEPTH_STENCIL_ATTACHMENT;

    class Framebuffer final
    {
        GLuint framebuffer;
        static inline thread_local GLuint lastBoundFramebuffer = 0;

        explicit Framebuffer(int) : framebuffer(0) {};
    public:
        static Framebuffer none() { return Framebuffer(-1); }

        Framebuffer() { glGenFramebuffers(1, &framebuffer); gl::checkError(); }
        ~Framebuffer() { glDeleteFramebuffers(1, &framebuffer); gl::checkError(); }

        // Disallow copying
        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        // Enable moving
        Framebuffer(Framebuffer&& o) noexcept : framebuffer(o.framebuffer) { o.framebuffer = 0; }
        Framebuffer& operator=(Framebuffer&& o) noexcept
        {
            std::swap(framebuffer, o.framebuffer);
            return *this;
        }

        void setName(const std::string& name)
        {
            glObjectLabel(GL_FRAMEBUFFER, framebuffer, (GLsizei)name.size(), name.data()); gl::checkError();
        }

        void bind() const
        { 
            if (lastBoundFramebuffer != framebuffer)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); gl::checkError();
                lastBoundFramebuffer = framebuffer;
            }
        }

        static void bindDefault()
        { 
            if (lastBoundFramebuffer != 0)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0); gl::checkError();
                lastBoundFramebuffer = 0;
            }
        }

        template <GLenum Target>
        void attach(Attachment attachment, const Texture<Target>& tex, GLint level = 0)
        {
            bind(); tex.bind();
            glFramebufferTexture(GL_FRAMEBUFFER, attachment.attachment, tex.texture, level); gl::checkError();
        }

        template <GLenum Target>
        void attachLayer(Attachment attachment, const Texture<Target>& tex, GLint level, GLint layer)
        {
            bind(); tex.bind();
            glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment.attachment, tex.texture, level, layer); gl::checkError();
        }

        void attach(Attachment attachment, const Renderbuffer& rb)
        {
            bind(); rb.bind();
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment.attachment, GL_RENDERBUFFER, rb.renderbuffer); gl::checkError();
        }

        void setDrawBuffers(std::initializer_list<Attachment> attachments)
        {
            bind();
            // This is sound because they are pointer-interconvertible
            glDrawBuffers((GLsizei)attachments.size(), reinterpret_cast<const GLenum*>(attachments.begin()));
        }

        template <std::same_as<Attachment>... As>
        void setDrawBuffers(As... attachments) { setDrawBuffers({ attachments... }); }

        auto getStatus() const { bind(); return static_cast<FramebufferStatus>(gl::checkError(glCheckFramebufferStatus(GL_FRAMEBUFFER))); }
    };
}

