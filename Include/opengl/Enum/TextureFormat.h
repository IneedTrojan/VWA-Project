#pragma once
#include <glad/glad.h>
#include "utility/EnumLinker.h"
#include "DataType.h"


namespace graphics
{
    enum class TextureFormat {
        DepthComponent16,
        DepthComponent24,
        DepthComponent32,
        DepthComponent32F,
        Depth24Stencil8,
        Depth32FStencil8,
        StencilIndex8,

        R8,
        R16,
        R16F,
        R32F,
        R8I,
        R16I,
        R32I,
        R8UI,
        R16UI,
        R32UI,

        RG8,
        RG16,
        RG16F,
        RG32F,
        RG8I,
        RG16I,
        RG32I,
        RG8UI,
        RG16UI,
        RG32UI,

        RGB8,
        RGB16,
        RGB16F,
        RGB32F,
        RGB8I,
        RGB16I,
        RGB32I,
        RGB8UI,
        RGB16UI,
        RGB32UI,
        R11F_G11F_B10F,
        RGB9_E5,

        RGBA8,
        RGBA16,
        RGBA16F,
        RGBA32F,
        RGBA8I,
        RGBA16I,
        RGBA32I,
        RGBA8UI,
        RGBA16UI,
        RGBA32UI,
        RGB10_A2,
        RGB10_A2UI,
        RGBA4,
        RGB5_A1,

        SRGB8,
        SRGB8_Alpha8,

        CompressedRed,
        CompressedRG,
        CompressedRGB,
        CompressedRGBA,
        CompressedSRGB,
        CompressedSRGBAlpha,
        Auto,
    };

    enum class ExternalTextureFormat {
        Red,
        RG,
        RGB,
        BGR,
        RGBA,
        BGRA,

        DepthComponent,
        DepthStencil,
        StencilIndex,

        Luminance,
        LuminanceAlpha,
    };


    inline utility::EnumLinker<TextureFormat, GLint>& InternalTextureFormatsGLEnum() {
        static utility::EnumLinker<TextureFormat, GLint> instance{
            {TextureFormat::DepthComponent16, GL_DEPTH_COMPONENT16},
            {TextureFormat::DepthComponent24, GL_DEPTH_COMPONENT24},
            {TextureFormat::DepthComponent32, GL_DEPTH_COMPONENT32},
            {TextureFormat::DepthComponent32F, GL_DEPTH_COMPONENT32F},
            {TextureFormat::Depth24Stencil8, GL_DEPTH24_STENCIL8},
            {TextureFormat::Depth32FStencil8, GL_DEPTH32F_STENCIL8},
            {TextureFormat::StencilIndex8, GL_STENCIL_INDEX8},

            {TextureFormat::R8, GL_R8},
            {TextureFormat::R16, GL_R16},
            {TextureFormat::R16F, GL_R16F},
            {TextureFormat::R32F, GL_R32F},
            {TextureFormat::R8I, GL_R8I},
            {TextureFormat::R16I, GL_R16I},
            {TextureFormat::R32I, GL_R32I},
            {TextureFormat::R8UI, GL_R8UI},
            {TextureFormat::R16UI, GL_R16UI},
            {TextureFormat::R32UI, GL_R32UI},

            {TextureFormat::RG8, GL_RG8},
            {TextureFormat::RG16, GL_RG16},
            {TextureFormat::RG16F, GL_RG16F},
            {TextureFormat::RG32F, GL_RG32F},
            {TextureFormat::RG8I, GL_RG8I},
            {TextureFormat::RG16I, GL_RG16I},
            {TextureFormat::RG32I, GL_RG32I},
            {TextureFormat::RG8UI, GL_RG8UI},
            {TextureFormat::RG16UI, GL_RG16UI},
            {TextureFormat::RG32UI, GL_RG32UI},

            {TextureFormat::RGB8, GL_RGB8},
            {TextureFormat::RGB16, GL_RGB16},
            {TextureFormat::RGB16F, GL_RGB16F},
            {TextureFormat::RGB32F, GL_RGB32F},
            {TextureFormat::RGB8I, GL_RGB8I},
            {TextureFormat::RGB16I, GL_RGB16I},
            {TextureFormat::RGB32I, GL_RGB32I},
            {TextureFormat::RGB8UI, GL_RGB8UI},
            {TextureFormat::RGB16UI, GL_RGB16UI},
            {TextureFormat::RGB32UI, GL_RGB32UI},
            {TextureFormat::R11F_G11F_B10F, GL_R11F_G11F_B10F},
            {TextureFormat::RGB9_E5, GL_RGB9_E5},

            {TextureFormat::RGBA8, GL_RGBA8},
            {TextureFormat::RGBA16, GL_RGBA16},
            {TextureFormat::RGBA16F, GL_RGBA16F},
            {TextureFormat::RGBA32F, GL_RGBA32F},
            {TextureFormat::RGBA8I, GL_RGBA8I},
            {TextureFormat::RGBA16I, GL_RGBA16I},
            {TextureFormat::RGBA32I, GL_RGBA32I},
            {TextureFormat::RGBA8UI, GL_RGBA8UI},
            {TextureFormat::RGBA16UI, GL_RGBA16UI},
            {TextureFormat::RGBA32UI, GL_RGBA32UI},
            {TextureFormat::RGB10_A2, GL_RGB10_A2},
            {TextureFormat::RGB10_A2UI, GL_RGB10_A2UI},
            {TextureFormat::RGBA4, GL_RGBA4},
            {TextureFormat::RGB5_A1, GL_RGB5_A1},

            {TextureFormat::SRGB8, GL_SRGB8},
            {TextureFormat::SRGB8_Alpha8, GL_SRGB8_ALPHA8},

            {TextureFormat::CompressedRed, GL_COMPRESSED_RED},
            {TextureFormat::CompressedRG, GL_COMPRESSED_RG},
            {TextureFormat::CompressedRGB, GL_COMPRESSED_RGB},
            {TextureFormat::CompressedRGBA, GL_COMPRESSED_RGBA},
            {TextureFormat::CompressedSRGB, GL_COMPRESSED_SRGB},
            {TextureFormat::CompressedSRGBAlpha, GL_COMPRESSED_SRGB_ALPHA}
        };
        return instance;
    }


