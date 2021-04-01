#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <type_traits>
#include <string>
#include "TextureFormats.hpp"
#include "wrappers/glParamFromType.hpp"
#include "wrappers/glDepthComparisonMode.hpp"
#include "wrappers/glException.hpp"

namespace gl
{
    constexpr GLint getDimensionsFrom(GLenum target)
    {
        switch (target)
        {
        case GL_TEXTURE_1D: return 1;
        case GL_TEXTURE_2D: return 2;
        case GL_TEXTURE_RECTANGLE: return 2;
        case GL_TEXTURE_CUBE_MAP: return 2;
        case GL_TEXTURE_1D_ARRAY: return 2;
        case GL_TEXTURE_3D: return 3;
        case GL_TEXTURE_2D_ARRAY: return 3;
        default: return -1;
        }
    }

    enum class FormatOptions : GLint
    {
        None, Integer, BGRA, BGRAInteger, Depth, DepthStencil
    };

    template <glm::length_t N>
    constexpr GLenum formatParam(FormatOptions options)
    {
        bool bgra = options == FormatOptions::BGRA || options == FormatOptions::BGRAInteger;
        bool integer = options == FormatOptions::Integer || options == FormatOptions::BGRAInteger;

        switch (N)
        {
        case 1: if (options == FormatOptions::Depth) return GL_DEPTH_COMPONENT;
              else return integer ? GL_RED_INTEGER : GL_RED;
        case 2: if (options == FormatOptions::DepthStencil) return GL_DEPTH_STENCIL;
              else return integer ? GL_RG_INTEGER : GL_RG;
        case 3: return integer ? GL_RGB_INTEGER : GL_RGB;
        case 4: if (bgra) return integer ? GL_BGRA_INTEGER : GL_BGRA;
              else return integer ? GL_RGBA_INTEGER : GL_RGBA;
        default: return 0;
        }
    }

    enum class MagFilter : GLint
    {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR
    };

    enum class MinFilter : GLint
    {
        Nearest = GL_NEAREST,
        Linear = GL_LINEAR,
        NearestMipNearest = GL_NEAREST_MIPMAP_NEAREST,
        NearestMipLinear = GL_NEAREST_MIPMAP_LINEAR,
        LinearMipNearest = GL_LINEAR_MIPMAP_NEAREST,
        LinearMipLinear = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class WrapEffect : GLint
    {
        Repeat = GL_REPEAT,
        MirroredRepeat = GL_MIRRORED_REPEAT,
        ClampToEdge = GL_CLAMP_TO_EDGE,
        ClampToBorder = GL_CLAMP_TO_BORDER
    };

    template <GLenum Target>
    class TextureBase
    {
    protected:
        GLuint texture;
        static inline thread_local GLuint lastBoundTexture = 0;

        TextureBase(GLuint texture) : texture(texture) {}
        ~TextureBase() { glDeleteTextures(1, &texture); gl::checkError(); }

    public:
        constexpr static auto NumDimensions = getDimensionsFrom(Target);
        static_assert(NumDimensions != -1, "Invalid target passed to Texture!");

        // Disallow copying
        TextureBase(const TextureBase&) = delete;
        TextureBase& operator=(const TextureBase&) = delete;

        // Enable moving
        TextureBase(TextureBase&& o) noexcept : texture(o.texture) { o.texture = 0; }
        TextureBase& operator=(TextureBase&& o) noexcept
        {
            std::swap(texture, o.texture);
            return *this;
        }

        void setName(const std::string& name)
        {
            glObjectLabel(GL_TEXTURE, texture, (GLsizei)name.size(), name.data()); gl::checkError();
        }

        void bind() const
        {
            if (lastBoundTexture != texture)
            {
                glBindTexture(Target, texture); gl::checkError();
                lastBoundTexture = texture;
            }
        }

        void bindTo(GLuint unit) const
        {
            glActiveTexture(GL_TEXTURE0 + unit); gl::checkError();
            glBindTexture(Target, texture); gl::checkError();
            lastBoundTexture = 0;
        }

        void generateMipmap() { this->bind(); glGenerateMipmap(Target); gl::checkError(); }

