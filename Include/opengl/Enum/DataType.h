#pragma once
#include <glad/glad.h>
#include "utility/EnumLinker.h"
namespace graphics
{
    enum class DataType {
        uint8_t,
        int8_t,
        uint16_t,
        int16_t,
        uint32_t,
        int32_t,
        uint64_t,
        int64_t,

        float64_t,
        float32_t,
        float16_t,

        UInt8_3_3_2,
        UInt8_2_3_3_Rev,
        UInt16_5_6_5,
        UInt16_5_6_5_Rev,
        UInt16_4_4_4_4,
        UInt16_4_4_4_4_Rev,
        UInt16_5_5_5_1,
        UInt16_1_5_5_5_Rev,
        UInt32_8_8_8_8,
        UInt32_8_8_8_8_Rev,
        UInt32_10_10_10_2,
        UInt32_2_10_10_10_Rev,

        DEPTH_COMPONENT,
        DEPTH_STENCIL,
        STENCIL_INDEX,

        COMPRESSED_RGB,
        COMPRESSED_RGBA,
        COMPRESSED_RED,
        COMPRESSED_RG,

        SRGB,
        SRGB8,
        SRGB_ALPHA,
        SRGB8_ALPHA8,
        NONE
    };

    inline utility::EnumLinker<DataType, int32_t>& DataTypeSize() {
        static utility::EnumLinker<DataType, int32_t> instance{
            {DataType::uint8_t, 1},
            {DataType::int8_t, 1},
            {DataType::uint16_t, 2},
            {DataType::int16_t, 2},
            {DataType::uint32_t, 4},
            {DataType::int32_t, 4},
            {DataType::uint64_t, 8},
            {DataType::int64_t, 8},
            {DataType::float64_t, 8},
            {DataType::float32_t, 4},
            {DataType::float16_t, 2},

            {DataType::UInt8_3_3_2, 1}, // Packed into a single byte
            {DataType::UInt8_2_3_3_Rev, 1}, // Packed into a single byte
            {DataType::UInt16_5_6_5, 2}, // Packed into two bytes
            {DataType::UInt16_5_6_5_Rev, 2}, // Packed into two bytes
            {DataType::UInt16_4_4_4_4, 2}, // Packed into two bytes
            {DataType::UInt16_4_4_4_4_Rev, 2}, // Packed into two bytes
            {DataType::UInt16_5_5_5_1, 2}, // Packed into two bytes
            {DataType::UInt16_1_5_5_5_Rev, 2}, // Packed into two bytes
            {DataType::UInt32_8_8_8_8, 4}, // Packed into four bytes
            {DataType::UInt32_8_8_8_8_Rev, 4}, // Packed into four bytes
            {DataType::UInt32_10_10_10_2, 4}, // Packed into four bytes

            {DataType::DEPTH_COMPONENT, 0},
            {DataType::DEPTH_STENCIL, 0},
            {DataType::STENCIL_INDEX, 0},
            {DataType::COMPRESSED_RGB, 0},
            {DataType::COMPRESSED_RGBA, 0},
            {DataType::COMPRESSED_RED, 0},
            {DataType::COMPRESSED_RG, 0},
            {DataType::SRGB, 0},
            {DataType::SRGB8, 0},
            {DataType::SRGB_ALPHA, 0},
            {DataType::SRGB8_ALPHA8, 0}
        };
        return instance;
    }

    inline utility::EnumLinker<DataType, GLint>& DataTypeGLEnum() {
        static utility::EnumLinker<DataType, GLint> instance{
            {DataType::uint8_t, GL_UNSIGNED_BYTE},
            {DataType::int8_t, GL_BYTE},
            {DataType::uint16_t, GL_UNSIGNED_SHORT},
            {DataType::int16_t, GL_SHORT},
            {DataType::uint32_t, GL_UNSIGNED_INT},
            {DataType::int32_t, GL_INT},
            {DataType::uint64_t, GL_UNSIGNED_INT},
            {DataType::int64_t, GL_INT},
            {DataType::float32_t, GL_DOUBLE},
            {DataType::float32_t, GL_FLOAT},
            {DataType::float16_t, GL_HALF_FLOAT},
            {DataType::UInt8_3_3_2, GL_UNSIGNED_BYTE_3_3_2},
            {DataType::UInt8_2_3_3_Rev, GL_UNSIGNED_BYTE_2_3_3_REV},
            {DataType::UInt16_5_6_5, GL_UNSIGNED_SHORT_5_6_5},
            {DataType::UInt16_5_6_5_Rev, GL_UNSIGNED_SHORT_5_6_5_REV},
            {DataType::UInt16_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4},
            {DataType::UInt16_4_4_4_4_Rev, GL_UNSIGNED_SHORT_4_4_4_4_REV},
            {DataType::UInt16_5_5_5_1, GL_UNSIGNED_SHORT_5_5_5_1},
            {DataType::UInt16_1_5_5_5_Rev, GL_UNSIGNED_SHORT_1_5_5_5_REV},
            {DataType::UInt32_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8},
            {DataType::UInt32_8_8_8_8_Rev, GL_UNSIGNED_INT_8_8_8_8_REV},
            {DataType::UInt32_10_10_10_2, GL_UNSIGNED_INT_10_10_10_2},
            {DataType::DEPTH_COMPONENT, GL_DEPTH_COMPONENT},
            {DataType::DEPTH_STENCIL, GL_DEPTH_STENCIL},
            {DataType::STENCIL_INDEX, GL_STENCIL_INDEX},
            {DataType::COMPRESSED_RGB, GL_COMPRESSED_RGB},
            {DataType::COMPRESSED_RGBA, GL_COMPRESSED_RGBA},
            {DataType::COMPRESSED_RED, GL_COMPRESSED_RED},
            {DataType::COMPRESSED_RG, GL_COMPRESSED_RG},
            {DataType::SRGB, GL_SRGB},
            {DataType::SRGB8, GL_SRGB8},
            {DataType::SRGB_ALPHA, GL_SRGB_ALPHA},
            {DataType::SRGB8_ALPHA8, GL_SRGB8_ALPHA8}
        };
        return instance;
    }
    inline GLint gl_enum_cast(DataType dataType)
    {
        return DataTypeGLEnum()[dataType];
    }

    inline int32_t size_multiplier(DataType format)
    {
        return DataTypeSize()[format];
    }


}