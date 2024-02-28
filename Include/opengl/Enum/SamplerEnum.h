#pragma once
#include <glad/glad.h>

#include "TextureTarget.h"
#include "utility/EnumLinker.h"

namespace graphics
{
    enum class WrapMode {
        REPEAT = GL_REPEAT,
        CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,
        MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
        CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE = GL_MIRROR_CLAMP_TO_EDGE
    };

    enum class MinFilter {
        NEAREST = GL_NEAREST,
        LINEAR = GL_LINEAR,
        NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST,
        LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
        NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
        LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
    };

    enum class MagFilter {
        NEAREST = GL_NEAREST,
        LINEAR = GL_LINEAR,
        AUTOMATIC_BLIT = 0,
    };

    enum class CompareMode {
        NONE = GL_NONE,
        COMPARE_REF_TO_TEXTURE = GL_COMPARE_REF_TO_TEXTURE
    };

    enum class CompareFunc {
        NEVER = GL_NEVER,
        LESS = GL_LESS,
        EQUAL = GL_EQUAL,
        LEQUAL = GL_LEQUAL,
        GREATER = GL_GREATER,
        NOTEQUAL = GL_NOTEQUAL,
        GEQUAL = GL_GEQUAL,
        ALWAYS = GL_ALWAYS
    };

    enum class SwizzleChannel {
        RED = GL_RED,
        GREEN = GL_GREEN,
        BLUE = GL_BLUE,
        ALPHA = GL_ALPHA,
        ZERO = GL_ZERO,
        ONE = GL_ONE
    };

    enum class SeamlessCubemap {
        ENABLED = GL_TRUE,
        DISABLED = GL_FALSE
    };

    enum class DepthStencilMode {
        LUMINANCE = GL_LUMINANCE,
        ALPHA = GL_ALPHA
    };

    enum class SRGBDecodeExt {
        DECODE_EXT = GL_DECODE_EXT,
        SKIP_DECODE_EXT = GL_SKIP_DECODE_EXT
    };


    enum class WriteAccess
    {
        Readonly = GL_READ_ONLY,
        Writeonly = GL_WRITE_ONLY
    };

    enum class CubeMapFace {
        POSITIVE_X,
        NEGATIVE_X,
        POSITIVE_Y,
        NEGATIVE_Y,
        POSITIVE_Z,
        NEGATIVE_Z,
    };

    



  
    inline GLint gl_enum_cast(CubeMapFace face)
    {
        return static_cast<int32_t>(face) + GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    }
}