    inline utility::EnumLinker<ExternalTextureFormat, GLint>& ExternalTextureFormatsGLEnum() {
        static utility::EnumLinker<ExternalTextureFormat, GLint> instance{
            {ExternalTextureFormat::Red, GL_RED},
            {ExternalTextureFormat::RG, GL_RG},
            {ExternalTextureFormat::RGB, GL_RGB},
            {ExternalTextureFormat::BGR, GL_BGR},
            {ExternalTextureFormat::RGBA, GL_RGBA},
            {ExternalTextureFormat::BGRA, GL_BGRA},
            {ExternalTextureFormat::DepthComponent, GL_DEPTH_COMPONENT},
            {ExternalTextureFormat::DepthStencil, GL_DEPTH_STENCIL},
            {ExternalTextureFormat::StencilIndex, GL_STENCIL_INDEX},
            {ExternalTextureFormat::Luminance, GL_LUMINANCE},  // Legacy
            {ExternalTextureFormat::LuminanceAlpha, GL_LUMINANCE_ALPHA},  // Legacy
        };
        return instance;
    }











    inline GLint gl_enum_cast(TextureFormat value)
    {
        return InternalTextureFormatsGLEnum()[value];
    }
    inline GLint gl_enum_cast(ExternalTextureFormat value)
    {
        return ExternalTextureFormatsGLEnum()[value];
    }
   

    struct TextureDataFormat
    {
        ExternalTextureFormat dataFormat;
        DataType dataType;
    };