        void setMagFilter(MagFilter filter) { this->bind(); glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filter)); gl::checkError(); }
        void setMinFilter(MinFilter filter) { this->bind(); glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filter)); gl::checkError(); }
        void setMaxAnisotropy(float f) { this->bind(); glTexParameterf(Target, GL_TEXTURE_MAX_ANISOTROPY, f); gl::checkError(); }

        void setBorderColor(const glm::vec4& color) { this->bind(); glTexParameterfv(Target, GL_TEXTURE_BORDER_COLOR, &color.x); gl::checkError(); }

        void enableComparisonMode(ComparisonFunction func = ComparisonFunction::Less)
        {
            this->bind();
            glTexParameteri(Target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); gl::checkError();
            glTexParameteri(Target, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(func)); gl::checkError();
        }

        void clearComparisonMode() { this->bind(); glTexParameteri(Target, GL_TEXTURE_COMPARE_MODE, GL_NONE); gl::checkError(); }

        friend class Framebuffer;
    };

    template <GLenum Target, std::size_t NumDims>
    class TextureDimensions
    {
        static_assert(NumDims != (std::size_t)-1, "Invalid target passed to Texture!");
    };

    template <GLenum Target>
    class TextureDimensions<Target, (std::size_t)1> : public TextureBase<Target>
    {
    protected:
        TextureDimensions(GLuint texture) : TextureBase<Target>(texture) {}

    public:
        template <typename T>
        void assign(GLint level, InternalFormat internalFormat, GLsizei width, Format format, const T* data)
        {
            this->bind(); glTexImage1D(Target, level, static_cast<GLenum>(internalFormat), width,
                0, static_cast<GLenum>(format), ParamFromType<T>, data); gl::checkError();
        }

        void assign(GLint level, InternalFormat internalFormat, GLsizei width)
        {
            assign(level, internalFormat, width, deriveDefaultFormat(internalFormat), static_cast<const float*>(nullptr));
        }

        void setWrapEffectS(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_S, static_cast<GLint>(effect)); gl::checkError(); }
    };

    template <GLenum Target>
    class TextureDimensions<Target, (std::size_t)2> : public TextureBase<Target>
    {
    protected:
        TextureDimensions(GLuint texture) : TextureBase<Target>(texture) {}

    public:
        template <typename T>
        void assign(GLint level, InternalFormat internalFormat, GLsizei width, GLsizei height, Format format, const T* data)
        {
            this->bind(); glTexImage2D(Target, level, static_cast<GLenum>(internalFormat), width, height,
                0, static_cast<GLenum>(format), ParamFromType<T>, data); gl::checkError();
        }

        void assign(GLint level, InternalFormat internalFormat, GLsizei width, GLsizei height)
        {
            this->bind(); glTexImage2D(Target, level, static_cast<GLenum>(internalFormat), width, height,
                0, static_cast<GLenum>(deriveDefaultFormat(internalFormat)), deriveDefaultType(internalFormat), nullptr);; gl::checkError();
        }

        void setWrapEffectS(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_S, static_cast<GLint>(effect)); gl::checkError(); }
        void setWrapEffectT(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_T, static_cast<GLint>(effect)); gl::checkError(); }
    };

    template <GLenum Target>
    class TextureDimensions<Target, (std::size_t)3> : public TextureBase<Target>
    {
    protected:
        TextureDimensions(GLuint texture) : TextureBase<Target>(texture) {}

    public:
        template <typename T>
        void assign(GLint level, InternalFormat internalFormat, GLsizei width, GLsizei height, GLsizei depth, Format format, const T* data)
        {
            this->bind(); glTexImage3D(Target, level, static_cast<GLenum>(internalFormat), width, height, depth,
                0, static_cast<GLenum>(format), ParamFromType<T>, data); gl::checkError();
        }

        void assign(GLint level, InternalFormat internalFormat, GLsizei width, GLsizei height, GLsizei depth)
        {
            assign(level, internalFormat, width, height, depth, deriveDefaultFormat(internalFormat), static_cast<const float*>(nullptr));
        }

        void setWrapEffectS(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_S, static_cast<GLint>(effect)); gl::checkError(); }
        void setWrapEffectT(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_T, static_cast<GLint>(effect)); gl::checkError(); }
        void setWrapEffectR(WrapEffect effect) { this->bind(); glTexParameteri(Target, GL_TEXTURE_WRAP_R, static_cast<GLint>(effect)); gl::checkError(); }
    };

    template <GLenum Target>
    using TexDim = TextureDimensions<Target, getDimensionsFrom(Target)>;

    template <GLenum Target>
    class Texture final : public TexDim<Target>
    {
    public:
        static Texture none() { return Texture(0); }

        Texture() : TexDim<Target>(0) { glGenTextures(1, &this->texture); gl::checkError(); }

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        // Enable moving
        Texture(Texture&& o) noexcept : TexDim<Target>(o.texture) { o.texture = 0; }
        Texture& operator=(Texture&& o) noexcept
        {
            std::swap(this->texture, o.texture);
            return *this;
        }

        friend class Framebuffer;
    };

    using Texture1D = Texture<GL_TEXTURE_1D>;
    using Texture2D = Texture<GL_TEXTURE_2D>;
    using Texture3D = Texture<GL_TEXTURE_3D>;
    using TextureCubeMap = Texture<GL_TEXTURE_CUBE_MAP>;
    using TextureRectangle = Texture<GL_TEXTURE_RECTANGLE>;
    using Texture1DArray = Texture<GL_TEXTURE_1D_ARRAY>;
    using Texture2DArray = Texture<GL_TEXTURE_2D_ARRAY>;
}
