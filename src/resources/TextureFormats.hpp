#pragma once

#include <glad/glad.h>

namespace gl
{
    enum class Format : GLenum
    {
        Red = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        BGR = GL_BGR,
        RGBA = GL_RGBA,
        BGRA = GL_BGRA,
        Depth = GL_DEPTH_COMPONENT,
        DepthStencil = GL_DEPTH_STENCIL,

        Redi = GL_RED_INTEGER,
        RGi = GL_RG_INTEGER,
        RGBi = GL_RGB_INTEGER,
        BGRi = GL_BGR_INTEGER,
        RGBAi = GL_RGBA_INTEGER,
        BGRAi = GL_BGRA_INTEGER,
    };

    enum class InternalFormat : GLenum
    {
        Red = GL_RED,
        RG = GL_RG,
        RGB = GL_RGB,
        BGR = GL_BGR,
        RGBA = GL_RGBA,
        Depth = GL_DEPTH_COMPONENT,
        DepthStencil = GL_DEPTH_STENCIL,

        // Sized internal format
        R8 = GL_R8,
        R8s = GL_R8_SNORM,
        R16 = GL_R16,
        R16s = GL_R16_SNORM,
        RG8 = GL_RG8,
        RG8s = GL_RG8_SNORM,
        RG16 = GL_RG16,
        RG16s = GL_RG16_SNORM,
        R3G3B2 = GL_R3_G3_B2,
        RGB4 = GL_RGB4,
        RGB5 = GL_RGB5,
        RGB8 = GL_RGB8,
        RGB8s = GL_RGB8_SNORM,
        RGB10 = GL_RGB10,
        RGB12 = GL_RGB12,
        RGB16 = GL_RGB16,
        RGB16s = GL_RGB16_SNORM,
        RGBA2 = GL_RGBA2,
        RGBA4 = GL_RGBA4,
        RGB5A1 = GL_RGB5_A1,
        RGBA8 = GL_RGBA8,
        RGBA8s = GL_RGBA8_SNORM,
        RGB10A2 = GL_RGB10_A2,
        RGB10A2ui = GL_RGB10_A2UI,
        RGBA12 = GL_RGBA12,
        RGBA16 = GL_RGBA16,
        sRGB8 = GL_SRGB8,
        sRGB8A8 = GL_SRGB8_ALPHA8,
        R16f = GL_R16F,
        RG16f = GL_RG16F,
        RGB16f = GL_RGB16F,
        RGBA16f = GL_RGBA16F,
        R32f = GL_R32F,
        RG32f = GL_RG32F,
        RGB32f = GL_RGB32F,
        RGBA32f = GL_RGBA32F,
        R11G11B10f = GL_R11F_G11F_B10F,
        RGB9e5 = GL_RGB9_E5,
        R8i = GL_R8I,
        R8ui = GL_R8UI,
        R16i = GL_R16I,
        R16ui = GL_R16UI,
        R32i = GL_R32I,
        R32ui = GL_R32UI,
        RG8i = GL_RG8I,
        RG8ui = GL_RG8UI,
        RG16i = GL_RG16I,
        RG16ui = GL_RG16UI,
        RG32i = GL_RG32I,
        RG32ui = GL_RG32UI,
        RGB8i = GL_RGB8I,
        RGB8ui = GL_RGB8UI,
        RGB16i = GL_RGB16I,
        RGB16ui = GL_RGB16UI,
        RGB32i = GL_RGB32I,
        RGB32ui = GL_RGB32UI,
        RGBA8i = GL_RGBA8I,
        RGBA8ui = GL_RGBA8UI,
        RGBA16i = GL_RGBA16I,
        RGBA16ui = GL_RGBA16UI,
        RGBA32i = GL_RGBA32I,
        RGBA32ui = GL_RGBA32UI,

