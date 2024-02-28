#pragma once
#include <glad/glad.h>
#include "Enum/EnumMath.h"

namespace graphics
{
	enum class StorageHint: uint32_t {
		None = 0x0000,
		DynamicStorage = GL_DYNAMIC_STORAGE_BIT, // Typically for buffers that are modified repeatedly and used many times.
		KeepCopyCpu = GL_CLIENT_STORAGE_BIT, // Typically for buffers that the client (CPU) will read from or write to.
		ReadAccess = GL_MAP_READ_BIT, // For buffer mapping, indicates that the returned pointer can be read.
		WriteAccess = GL_MAP_WRITE_BIT, // For buffer mapping, indicates that the returned pointer can be written to.
		CoherentMapping = GL_MAP_COHERENT_BIT, // For coherent mapping of buffer data.
		MapInvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT, // Indicates that a subrange of the buffer may be invalidated.
		MapInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT, // Indicates that the entire buffer may be invalidated.

		PRESETDynamic = DynamicStorage | WriteAccess,
		PRESETImmutable = ReadAccess,
		PRESETStreamDraw = DynamicStorage | MapInvalidateRange,
		PRESETStaticDraw = WriteAccess,
		PRESETCoherentRead = ReadAccess | CoherentMapping,
		PRESETCoherentWrite = WriteAccess | CoherentMapping,
		PRESETCoherentReadWrite = ReadAccess | WriteAccess | CoherentMapping
	};
	ENABLE_ENUM_MATH(StorageHint);





}
