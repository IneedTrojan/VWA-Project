#pragma once
#include "EnumMath.h"
#include "utility/EnumLinker.h"

namespace graphics
{
    enum class TextureTarget : uint32_t {
        Texture1D,
        Texture2D,
        Texture3D,
        Texture1DArray,
        Texture2DArray,
        TextureRectangle,
        TextureCubeMap,
        TextureCubeMapArray,
        TextureBuffer,
        Texture2DMultisample,
        Texture2DMultisampleArray,
        END
    };
    ENABLE_ENUM_MATH(TextureTarget);

    inline utility::EnumLinker<TextureTarget, GLint>& TextureTypeGLEnum() {
        static utility::EnumLinker<TextureTarget, GLint> instance{
            {TextureTarget::Texture1D, GL_TEXTURE_1D},
            {TextureTarget::Texture2D, GL_TEXTURE_2D},
            {TextureTarget::Texture3D, GL_TEXTURE_3D},
            {TextureTarget::Texture1DArray, GL_TEXTURE_1D_ARRAY},
            {TextureTarget::Texture2DArray, GL_TEXTURE_2D_ARRAY},
            {TextureTarget::TextureRectangle, GL_TEXTURE_RECTANGLE},
            {TextureTarget::TextureCubeMap, GL_TEXTURE_CUBE_MAP},
            {TextureTarget::TextureCubeMapArray, GL_TEXTURE_CUBE_MAP_ARRAY},
            {TextureTarget::TextureBuffer, GL_TEXTURE_BUFFER},
            {TextureTarget::Texture2DMultisample, GL_TEXTURE_2D_MULTISAMPLE},
            {TextureTarget::Texture2DMultisampleArray, GL_TEXTURE_2D_MULTISAMPLE_ARRAY}
        };
        return instance;
    }
    inline int32_t gl_enum_cast(TextureTarget target)
    {
        return TextureTypeGLEnum()[target];
    }
}