        // Compressed formats
        CompressedRed = GL_COMPRESSED_RED,
        CompressedRG = GL_COMPRESSED_RG,
        CompressedRGB = GL_COMPRESSED_RGB,
        CompressedRGBA = GL_COMPRESSED_RGBA,
        CompressedsRGB = GL_COMPRESSED_SRGB,
        CompressedsRGBAlpgha = GL_COMPRESSED_SRGB_ALPHA,
        CompressedRGTC1 = GL_COMPRESSED_RED_RGTC1,
        CompressedSignedRGTC1 = GL_COMPRESSED_SIGNED_RED_RGTC1,
        CompressedRGTC2 = GL_COMPRESSED_RG_RGTC2,
        CompressedSignedRGTC2 = GL_COMPRESSED_SIGNED_RG_RGTC2,

        // Depth formats
        Depth16 = GL_DEPTH_COMPONENT16,
        Depth24 = GL_DEPTH_COMPONENT24,
        Depth32 = GL_DEPTH_COMPONENT32,
        Depth32f = GL_DEPTH_COMPONENT32F,
        Depth24Stencil8 = GL_DEPTH24_STENCIL8,
        Depth32fStencil8 = GL_DEPTH32F_STENCIL8,
    };

    // The format used if the pointer is null
    constexpr Format deriveDefaultFormat(InternalFormat internalFormat)
    {
        switch (internalFormat)
        {
        case InternalFormat::Depth:
        case InternalFormat::Depth16:
        case InternalFormat::Depth24:
        case InternalFormat::Depth32:
        case InternalFormat::Depth32f:
            return Format::Depth;

        case InternalFormat::DepthStencil:
            return Format::DepthStencil;

        case InternalFormat::RGB10A2ui:
        case InternalFormat::R8i:
        case InternalFormat::R8ui:
        case InternalFormat::R16i:
        case InternalFormat::R16ui:
        case InternalFormat::R32i:
        case InternalFormat::R32ui:
        case InternalFormat::RG8i:
        case InternalFormat::RG8ui:
        case InternalFormat::RG16i:
        case InternalFormat::RG16ui:
        case InternalFormat::RG32i:
        case InternalFormat::RG32ui:
        case InternalFormat::RGB8i:
        case InternalFormat::RGB8ui:
        case InternalFormat::RGB16i:
        case InternalFormat::RGB16ui:
        case InternalFormat::RGB32i:
        case InternalFormat::RGB32ui:
        case InternalFormat::RGBA8i:
        case InternalFormat::RGBA8ui:
        case InternalFormat::RGBA16i:
        case InternalFormat::RGBA16ui:
        case InternalFormat::RGBA32i:
        case InternalFormat::RGBA32ui:
            return Format::Redi;

        default: return Format::Red;
        }
    }

    constexpr GLenum deriveDefaultType(InternalFormat internalFormat)
    {
        switch (internalFormat)
        {
        case InternalFormat::RGB10A2ui:
        case InternalFormat::R8i:
        case InternalFormat::R8ui:
        case InternalFormat::R16i:
        case InternalFormat::R16ui:
        case InternalFormat::R32i:
        case InternalFormat::R32ui:
        case InternalFormat::RG8i:
        case InternalFormat::RG8ui:
        case InternalFormat::RG16i:
        case InternalFormat::RG16ui:
        case InternalFormat::RG32i:
        case InternalFormat::RG32ui:
        case InternalFormat::RGB8i:
        case InternalFormat::RGB8ui:
        case InternalFormat::RGB16i:
        case InternalFormat::RGB16ui:
        case InternalFormat::RGB32i:
        case InternalFormat::RGB32ui:
        case InternalFormat::RGBA8i:
        case InternalFormat::RGBA8ui:
        case InternalFormat::RGBA16i:
        case InternalFormat::RGBA16ui:
        case InternalFormat::RGBA32i:
        case InternalFormat::RGBA32ui:
            return GL_INT;
        default: return GL_FLOAT;
        }
    }
}

