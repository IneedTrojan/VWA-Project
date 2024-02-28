#pragma once
#include <bitset>
#include <cstdint>
#include <memory>
#include <glad/glad.h>
#include "Enum/BufferTarget.h"
#include "Enum/BufferStorageHints.h"
#include "Enum/SamplerEnum.h"
#include "utility/Span.h"
#include "Enum/BufferMapping.h"

namespace graphics
{
	struct BufferObject
	{
	private:
		GLuint ID = 0;
		int32_t bound_unit = -1;
		uint64_t handle = 0;

	public:
		


		BufferObject() = default;


		GLuint GetID()const
		{
			return ID;
		}
		uint64_t GetHandle()const
		{
			return handle;
		}

		static std::unique_ptr<BufferObject> Create()
		{
			BufferObject buffer;
			glCreateBuffers( 1, &buffer.ID);
			return std::make_unique<BufferObject>(std::move(buffer));
		}
		static BufferObject CreateStack()
		{
			BufferObject buffer;
			glCreateBuffers( 1, & buffer.ID);
			return buffer;
		}

		void BindBufferRange(BufferTarget target, int32_t binding, int32_t offset, int32_t size)const
		{
			glBindBufferRange( gl_enum_cast(target), binding, ID, offset, size);
		}
		void BindBufferBase(BufferTarget target, int32_t binding)const
		{
			glBindBufferBase( gl_enum_cast(target), binding, ID);
		}


		BufferObject(BufferObject&& other) noexcept
			: ID(std::exchange(other.ID, 0)),
			handle(std::exchange(other.handle, 0))
		{
		}

		BufferObject& operator=(BufferObject&& other) noexcept
		{
			if (&other != this)
			{
				free();
				ID = std::exchange(other.ID, 0);
				handle = std::exchange(other.handle, 0);
			}
			return *this;
		}
		BufferObject(const BufferObject& other) = delete;
		BufferObject& operator =(const BufferObject& other) = delete;


		~BufferObject()
		{
			free();
		}

		

		void Bind(BufferTarget target)const
		{
			glBindBuffer( gl_enum_cast(target), ID);
		}
		void bufferData(utility::const_span<void> read, StorageHint flags)const
		{
			glNamedBufferStorage(ID, static_cast<int32_t>(read.byteSize()), read.begin_ptr, *flags);
		}
		void bufferSubData(utility::const_span<void> read, math::Number<int32_t> offset)const
		{
			glNamedBufferSubData(ID, offset, static_cast<int32_t>(read.byteSize()), read.begin_ptr);
		}
		void getBufferSubData(utility::span<void> write, math::Number<int32_t> offset)const
		{
			glGetNamedBufferSubData(ID, offset, static_cast<int32_t>(write.byteSize()), write.begin_ptr);
		}
		void* map(utility::const_span<void> area, BufferMapBits bits)const
		{
			return glMapNamedBufferRange(ID, area.offset(), area.byteSize(), *bits);
		}
		void unMap()const
		{
			glUnmapNamedBuffer(ID);
		}

		void free()
		{
			if (handle)
			{
				glMakeNamedBufferNonResidentNV(ID);
			}
			if (ID != 0)
			{
				glDeleteBuffers(1, &ID);
				ID = 0;
			}
		}

		void Bind(GLenum target) const
		{
			glBindBuffer(target, ID);
		}
	};

	class Buffer
	{
		BufferObject buffer;
		BufferTarget target;

		int32_t size = 0;

		mutable int32_t bindingOffset = 0;
		mutable int32_t bindingLength = 0;

	public:
		

		static std::shared_ptr<Buffer> Create(BufferTarget target, BufferObject&& obj = BufferObject::CreateStack())
		{
			return std::shared_ptr<Buffer>(new Buffer(target, std::move(obj)));
		}

		void BufferData(utility::const_span<void> read, StorageHint bufferFlags)
		{
			size = static_cast<int32_t>(read.byteSize());
			bindingLength = size;

			buffer.bufferData(read, bufferFlags);
		}
		void BufferSubData(utility::const_span<void> read, int32_t offset = 0)const
		{
			read = utility::const_span<void>(read.begin_ptr, std::min(size, static_cast<int32_t>(read.byteSize())));
			buffer.bufferSubData(read, offset);
		}
		void GetBufferSubData(utility::span<void> write, int32_t offset)const
		{
			write = utility::span<void>(write.begin_ptr, std::min(size, static_cast<int32_t>(write.byteSize())));
			buffer.getBufferSubData(write, offset);
		}
		int32_t byteSize()const
		{
			return size;
		}

		void SetBufferRange(int32_t offset, int32_t length = std::numeric_limits<int32_t>::max())const
		{
			length = std::min(size - offset, length);
			bindingOffset = offset;
			bindingLength = length;
		}


		void BindBufferRange(int32_t binding)const
		{
			buffer.BindBufferRange(target, binding, bindingOffset, bindingLength);
		}
		void BindBufferBase(int32_t binding)const
		{
			buffer.BindBufferBase(target, binding);
		}



		operator const BufferObject* ()const
		{
			return &buffer;
		}
		operator const BufferObject& ()const
		{
			return buffer;
		}
		void SetTarget(BufferTarget newTarget)
		{
			target = newTarget;
			buffer.Bind(target);
		}
		void Bind()const
		{
			buffer.Bind(target);
		}

		utility::span<void> MapBufferWrite(utility::const_span<> span = utility::InfinitySpan())const {
			span = span.clamp(byteSize() - span.offset());
			void* begin = buffer.map(span, BufferMapBits::WRITE_BIT | BufferMapBits::INVALIDATE_RANGE_BIT);
			return { begin, span.byteSize() };
		}
		utility::const_span<void> MapBufferRead(utility::const_span<> span = utility::InfinitySpan())const {
			span = span.clamp(byteSize() - span.offset());
			void* begin = buffer.map(span, BufferMapBits::READ_BIT);
			return { begin, span.byteSize() };
		}
		utility::const_span<void> MapBufferReadWrite(utility::const_span<> span = utility::InfinitySpan())const {
			span = span.clamp(byteSize() - span.offset());
			void* begin = buffer.map(span, BufferMapBits::READ_BIT);
			return { begin, span.byteSize() };
		}
		void UnMap()const
		{
			buffer.unMap();
		}

	public:
		Buffer(BufferTarget _target, BufferObject&& object) : buffer(std::move(object)), target(_target) {

		}
	};








}