    inline utility::EnumLinker<TextureFormat, size_t>& InternalTextureFormatSizePerPixel() {
        static utility::EnumLinker<TextureFormat, size_t> instance{
            {TextureFormat::DepthComponent16, 2},
            {TextureFormat::DepthComponent24, 3},
            {TextureFormat::DepthComponent32, 4},
            {TextureFormat::DepthComponent32F, 4},
            {TextureFormat::Depth24Stencil8, 4},
            {TextureFormat::Depth32FStencil8, 5},
            {TextureFormat::StencilIndex8, 1},

            {TextureFormat::R8, 1},
            {TextureFormat::R16, 2},
            {TextureFormat::R16F, 2},
            {TextureFormat::R32F, 4},
            {TextureFormat::R8I, 1},
            {TextureFormat::R16I, 2},
            {TextureFormat::R32I, 4},
            {TextureFormat::R8UI, 1},
            {TextureFormat::R16UI, 2},
            {TextureFormat::R32UI, 4},

            {TextureFormat::RG8, 2},
            {TextureFormat::RG16, 4},
            {TextureFormat::RG16F, 4},
            {TextureFormat::RG32F, 8},
            {TextureFormat::RG8I, 2},
            {TextureFormat::RG16I, 4},
            {TextureFormat::RG32I, 8},
            {TextureFormat::RG8UI, 2},
            {TextureFormat::RG16UI, 4},
            {TextureFormat::RG32UI, 8},

            {TextureFormat::RGB8, 3},
            {TextureFormat::RGB16, 6},
            {TextureFormat::RGB16F, 6},
            {TextureFormat::RGB32F, 12},
            {TextureFormat::RGB8I, 3},
            {TextureFormat::RGB16I, 6},
            {TextureFormat::RGB32I, 12},
            {TextureFormat::RGB8UI, 3},
            {TextureFormat::RGB16UI, 6},
            {TextureFormat::RGB32UI, 12},
            {TextureFormat::R11F_G11F_B10F, 4}, // This is a special case
            {TextureFormat::RGB9_E5, 4}, // This is a special case

            {TextureFormat::RGBA8, 4},
            {TextureFormat::RGBA16, 8},
            {TextureFormat::RGBA16F, 8},
            {TextureFormat::RGBA32F, 16},
            {TextureFormat::RGBA8I, 4},
            {TextureFormat::RGBA16I, 8},
            {TextureFormat::RGBA32I, 16},
            {TextureFormat::RGBA8UI, 4},
            {TextureFormat::RGBA16UI, 8},
            {TextureFormat::RGBA32UI, 16},
            {TextureFormat::RGB10_A2, 4}, // 10 bits per RGB channel and 2 bits for alpha
            {TextureFormat::RGB10_A2UI, 4},
            {TextureFormat::RGBA4, 2}, // 4 bits for each channel
            {TextureFormat::RGB5_A1, 2}, // 5 bits for RGB and 1 bit for alpha

            {TextureFormat::SRGB8, 3},
            {TextureFormat::SRGB8_Alpha8, 4},

            // Compressed formats - sizes vary based on compression. Placeholder values used here.
            {TextureFormat::CompressedRed, 1},
            {TextureFormat::CompressedRG, 2},
            {TextureFormat::CompressedRGB, 3},
            {TextureFormat::CompressedRGBA, 4},
            {TextureFormat::CompressedSRGB, 3},
            {TextureFormat::CompressedSRGBAlpha, 4}
        };
        return instance;
    }

    inline utility::EnumLinker<ExternalTextureFormat, std::array<int32_t, 5>>& ExternalTextureFormatChannelLayout() {
        static utility::EnumLinker<ExternalTextureFormat, std::array<int32_t, 5>> instance{
            {ExternalTextureFormat::Red, {1,0, -1, -1, -1}},
            {ExternalTextureFormat::RG, {2,0, 1, -1, -1}},
            {ExternalTextureFormat::RGB, {3,0, 1, 2, -1}},
            {ExternalTextureFormat::RGBA, {4,0, 1, 2, 3}},
            {ExternalTextureFormat::BGRA, {4,2, 1, 0, 3}},

            // For non-color formats, we use 0 to indicate the presence of data,
            // but we need to understand that it's not representing a color channel in the usual sense.
            {ExternalTextureFormat::DepthComponent, {1,0, -1, -1, -1}},
            {ExternalTextureFormat::DepthStencil, {1,0, 0, -1, -1}}, // Both depth and stencil data present
            {ExternalTextureFormat::StencilIndex, {1,0, -1, -1, -1}},

            // Luminance and LuminanceAlpha can be treated similar to Red and RG respectively.
            {ExternalTextureFormat::Luminance, {1,0, -1, -1, -1}},
            {ExternalTextureFormat::LuminanceAlpha, {1,0, 1, -1, -1}},
        };
        return instance;
    }
    inline int32_t size_multiplier(ExternalTextureFormat format)
    {
        return ExternalTextureFormatChannelLayout()[format][0];
    }



}
