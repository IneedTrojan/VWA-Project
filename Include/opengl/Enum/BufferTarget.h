#pragma once
#include "opengl/Enum/EnumMath.h"
#include "utility/EnumLinker.h"

namespace graphics
{
	enum class BufferTarget: uint32_t {
		ArrayBuffer,
		ElementArrayBuffer,
		UniformBuffer,
		PixelUnpackBuffer,
		PixelPackBuffer,
		ShaderStorageBuffer,
		AtomicCounterBuffer,
		DispatchIndirectBuffer,
		QueryBuffer,
		TextureBuffer,
		TransformFeedbackBuffer,
		CopyReadBuffer,
		CopyWriteBuffer,
		DrawIndirectBuffer,
		ParameterBuffer, 
		TextureBufferEXT, 
	};
	inline utility::EnumLinker<BufferTarget, GLint>& BufferTargetGLEnum()
	{
		static utility::EnumLinker<BufferTarget, GLint> instance{
			{BufferTarget::ArrayBuffer, GL_ARRAY_BUFFER},
			{BufferTarget::ElementArrayBuffer, GL_ELEMENT_ARRAY_BUFFER},
			{BufferTarget::UniformBuffer, GL_UNIFORM_BUFFER},
			{BufferTarget::PixelUnpackBuffer, GL_PIXEL_UNPACK_BUFFER},
			{BufferTarget::PixelPackBuffer, GL_PIXEL_PACK_BUFFER},
			{BufferTarget::ShaderStorageBuffer, GL_SHADER_STORAGE_BUFFER},
			{BufferTarget::AtomicCounterBuffer, GL_ATOMIC_COUNTER_BUFFER},
			{BufferTarget::DispatchIndirectBuffer, GL_DISPATCH_INDIRECT_BUFFER},
			{BufferTarget::QueryBuffer, GL_QUERY_BUFFER},
			{BufferTarget::TextureBuffer, GL_TEXTURE_BUFFER},
			{BufferTarget::TransformFeedbackBuffer, GL_TRANSFORM_FEEDBACK_BUFFER},
			{BufferTarget::CopyReadBuffer, GL_COPY_READ_BUFFER},
			{BufferTarget::CopyWriteBuffer, GL_COPY_WRITE_BUFFER},
			{BufferTarget::DrawIndirectBuffer, GL_DRAW_INDIRECT_BUFFER},
			{BufferTarget::ParameterBuffer, GL_PARAMETER_BUFFER_ARB}, // Example: GL_ARB_shader_draw_parameters extension
			{BufferTarget::TextureBufferEXT, GL_TEXTURE_BUFFER_EXT} // Example: GL_EXT_texture_buffer extension
			// Add more mappings as needed
		};
		return instance;
	}
	inline GLint gl_enum_cast(BufferTarget bufferTarget)
	{
		return BufferTargetGLEnum()[bufferTarget];
	}
	ENABLE_ENUM_MATH(BufferTarget);


}