//#pragma once
//#include <bitset>
//#include <corecrt_io.h>
//#include <utility>
//#include <glad/glad.h>
//#include <json/Json.h>
//#include <magic_enum/magic_enum_all.hpp>
//
//#include "math/BoundingBox.h"
//#include "Texture/TextureEnum.h"
//#include "GL_CALL.h"
//#include "utility/Span.h"
//#include "gl/shader/ShaderState.h"
//#include "utility/Pooling/SpanBatcher.h"
//#include "glm/glm.hpp"
//#include "math/Bitfield.h"
//#include "utility/EnumLinker.h"
//#include "math/DynamicTransform.h"
//#include "components/Entity.h"
//
//namespace scene
//{
//	class Window;
//	class Scene;
//	class Input;
//}
//namespace graphics
//{
//	using namespace gl;
//
//	
//
//	
//
//	struct TextureObject
//	{
//	private:
//		GLuint m_ID = 0;
//		mutable int32_t TextureUnit = 0;
//		uint64_t handle = 0;
//
//	public:
//		GLuint GetID()const
//		{
//			return m_ID;
//		}
//		int32_t GetTextureUnit()const
//		{
//			return TextureUnit;
//		}
//		void SetTextureUnit(int32_t unit)const
//		{
//			TextureUnit = unit;
//		}
//		TextureObject() = default;
//
//		static std::unique_ptr<TextureObject> Create(gl::TextureTarget textureTarget)
//		{
//			TextureObject texture;
//			GL_CALL(glCreateTextures, gl::gl_enum_cast(textureTarget), 1, &texture.m_ID);
//			return std::make_unique<TextureObject>(std::move(texture));
//		}
//		static TextureObject CreateStack(gl::TextureTarget textureTarget)
//		{
//			TextureObject texture;
//			GL_CALL(glCreateTextures, gl::gl_enum_cast(textureTarget), 1, &texture.m_ID);
//			return texture;
//		}
//
//		TextureObject(TextureObject&& other)noexcept
//			:m_ID(std::exchange(other.m_ID, 0)),
//			handle(std::exchange(other.m_ID, 0))
//		{
//
//		}
//		TextureObject& operator=(TextureObject&& other)noexcept
//		{
//			if (&other != this)
//			{
//				free();
//				m_ID = std::exchange(other.m_ID, 0);
//				handle = std::exchange(other.handle, 0);
//			}
//			return *this;
//		}
//
//		TextureObject(const TextureObject& other) = delete;
//		TextureObject& operator =(const TextureObject& other) = delete;
//
//		~TextureObject()
//		{
//			free();
//		}
//		void makeBindLess(const SamplerObject& sampler)
//		{
//			handle = GL_CALL(glGetTextureSamplerHandleARB, m_ID, sampler.GetID());
//			GL_CALL(glMakeTextureHandleResidentARB, handle);
//		}
//		void makeBindLess()
//		{
//			handle = GL_CALL(glGetTextureHandleARB, m_ID);
//			glMakeTextureHandleResidentARB(handle);
//		}
//
//		void free()
//		{
//			if (handle) {
//				GL_CALL(glMakeTextureHandleNonResidentARB, handle);
//			}
//			if (m_ID) {
//				GL_CALL(glDeleteTextures, 1, &m_ID);
//				m_ID = 0;
//			}
//		}
//
//		void textureStorage1D(int32_t _w, gl::TextureFormat textureFormat, int32_t mipMaps = 1)const
//		{
//			assert(mipMaps > 0);
//			GL_CALL(glTextureStorage1D,
//				m_ID,
//				std::min(mipMaps, maxMipMaps(_w)),
//				gl_enum_cast(textureFormat),
//				_w
//			);
//		}
//		void textureStorage2D(int32_t _w, int32_t _h, gl::TextureFormat textureFormat, int32_t mipMaps = 1)const
//		{
//			assert(mipMaps > 0);
//			GL_CALL(glTextureStorage2D,
//				m_ID,
//				std::min(mipMaps, maxMipMaps(_w, _h)),
//				gl_enum_cast(textureFormat),
//				_w,
//				_h
//			);
//		}
//		void textureStorage3D(int32_t _w, int32_t _h, int32_t _d, gl::TextureFormat textureFormat, int32_t mipMaps = 1)const
//		{
//			assert(mipMaps > 0);
//			GL_CALL(glTextureStorage3D,
//				m_ID,
//				std::min(mipMaps, maxMipMaps(_w, _h, _d)),
//				gl_enum_cast(textureFormat),
//				_w,
//				_h,
//				_d
//			);
//		}
//		void textureStorage2DMultiSample(int32_t _w, int32_t _h, gl::TextureFormat textureFormat, int32_t numSamples, bool fixedSampleLocations = true)const
//		{
//			GL_CALL(glTextureStorage2DMultisample,
//				m_ID,
//				numSamples,
//				gl_enum_cast(textureFormat),
//				_w,
//				_h,
//				fixedSampleLocations
//			);
//		}
//		void textureStorage3DMultiSample(int32_t w, int32_t h, int32_t d, gl::TextureFormat textureFormat, int32_t numSamples, bool fixedSampleLocations = true)const
//		{
//			GL_CALL(glTextureStorage3DMultisample,
//				m_ID,
//				numSamples,
//				gl_enum_cast(textureFormat),
//				w,
//				h,
//				d,
//				fixedSampleLocations
//			);
//		}
//		void textureSubImage1D(utility::const_span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, int32_t x, int32_t w, int32_t level = 0)const
//		{
//			GL_CALL(glTextureSubImage1D, m_ID, level, x, w, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
//		}
//		void textureSubImage2D(utility::const_span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, int32_t x, int32_t y, int32_t w, int32_t h, int32_t level = 0)const
//		{
//			GL_CALL(glTextureSubImage2D, m_ID, level, x, y, w, h, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
//		}
//		void textureSubImage2DForCubeMap(utility::const_span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, CubeMapFace face, int32_t x, int32_t y, int32_t w, int32_t h, int32_t level = 0)const
//		{
//			Bind(TextureTarget::TextureCubeMap);
//			GL_CALL(glTextureSubImage2D, gl_enum_cast(face), level, x, y, w, h, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
//		}
//		void textureSubImage3D(utility::const_span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
//		{
//			GL_CALL(glTextureSubImage3D, m_ID, level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
//		}
//		void getTextureSubImage3D(utility::span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
//		{
//			GL_CALL(glGetTextureSubImage, m_ID, level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), static_cast<int32_t>(pixels.byteSize()), pixels.begin_ptr);
//		}
//		void getTextureSubImage3DForCubeMap(utility::span<void> pixels, gl::ExternalTextureFormat dataFormat, gl::DataType dataType, CubeMapFace face, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
//		{
//			Bind(TextureTarget::TextureCubeMap);
//			GL_CALL(glGetTextureSubImage, gl_enum_cast(face), level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), static_cast<int32_t>(pixels.byteSize()), pixels.begin_ptr);
//		}
//
//		void BindImageTexture(gl::TextureFormat pixelFormat, int32_t ImageUnit, gl::WriteAccess writeAccess) const {
//			GL_CALL(glBindImageTexture, ImageUnit, m_ID, 0, GL_FALSE, 0, gl_enum_cast(writeAccess), gl_enum_cast(pixelFormat));
//		}
//		void Bind(TextureTarget target)const
//		{
//			GL_CALL(glBindTexture, gl_enum_cast(target), m_ID);
//		}
//		static void ActiveUnit(int32_t unit)
//		{
//			GL_CALL(glActiveTexture, GL_TEXTURE0 + unit);
//		}
//
//		const uint64_t& getNVIDIAHandle()const
//		{
//			assert(handle != 0);
//			return handle;
//		}
//
//	private:
//		static int32_t maxMipMaps(int32_t width)
//		{
//			return static_cast<int32_t>(std::bit_width(static_cast<uint32_t>(width)));
//		}
//		static int32_t maxMipMaps(int32_t width, int32_t height)
//		{
//			return maxMipMaps(std::max(width, height));
//		}
//		static int32_t maxMipMaps(int32_t width, int32_t height, int32_t depth)
//		{
//			return maxMipMaps(std::max(width, std::max(depth, height)));
//		}
//	};
//
//
//	constexpr static size_t TextureTargets = static_cast<size_t>(TextureTarget::END);
//
//	struct TextureBindParameters
//	{
//		int32_t TextureID;
//		int32_t SamplerID;
//		int32_t outUnit;
//	};
//
//
//	class TextureBatcher
//	{
//		std::vector<uint32_t> Samplers;
//		std::vector<uint32_t> IDS;
//		std::vector<int32_t> BatchSizes;
//
//		int32_t numTextureUnits = 0;
//
//		int32_t batchIndex = 0;
//		int32_t flushIndex = 0;
//	public:
//
//		static void Reset()
//		{
//			auto& instance = TextureBatcher::instance();
//			instance.reset();
//		}
//
//		static void BindTextures(utility::span<TextureBindParameters> tasks, int32_t* _batch_index)
//		{
//			auto& instance = TextureBatcher::instance();
//			int32_t position = 0;
//
//			for (auto begin_index = tasks.begin_index(); begin_index != tasks.end_index(); ++begin_index)
//			{
//				position = instance.findUnit(begin_index->TextureID);
//				int32_t unit = position % instance.numTextureUnits;
//				if (unit < 0)
//				{
//					unit = instance.push_back_texture(begin_index->TextureID, begin_index->SamplerID);
//					if (unit < 0)
//					{
//						instance.NewBatch();
//						return BindTextures(tasks, _batch_index);
//					}
//				}
//				begin_index->outUnit = unit;
//			}
//			*_batch_index = position / instance.numTextureUnits;
//		}
//
//		static bool Flush()
//		{
//			auto& instance = TextureBatcher::instance();
//			if (instance.flushIndex != instance.batchIndex + 1)
//			{
//				const int32_t flushBegin = instance.flushIndex * instance.numTextureUnits;
//				const int32_t flushSize = std::min(static_cast<int32_t>(instance.IDS.size()) - flushBegin, instance.numTextureUnits);
//
//				glBindTextures(0, flushSize, instance.IDS.data() + flushBegin);
//				glBindSamplers(0, flushSize, instance.Samplers.data() + flushBegin);
//				instance.flushIndex++;
//			}
//			return instance.flushIndex != instance.batchIndex + 1;
//		}
//
//	private:
//
//		void reset()
//		{
//			Samplers.clear();
//			IDS.clear();
//			batchIndex = 0;
//			flushIndex = 0;
//		}
//
//
//		void NewBatch()
//		{
//			Samplers.resize(batchEnd(batchIndex));
//			IDS.resize(batchEnd(batchIndex));
//			++batchIndex;
//		}
//
//		int32_t push_back_texture(int32_t ID, int32_t SamplerID)
//		{
//			auto unit = static_cast<int32_t>(IDS.size());
//			if (unit < numTextureUnits * (batchIndex + 1))
//			{
//				IDS.push_back(ID);
//				Samplers.push_back(SamplerID);
//			}
//			else
//			{
//				unit = -1;
//			}
//			return unit;
//		}
//
//		int32_t findUnit(uint32_t TextureID)
//		{
//			const auto it = std::ranges::find(IDS.begin_index() + batchBegin(batchIndex), IDS.end_index(), TextureID);
//			return it == IDS.end_index() ? -1 : static_cast<int32_t>(std::distance(IDS.begin_index(), it));
//		}
//
//
//		int32_t batchBegin(int32_t batch)const
//		{
//			return batch * numTextureUnits;
//		}
//		int32_t batchEnd(int32_t batch)const
//		{
//			return (batch + 1) * (numTextureUnits);
//		}
//		TextureBatcher()
//		{
//			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &numTextureUnits);
//			reset();
//		}
//
//		static TextureBatcher& instance()
//		{
//			static TextureBatcher _instance;
//			return _instance;
//		}
//
//	};
//
//	
//
//	struct BufferObject
//	{
//	private:
//		GLuint ID = 0;
//		int32_t bound_unit = -1;
//		uint64_t handle = 0;
//
//	public:
//		enum Flags {
//			None = 0x0000,
//			DynamicStorage = GL_DYNAMIC_STORAGE_BIT, // Typically for buffers that are modified repeatedly and used many times.
//			KeepCopyCpu = GL_CLIENT_STORAGE_BIT, // Typically for buffers that the client (CPU) will read from or write to.
//			ReadAccess = GL_MAP_READ_BIT, // For buffer mapping, indicates that the returned pointer can be read.
//			WriteAccess = GL_MAP_WRITE_BIT, // For buffer mapping, indicates that the returned pointer can be written to.
//			CoherentMapping = GL_MAP_COHERENT_BIT, // For coherent mapping of buffer data.
//			MapInvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT, // Indicates that a subrange of the buffer may be invalidated.
//			MapInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT, // Indicates that the entire buffer may be invalidated.
//
//			PRESETDynamic = DynamicStorage | WriteAccess,
//			PRESETImmutable = ReadAccess,
//			PRESETStreamDraw = DynamicStorage | MapInvalidateRange, 
//			PRESETStaticDraw = WriteAccess, 
//			PRESETCoherentRead = ReadAccess | CoherentMapping,
//			PRESETCoherentWrite = WriteAccess | CoherentMapping,
//			PRESETCoherentReadWrite = ReadAccess | WriteAccess | CoherentMapping
//		};
//
//
//		BufferObject() = default;
//
//
//		GLuint GetID()const
//		{
//			return ID;
//		}
//		uint64_t GetHandle()const
//		{
//			return handle;
//		}
//
//		static std::unique_ptr<BufferObject> Create()
//		{
//			BufferObject buffer;
//			GL_CALL(glCreateBuffers, 1, &buffer.ID);
//			return std::make_unique<BufferObject>(std::move(buffer));
//		}
//		static BufferObject CreateStack()
//		{
//			BufferObject buffer;
//			GL_CALL(glCreateBuffers, 1, &buffer.ID);
//			return buffer;
//		}
//
//		void BindBufferRange(BufferTarget target, int32_t binding, int32_t offset, int32_t size)const
//		{
//			GL_CALL(glBindBufferRange, gl_enum_cast(target), binding, ID, offset, size);
//		}
//		void BindBufferBase(BufferTarget target, int32_t binding)const
//		{
//			GL_CALL(glBindBufferBase, gl_enum_cast(target), binding, ID);
//		}
//
//
//		BufferObject(BufferObject&& other) noexcept
//			: ID(std::exchange(other.ID, 0)),
//			handle(std::exchange(other.handle, 0))
//		{
//		}
//
//		BufferObject& operator=(BufferObject&& other) noexcept
//		{
//			if (&other != this)
//			{
//				free();
//				ID = std::exchange(other.ID, 0);
//				handle = std::exchange(other.handle, 0);
//			}
//			return *this;
//		}
//		BufferObject(const BufferObject& other) = delete;
//		BufferObject& operator =(const BufferObject& other) = delete;
//
//
//		~BufferObject()
//		{
//			free();
//		}
//
//		void makeBindLess(gl::WriteAccess writeAccess = gl::WriteAccess::Readonly)
//		{
//			GL_CALL(glGetNamedBufferParameterui64vNV, ID, GL_BUFFER_GPU_ADDRESS_NV, &handle);
//			GL_CALL(glMakeNamedBufferResidentNV, ID, gl_enum_cast(writeAccess));
//		}
//
//		void Bind(BufferTarget target)const
//		{
//			GL_CALL(glBindBuffer, gl_enum_cast(target), ID);
//		}
//		void bufferData(utility::const_span<void> read, Flags flags)const
//		{
//			GL_CALL(glNamedBufferStorage, ID, static_cast<int32_t>(read.byteSize()), read.begin_ptr, flags);
//		}
//		void bufferSubData(utility::const_span<void> read, math::Number<int32_t> offset)const
//		{
//			GL_CALL(glNamedBufferSubData, ID, offset, static_cast<int32_t>(read.byteSize()), read.begin_ptr);
//		}
//		void getBufferSubData(utility::span<void> write, math::Number<int32_t> offset)const
//		{
//			GL_CALL(glGetNamedBufferSubData, ID, offset, static_cast<int32_t>(write.byteSize()), write.begin_ptr);
//		}
//		void* map(utility::const_span<void> area,  std::bitset<32> bits)const
//		{
//			return glMapNamedBufferRange(ID, area.offset(), area.byteSize(), bits.to_ulong());
//		}
//		void unMap()const
//		{
//			glUnmapNamedBuffer(ID);
//		}
//
//		void free()
//		{
//			if (handle)
//			{
//				GL_CALL(glMakeNamedBufferNonResidentNV, ID);
//			}
//			if (ID != 0)
//			{
//				GL_CALL(glDeleteBuffers, 1, &ID);
//				ID = 0;
//			}
//		}
//
//		void Bind(GLenum target) const
//		{
//			GL_CALL(glBindBuffer, target, ID);
//		}
//	};
//
//
//
//	class VertexArrayObject
//	{
//		GLuint ID = 0;
//	public:
//
//		enum AttributeTarget : uint32_t
//		{
//			Position = 0,
//			Color = 1,
//			UV0 = 2,
//			Normal = 3,
//			Tangent = 4,
//			Bitangent = 5,
//			UV1X = 6,
//			UV2 = 7,
//			UV3 = 8,
//			UV4 = 9,
//			UV5 = 10,
//			UV6XY = 11,
//			UV7 = 12,
//			UV8 = 13,
//			UV9 = 14,
//			UV10 = 15,
//			UV11 = 16,
//			UV12XYZ = 17,
//			UV13 = 18,
//			UV14 = 19,
//			UV15 = 20,
//			UV16 = 21,
//			UV17 = 22,
//			UV18XYZW = 23,
//			UV19 = 24,
//			UV20 = 25,
//			UV21 = 26,
//			UV22 = 27,
//			UV23 = 28,
//			UV24 = 29,
//			UV25 = 30,
//			UV26 = 31
//		};
//		static int32_t AttributeTargetNumElements(AttributeTarget index)
//		{
//			utility::EnumLinker<AttributeTarget, int32_t> linker{
//				{Position , 3},
//				{Color , 4},
//				{UV0 , 2},
//				{Normal , 3},
//				{Tangent , 4},
//				{Bitangent , 3},
//				{UV1X , 1},
//				{UV2 , 1},
//				{UV3 , 1},
//				{UV4 , 1},
//				{UV5 , 1},
//				{UV6XY , 2},
//				{UV7 , 2},
//				{UV8 , 2},
//				{UV9 , 2},
//				{UV10 , 2},
//				{UV11 , 2},
//				{UV12XYZ , 3},
//				{UV13 , 3},
//				{UV14 , 3},
//				{UV15 , 3},
//				{UV16 , 3},
//				{UV17 , 3},
//				{UV18XYZW , 4},
//				{UV19 , 3},
//				{UV20 , 3},
//				{UV21 , 4},
//				{UV22 , 4},
//				{UV23 , 4},
//				{UV24 , 4},
//				{UV25 , 4},
//				{UV26 , 4},
//			};
//			return linker[index];
//		}
//
//		VertexArrayObject() = default;
//
//		GLuint GetID()const { return ID; }
//
//		static std::unique_ptr<VertexArrayObject> Create()
//		{
//			VertexArrayObject vertexArrayObj;
//			GL_CALL(glCreateVertexArrays, 1, &vertexArrayObj.ID);
//			return std::make_unique<VertexArrayObject>(std::move(vertexArrayObj));
//		}
//		static VertexArrayObject CreateStack()
//		{
//			VertexArrayObject vertexArrayObj;
//			GL_CALL(glCreateVertexArrays, 1, &vertexArrayObj.ID);
//			return vertexArrayObj;
//		}
//		VertexArrayObject(VertexArrayObject&& other)noexcept
//			:ID(std::exchange(other.ID, 0))
//		{
//
//		}
//		VertexArrayObject& operator=(VertexArrayObject&& other)noexcept
//		{
//			if (&other != this)
//			{
//				free();
//				ID = std::exchange(other.ID, 0);
//			}
//			return *this;
//		}
//		VertexArrayObject(const VertexArrayObject& other) = delete;
//		VertexArrayObject& operator =(const VertexArrayObject& other) = delete;
//
//
//		void VertexArrayAttribIFormat(AttributeTarget target, int32_t stride, int32_t relativeOffset, gl::DataType dataType, const BufferObject* buffer, int32_t bufferBindingIndex, int32_t bufferOffset = 0)const
//		{
//			const auto attribIndex = static_cast<int32_t>(target);
//			const int32_t numElements = AttributeTargetNumElements(target);
//			const GLuint BufferID = buffer->GetID();
//			GL_CALL(glVertexArrayAttribBinding, ID, attribIndex, bufferBindingIndex);
//			GL_CALL(glVertexArrayVertexBuffer, ID, bufferBindingIndex, BufferID, bufferOffset, stride);
//			GL_CALL(glVertexArrayAttribIFormat, ID, attribIndex, numElements, gl_enum_cast(dataType), relativeOffset);
//			Activate(target);
//		}
//		void VertexArrayAttribFormat(AttributeTarget target, int32_t stride, int32_t relativeOffset, gl::DataType dataType, const BufferObject* buffer, int32_t bufferBindingIndex = 0, int32_t bufferOffset = 0, bool normalize = false)const
//		{
//			GL_CALL(glVertexArrayAttribBinding, ID, static_cast<int32_t>(target), bufferBindingIndex);
//			GL_CALL(glVertexArrayVertexBuffer, ID, bufferBindingIndex, buffer->GetID(), bufferOffset, stride);
//			GL_CALL(glVertexArrayAttribFormat, ID, static_cast<int32_t>(target), AttributeTargetNumElements(target), gl_enum_cast(dataType), normalize, relativeOffset);
//			Activate(target);
//		}
//		void Activate(AttributeTarget target)const
//		{
//			GL_CALL(glEnableVertexArrayAttrib, ID, static_cast<int32_t>(target));
//		}
//		void Deactivate(AttributeTarget target)const
//		{
//			GL_CALL(glDisableVertexArrayAttrib, ID, static_cast<int32_t>(target));
//		}
//
//		void Bind()const
//		{
//			GL_CALL(glBindVertexArray, ID);
//		}
//
//
//		~VertexArrayObject()
//		{
//			free();
//		}
//
//
//		void free()
//		{
//			if (ID != 0)
//			{
//				GL_CALL(glDeleteVertexArrays, 1, &ID);
//				ID = 0;
//			}
//		}
//	};
//
//
//
//
//	class Sampler
//	{
//		SamplerObject sampler;
//
//		Sampler() = default;
//	public:
//
//
//		static std::shared_ptr<Sampler> Create(SamplerObject&& _sampler = SamplerObject::CreateStack())
//		{
//			return std::shared_ptr<Sampler>(new Sampler(std::move(_sampler)));
//		}
//
//
//		void WrapMode(gl::WrapMode wrapMode, GLbitfield target = 0b111)const
//		{
//			if (target & 0b1) {
//				sampler.samplerParameterI(SamplerObject::WrapR, gl_enum_cast(wrapMode));
//			}
//			if (target & 0b10) {
//				sampler.samplerParameterI(SamplerObject::WrapS, gl_enum_cast(wrapMode));
//			}
//			if (target & 0b100) {
//				sampler.samplerParameterI(SamplerObject::WrapT, gl_enum_cast(wrapMode));
//			}
//		}
//		void MagFilter(gl::WrapMode filterMode)const
//		{
//			sampler.samplerParameterI(SamplerObject::MagFilter, gl_enum_cast(filterMode));
//		}
//		void MinFilter(gl::WrapMode filterMode)const
//		{
//			sampler.samplerParameterI(SamplerObject::MinFilter, gl_enum_cast(filterMode));
//		}
//		void SamplerParameterI(SamplerObject::ParametersI parameter, int32_t value)const
//		{
//			sampler.samplerParameterI(parameter, value);
//		}
//		void SamplerParameterF(SamplerObject::ParametersF parameter, float value)const
//		{
//			sampler.samplerParameterF(parameter, value);
//		}
//		void SamplerParameterFV(SamplerObject::ParametersFV parameter, const float* value)const
//		{
//			sampler.samplerParameterFV(parameter, value);
//		}
//
//
//		operator const SamplerObject& ()const
//		{
//			return sampler;
//		}
//		operator const SamplerObject* ()const
//		{
//			return &sampler;
//		}
//		GLuint GetID()const
//		{
//			return sampler.GetID();
//		}
//
//	private:
//		Sampler(SamplerObject&& _sampler) :sampler(std::move(_sampler))
//		{
//
//		}
//
//	};
//
//
//	struct TextureProperties
//	{
//		int32_t width = 1;
//		int32_t height = 1;
//		int32_t depth = 1;
//		int32_t numMipMaps = 1;
//		int32_t numSamples = 1;
//
//
//		TextureTarget textureTarget = TextureTarget::Texture2D;
//		TextureFormat textureFormat = TextureFormat::Auto;
//
//		TextureProperties() = default;
//
//		TextureProperties(TextureTarget _textureTarget, TextureFormat _textureFormat, int32_t _w, int32_t _h, int32_t _d, int32_t _mipMaps, int32_t _samples)
//			:
//			width(_w),
//			height(_h),
//			depth(_d),
//			numMipMaps(_mipMaps),
//			numSamples(_samples),
//			textureTarget(_textureTarget),
//			textureFormat(_textureFormat)
//		{
//
//		}
//	};
//
//
//
//	class Texture
//	{
//	protected:
//		TextureObject texture;
//		TextureProperties properties;
//		std::shared_ptr<Sampler> sampler;
//	public:
//
//		int32_t width()const { return properties.width; }
//		int32_t height()const { return properties.height; }
//		int32_t depth()const { return properties.depth; }
//
//		void Bind()const
//		{
//			texture.Bind(properties.textureTarget);
//		}
//		void Bind(int32_t unit)const
//		{
//			glBindSampler(unit, sampler ? sampler->GetID() : 0);
//			glBindTextureUnit(unit, texture.GetID());
//		}
//
//		
//		void MakeBindLessNVIDIA()
//		{
//			return texture.makeBindLess(*sampler);
//		}
//		glm::uvec2 GetNVIDIATextureHandle()
//		{
//			return reinterpret_cast<const glm::uvec2&>(texture.getNVIDIAHandle());
//		}
//		gl::TextureFormat GetTextureFormat()const {
//			return properties.textureFormat;
//		}
//		const std::shared_ptr<Sampler>& GetSampler()const {
//			return sampler;
//		}
//		void SetSampler(std::shared_ptr<Sampler> sampler) {
//			sampler = std::move(sampler);
//		}
//		const TextureObject& GetBase()
//		{
//			return texture;
//		}
//
//		void BindToImageUnit(int32_t unit, WriteAccess access, int32_t level = 0, int32_t layer = 0)const
//		{
//			glBindImageTexture(unit, texture.GetID(), level, IsTargetLayered(), layer, gl_enum_cast(access), gl_enum_cast(GetTextureFormat()));
//		}
//		
//
//		operator const TextureObject& ()const
//		{
//			return texture;
//		}
//		operator const TextureObject* ()const
//		{
//			return &texture;
//		}
//		operator const std::shared_ptr<Sampler>& ()const
//		{
//			return sampler;
//		}
//
//		void resize(int32_t w, int32_t h, int32_t d) {
//
//			properties.width = w;
//			properties.height = h;
//			properties.depth = d;
//
//			switch (properties.textureTarget) {
//			case TextureTarget::Texture1D:
//			case TextureTarget::TextureBuffer:
//				texture.textureStorage1D(properties.width, properties.textureFormat, properties.numMipMaps);
//				break;
//
//			case TextureTarget::Texture2D:
//			case TextureTarget::TextureCubeMap:
//			case TextureTarget::Texture1DArray:
//				texture.textureStorage2D(properties.width, properties.height, properties.textureFormat, properties.numMipMaps);
//				break;
//
//
//			case TextureTarget::TextureCubeMapArray:
//			case TextureTarget::Texture2DArray:
//			case TextureTarget::Texture3D:
//				texture.textureStorage3D(properties.width, properties.height,properties.depth, properties.textureFormat, properties.numMipMaps);
//				break;
//
//			case TextureTarget::Texture2DMultisample:
//				texture.textureStorage2DMultiSample(properties.width, properties.height, properties.textureFormat, properties.numSamples);
//				break;
//
//			case TextureTarget::Texture2DMultisampleArray:
//				texture.textureStorage3DMultiSample(properties.width, properties.height, properties.depth, properties.textureFormat, properties.numSamples);
//				break;
//			}
//
//		}
//
//
//
//	protected:
//		Texture(TextureObject&& txtObject, const TextureProperties& settings, std::shared_ptr<Sampler> _sampler) :
//			texture(std::move(txtObject)), properties(settings), sampler(std::move(_sampler))
//		{
//
//		}
//
//
//
//
//		bool IsTargetLayered()const
//		{
//			switch (properties.textureTarget)
//			{
//			case TextureTarget::Texture1D:
//			case TextureTarget::Texture2D:
//			case TextureTarget::Texture3D:
//				return false;
//			case TextureTarget::Texture1DArray:
//			case TextureTarget::Texture2DArray:
//				return true;
//			case TextureTarget::TextureRectangle:
//			case TextureTarget::TextureCubeMap:
//				return false;
//			case TextureTarget::TextureCubeMapArray:
//				return true;
//			case TextureTarget::TextureBuffer:
//			case TextureTarget::Texture2DMultisample:
//				return false;
//			case TextureTarget::Texture2DMultisampleArray:
//				return true;
//			default:
//				return false;
//			}
//		}
//	};
//
//	class Texture2D :public Texture
//	{
//	public:
//		static std::shared_ptr<Texture2D> Create(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t numMipMaps = 1,
//			const std::shared_ptr<Sampler>& _textureSampler = nullptr,
//			TextureObject&& obj = TextureObject::CreateStack(TextureTarget::Texture2D)
//		)
//		{
//			return std::shared_ptr<Texture2D>(new Texture2D(textureFormat, _w, _h, numMipMaps, _textureSampler, std::move(obj)));
//		}
//
//		void TextureStorage2D()const
//		{
//			texture.textureStorage2D(properties.width, properties.height, properties.textureFormat, properties.numMipMaps);
//		}
//		void ReAllocate(TextureFormat textureFormat, int32_t w, int32_t h, int32_t mipmaps) {
//			properties.width = w;
//			properties.height = h;
//			properties.numMipMaps = mipmaps;
//			properties.textureFormat = textureFormat;
//			TextureStorage2D();
//		}
//
//
//		void TextureSubImage2D(utility::const_span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.textureSubImage2D(PixelSrc, dataFormat, dataType, x, y, w, h, level);
//		}
//		void GetTextureSubImage2D(utility::span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.getTextureSubImage3D(PixelSrc, dataFormat, dataType, x, y, 1, w, h, 1, level);
//		}
//
//		int32_t width()const { return properties.width; }
//		int32_t height()const { return properties.height; }
//
//	private:
//		Texture2D(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t numMipMaps,
//			std::shared_ptr<Sampler> _sampler,
//			TextureObject&& obj
//		) : Texture(std::move(obj), { TextureTarget::Texture2D, textureFormat, _w, _h, 1, numMipMaps, 0 }, std::move(_sampler))
//		{
//		}
//	};
//
//	class Texture2DArray :public Texture
//	{
//	public:
//		static std::shared_ptr<Texture2DArray> Create(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t _l, int32_t numMipMaps,
//			const std::shared_ptr<Sampler>& _textureSampler,
//			TextureObject&& obj = TextureObject::CreateStack(TextureTarget::Texture2D)
//		)
//		{
//			return  std::shared_ptr<Texture2DArray>(new Texture2DArray(textureFormat, _w, _h, _l, numMipMaps, _textureSampler, std::move(obj)));
//		}
//
//		void Texture2DArrayStorage()const
//		{
//			texture.textureStorage3D(properties.width, properties.height, properties.depth, properties.textureFormat, properties.numMipMaps);
//		}
//		void Texture2DArraySubImage(utility::const_span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			int32_t layer,
//			int32_t numLayers = 1,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.textureSubImage3D(PixelSrc, dataFormat, dataType, x, y, layer, w, h, numLayers, level);
//		}
//		void GetTexture2DArraySubImage(utility::span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			int32_t layer,
//			int32_t numLayers = 1,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.getTextureSubImage3D(PixelSrc, dataFormat, dataType, x, y, layer, w, h, numLayers, level);
//		}
//
//		int32_t width()const { return properties.width; }
//		int32_t height()const { return properties.height; }
//
//	private:
//		Texture2DArray(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t _l, int32_t numMipMaps,
//			std::shared_ptr<Sampler> _sampler,
//			TextureObject&& obj
//		) : Texture(std::move(obj), { TextureTarget::Texture2D, textureFormat, _w, _h, _l, numMipMaps, 0 }, std::move(_sampler))
//		{
//		}
//	};
//
//	class TextureCubeMap :public Texture
//	{
//	public:
//		static std::shared_ptr<TextureCubeMap> Create(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t numMipMaps,
//			const std::shared_ptr<Sampler>& _textureSampler,
//			TextureObject&& obj = TextureObject::CreateStack(TextureTarget::Texture2D)
//		)
//		{
//			return std::shared_ptr<TextureCubeMap>(new TextureCubeMap(textureFormat, _w, _h, numMipMaps, _textureSampler, std::move(obj)));
//		}
//
//		void TextureStorage2D()const
//		{
//			texture.textureStorage2D(properties.width, properties.height, properties.textureFormat, properties.numMipMaps);
//		}
//		void TextureSubImage2D(utility::const_span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			CubeMapFace face,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			texture.Bind(TextureTarget::TextureCubeMap);
//
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.textureSubImage2DForCubeMap(PixelSrc, dataFormat, dataType, face, x, y, w, h, level);
//		}
//		void GetTextureSubImage2D(utility::span<void> PixelSrc, ExternalTextureFormat dataFormat, DataType dataType,
//			CubeMapFace face,
//			int32_t x = 0,
//			int32_t y = 0,
//			int32_t w = std::numeric_limits<int32_t>::max(),
//			int32_t h = std::numeric_limits<int32_t>::max(),
//			int32_t level = 0
//		)const
//		{
//			w = std::min(w, properties.width - x);
//			h = std::min(h, properties.height - y);
//			texture.getTextureSubImage3DForCubeMap(PixelSrc, dataFormat, dataType, face, x, y, 1, w, h, 1, level);
//		}
//
//		int32_t width()const { return properties.width; }
//		int32_t height()const { return properties.height; }
//		int32_t layers()const { return properties.depth; }
//
//	private:
//		TextureCubeMap(
//			TextureFormat textureFormat,
//			int32_t _w, int32_t _h, int32_t numMipMaps,
//			std::shared_ptr<Sampler> _sampler,
//			TextureObject&& obj
//		) : Texture(std::move(obj), { TextureTarget::Texture2D, textureFormat, _w, _h, 1, numMipMaps, 0 }, std::move(_sampler))
//		{
//		}
//	};
//
//	
//	
//
//
//	
//
//
//	
//	struct RenderConfiguration
//	{
//		using formatMask = std::bitset<static_cast<size_t>(TextureFormat::Auto)>;
//
//
//		constexpr static uint32_t GetTargetFromBit(FrameBufferTarget bits)
//		{
//			return std::bit_width(static_cast<uint64_t>(bits))-1;
//		}
//		
//
//		RenderConfiguration() :
//			configuration(defaultConfig())
//		{
//		}
//
//		static std::shared_ptr<RenderConfiguration> Create(const nlohmann::json& jsonObject = nlohmann::json())
//		{
//			return std::make_shared<RenderConfiguration>(jsonObject);
//		}
//
//		RenderConfiguration(const nlohmann::json& jsonObject):
//			configuration(defaultConfig())
//		{
//			for(const auto& [key, value] : jsonObject.items())
//			{
//				auto _key = magic_enum::enum_cast<FrameBufferTarget>(key);
//				auto temp = value.get<std::string>();
//				auto _value = magic_enum::enum_cast<TextureFormat>(temp);
//				if(_key.has_value() && _value.has_value()){
//					uint32_t index = GetTargetFromBit(_key.value());
//					configuration[index] = _value.value();
//				}else{
//					throw std::runtime_error("Enum Value cannot be parsed");
//				}
//			}
//		}
//
//		
//
//		TextureFormat GetTextureFormat(FrameBufferTarget target)const
//		{
//			return configuration[GetTargetFromBit(target)];
//		}
//		std::shared_ptr<Texture2D> Build2D(FrameBufferTarget target, int32_t width, int32_t height, int32_t mipMaps = 1)const
//		{
//			auto txt = Texture2D::Create(GetTextureFormat(target), width, height, mipMaps, nullptr);
//			txt->TextureStorage2D();
//			return txt;
//		}
//		std::shared_ptr<TextureCubeMap> BuildCubeMap(FrameBufferTarget target, int32_t width, int32_t height, int32_t mipMaps = 1)const
//		{
//			auto txt = TextureCubeMap::Create(GetTextureFormat(target), width, height, mipMaps, nullptr);
//			txt->TextureStorage2D();
//			return txt;
//		}
//
//		static bool IsFormatAllowed(FrameBufferTarget target, TextureFormat pixelFormat)
//		{
//			const uint32_t index = GetTargetFromBit(target);
//			formatMask mask = FormatMask()[index];
//
//			return (getFormatBit(pixelFormat) & mask).to_ulong();
//		}
//
//	private:
//		constexpr static int32_t numTargets = std::bit_width(static_cast<uint64_t>(FrameBufferTarget::Stencil)+1);
//
//		std::array<TextureFormat, numTargets> configuration;
//
//		static std::array<TextureFormat, numTargets> defaultConfig()
//		{
//			std::array<TextureFormat, numTargets> config{};
//			for(size_t i = 0;i< numTargets;i++)
//			{
//				formatMask set = FormatMask()[i];
//				for(size_t b = 0;b< set.size(); b++)
//				{
//					if(set[b])
//					{
//						config[i] = static_cast<TextureFormat>(b);
//						break;
//					}
//				}
//			}
//			return config;
//		}
//		static formatMask getFormatBit(TextureFormat pixelFormat)
//		{
//			return 1ull << static_cast<size_t>(pixelFormat);
//		}
//		static formatMask getFormatBit(std::initializer_list<TextureFormat> formats)
//		{
//			formatMask mask = 0;
//			for (const auto pixelFormat : formats)
//			{
//				mask |= getFormatBit(pixelFormat);
//			}
//			return mask;
//		}
//		static utility::EnumLinker<uint32_t, formatMask>& FormatMask()
//		{
//			using Enum = TextureFormat;
//			static utility::EnumLinker<uint32_t, formatMask> allowedFormats{
//				{GetTargetFromBit(FrameBufferTarget::Albedo), getFormatBit({Enum::RGBA8})},
//				{GetTargetFromBit(FrameBufferTarget::WorldNormal), getFormatBit({Enum::RGB10_A2})},
//				{GetTargetFromBit(FrameBufferTarget::ScreenNormal), getFormatBit({Enum::RGB10_A2})},
//				{GetTargetFromBit(FrameBufferTarget::WorldPosition), getFormatBit({Enum::RGBA16})},
//				{GetTargetFromBit(FrameBufferTarget::RoughnessMetallicSpecular), getFormatBit({Enum::RGBA8})},
//				{GetTargetFromBit(FrameBufferTarget::AmbientOcclusion), getFormatBit({Enum::
// })},
//				{GetTargetFromBit(FrameBufferTarget::Emissive), getFormatBit({Enum::RGBA8})},
//				{GetTargetFromBit(FrameBufferTarget::Velocity), getFormatBit({Enum::RGB10_A2})},
//				{GetTargetFromBit(FrameBufferTarget::Depth), getFormatBit({Enum::DepthComponent24})},
//				{GetTargetFromBit(FrameBufferTarget::Stencil), getFormatBit({Enum::Depth32FStencil8})}
//			};
//			return allowedFormats;
//		}
//	};
//	class FrameBufferObject;
//
//	class TextureComponent :public component::ComponentBase, private TextureProperties
//	{
//		friend class FrameBufferObject;
//		TextureObject object;
//		std::shared_ptr<Sampler> sampler;
//	public:
//		void initialize(const TextureProperties& properties)
//		{
//			object = TextureObject::CreateStack(properties.textureTarget);
//			*static_cast<TextureProperties*>(this) = properties;
//			allocate();
//		}
//		void resize(int32_t w, int32_t h = 1, int32_t d = 1) {
//
//			width = w;
//			height = h;
//			depth = d;
//			allocate();
//		}
//
//
//
//		void Bind()const
//		{
//			object.Bind(textureTarget);
//		}
//		void Bind(int32_t unit)const
//		{
//			glBindSampler(unit, sampler ? sampler->GetID() : 0);
//			glBindTextureUnit(unit, object.GetID());
//		}
//		gl::TextureTarget GetTextureTarget()const {
//			return textureTarget;
//		}
//		gl::TextureFormat GetTextureFormat()const {
//			return textureFormat;
//		}
//		const std::shared_ptr<Sampler>& GetSampler()const {
//			return sampler;
//		}
//		void SetSampler(std::shared_ptr<Sampler> _sampler) {
//			sampler = std::move(_sampler);
//		}
//		const TextureObject& GetBase()const
//		{
//			return object;
//		}
//
//		void BindToImageUnit(int32_t unit, WriteAccess access, int32_t level = 0, int32_t layer = 0)const
//		{
//			glBindImageTexture(unit, object.GetID(), level, IsTargetLayered(textureTarget), layer, gl_enum_cast(access), gl_enum_cast(GetTextureFormat()));
//		}
//
//
//		operator const TextureObject& ()const
//		{
//			return object;
//		}
//		operator const TextureObject* ()const
//		{
//			return &object;
//		}
//		operator const std::shared_ptr<Sampler>& ()const
//		{
//			return sampler;
//		}
//
//	private:
//		void allocate()const
//		{
//			switch(textureTarget)
//			{
//			case TextureTarget::Texture1D:
//				object.textureStorage1D(width, textureFormat, numMipMaps);
//				break;
//			case TextureTarget::Texture1DArray:
//			case TextureTarget::Texture2D:
//			case TextureTarget::TextureCubeMap:
//				object.textureStorage2D(width, height, textureFormat, numMipMaps);
//				break;
//			case TextureTarget::Texture3D:
//			case TextureTarget::TextureCubeMapArray:
//			case TextureTarget::Texture2DArray:
//				object.textureStorage3D(width, height, depth, textureFormat, numMipMaps);
//				break;
//			case TextureTarget::Texture2DMultisample:
//				object.textureStorage2DMultiSample(width, height, textureFormat, numSamples);
//				break;
//			case TextureTarget::Texture2DMultisampleArray:
//				object.textureStorage3DMultiSample(width, height, depth, textureFormat, numSamples);
//			default:
//				break;
//			}
//		}
//
//
//		static constexpr bool IsTargetLayered(TextureTarget target)
//		{
//			switch (target)
//			{
//			case TextureTarget::Texture1D:
//			case TextureTarget::Texture2D:
//			case TextureTarget::Texture3D:
//				return false;
//			case TextureTarget::Texture1DArray:
//			case TextureTarget::Texture2DArray:
//				return true;
//			case TextureTarget::TextureRectangle:
//			case TextureTarget::TextureCubeMap:
//				return false;
//			case TextureTarget::TextureCubeMapArray:
//				return true;
//			case TextureTarget::TextureBuffer:
//			case TextureTarget::Texture2DMultisample:
//				return false;
//			case TextureTarget::Texture2DMultisampleArray:
//				return true;
//			case TextureTarget::END:
//				return false;
//			}
//		}
//
//	};
//	
//	class FrameBufferObject:public component::ComponentBase
//	{
//		friend class scene::Window;
//		std::unordered_map<FrameBufferTarget, component::Entity<TextureComponent>> attachments;
//
//		FrameBufferObjectBase object;
//		mutable std::bitset<64> attachedTargets = static_cast<uint64_t>(FrameBufferTarget::Albedo| FrameBufferTarget::Depth);
//		int32_t activeLod = 0;
//		int32_t numMipMaps = 1;
//
//		mutable std::vector<uint32_t> drawBufferList;
//		mutable std::bitset<32> latestDrawBuffers = false;
//		mutable uint32_t latestReadBuffer = false;
//
//		int32_t w, h, d;
//
//
//		std::string name;
//
//		entt::registry* registry;
//
//	public:
//		FrameBufferObject() : numMipMaps(1), activeLod(0), w(1), h(1), d(1), name("Default"), registry(nullptr)
//		{
//
//		}
//		void Init(const component::ManualEntity& entity)
//		{
//			this->registry = entity.GetRegistry();
//		}
//
//		void initialize( std::string _name ,int32_t _w, int32_t _h, int32_t _d = 1, int32_t mipMaps = 1)
//		{
//			object = FrameBufferObjectBase::CreateStack();
//			w = _w;
//			h = _h;
//			d = _d;
//			numMipMaps = mipMaps;
//			attachedTargets = 0ULL;
//			name = std::move(_name);
//		}
//		
//
//		void EmplaceTexture(FrameBufferTarget targetBit, TextureFormat _textureFormat, TextureTarget _textureTarget = TextureTarget::Texture2D,std::shared_ptr<graphics::Sampler> sampler = nullptr, int32_t numLayers = 0, int32_t _samples = 1)
//		{
//				component::Entity<TextureComponent> entity = component::Entity<>(registry);
//
//				auto& [tex] = entity.tie();
//				tex->initialize(TextureProperties(_textureTarget, _textureFormat, w, h, numLayers > 0 ? numLayers : d, numMipMaps, _samples));
//				tex->SetSampler(std::move(sampler));
//				AttachTexture(targetBit, std::move(entity));
//		}
//		const TextureComponent& operator[](FrameBufferTarget target)
//		{
//			return attachments[target].GetStoredComponent<TextureComponent>();
//		}
//
//
//		void AttachTexture(FrameBufferTarget targetBit, component::Entity<TextureComponent>&& entity)
//		{
//			auto it = attachments.find(targetBit);
//
//			if (it != attachments.end_index())
//			{
//				throw std::runtime_error("TargetAlreadyAttached");
//			}
//			if(it == attachments.end_index())
//			{
//				it = attachments.emplace(targetBit, std::move(entity)).first;
//			}
//			
//
//			const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
//			attachedTargets |= 1ULL << target;
//			bindTexture(targetBit);
//		}
//		component::Entity<TextureComponent> DetachTexture(FrameBufferTarget targetBit)
//		{
//			const auto it = attachments.find(targetBit);
//			if (it == attachments.end_index())
//			{
//				throw std::runtime_error("Target was not present");
//			}
//
//			const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
//			attachedTargets &= ~(1ULL << target);
//
//			component::Entity<TextureComponent> temp = std::move(it->second);
//			attachments.erase(it);
//			return temp;
//		}
//
//		void bindTexture(FrameBufferTarget targetBit, int32_t layer = 0, CubeMapFace face = CubeMapFace::NEGATIVE_X)
//		{
//			if(object)
//			{
//				const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
//				attachedTargets |= (1ULL << target);//todo error prone
//
//				const auto& texture = attachments[targetBit].GetStoredComponent<TextureComponent>();
//				const auto& base = texture.GetBase();
//
//				switch (texture.GetTextureTarget())
//				{
//				case TextureTarget::Texture1D:
//				case TextureTarget::Texture2D:
//					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod);
//					break;
//				case TextureTarget::Texture3D:
//				case TextureTarget::Texture1DArray:
//				case TextureTarget::Texture2DArray:
//				case TextureTarget::Texture2DMultisampleArray:
//				case TextureTarget::TextureCubeMapArray:
//					glNamedFramebufferTextureLayer(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod, layer);
//					break;
//				case TextureTarget::TextureRectangle:
//					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), 0);
//					break;
//				case TextureTarget::TextureCubeMap:
//					glNamedFramebufferTextureLayer(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod, gl_enum_cast(face));
//					break;
//				case TextureTarget::TextureBuffer:
//				case TextureTarget::Texture2DMultisample:
//					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), 0);
//					break;
//				case TextureTarget::END:
//					break;
//				}
//			}
//		}
//
//		void Bind(math::Bounds2D<float> size = math::Bounds2D<float>::Unit(), FrameBufferTarget targets = FrameBufferTarget::ALL)const
//		{
//			if(bindGlobal(this))
//			{
//				if (size.width() < 1.01f && size.height() < 1.01f)
//				{
//					size.x0 *= static_cast<float>(w);
//					size.x1 *= static_cast<float>(w);
//					size.y0 *= static_cast<float>(h);
//					size.y1 *= static_cast<float>(h);
//				}
//				glViewport(
//					static_cast<int32_t>(size.x0),
//					static_cast<int32_t>(size.y0),
//					static_cast<int32_t>(size.x1 - size.x0),
//					static_cast<int32_t>(size.y1 - size.y0)
//				);
//			}
//			drawBuffers(targets);
//		}
//
//		void ClearColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255, FrameBufferTarget targets = FrameBufferTarget::ALL)const
//		{
//			glClearColor(
//				static_cast<float>(r) / 255.0f,
//				static_cast<float>(g) / 255.0f,
//				static_cast<float>(b) / 255.0f,
//				static_cast<float>(a) / 255.0f
//			);
//
//			targets = targets & static_cast<FrameBufferTarget>(attachedTargets.to_ullong());
//			std::bitset<32> colorBits = static_cast<uint32_t>(targets);
//			bool depth = static_cast<bool>(targets & FrameBufferTarget::Depth);
//			bool stencil = static_cast<bool>(targets & FrameBufferTarget::Stencil);
//
//			bindGlobal(this);
//			drawBuffers(targets);
//			GLenum bit = colorBits.any() ? GL_COLOR_BUFFER_BIT : 0;
//			bit |= depth ? GL_DEPTH_BUFFER_BIT : bit;
//			bit |= stencil ? GL_STENCIL_BUFFER_BIT : bit;
//
//			glClear(bit);
//		}
//
//		void SetReadBuffer(FrameBufferTarget read)const
//		{
//			if(read != static_cast<FrameBufferTarget>(0) && object)
//			{
//				readBuffer(read);
//			}
//		}
//		void SetDrawBuffer(FrameBufferTarget src)const
//		{
//			if (src != static_cast<FrameBufferTarget>(0) && object)
//			{
//				drawBuffers(src);
//			}
//		}
//
//
//
//		void Blit(const FrameBufferObject& other, FrameBufferTarget targetOverwrite = FrameBufferTarget::ALL_COLOR | FrameBufferTarget::Depth, math::Bounds2D<float> dst = math::Bounds2D<float>::Unit(), math::Bounds2D<float> src = math::Bounds2D<float>::Unit(),  FilterMode filterMode = FilterMode::Automatic)const
//		{
//			const math::Rectangle srcArea = math::UseBounds2DAsTexSize(src, w, h);
//			const math::Rectangle dstArea = math::UseBounds2DAsTexSize(dst, w, h);
//
//
//			
//
//
//			bindGlobal(this);
//			std::bitset<64> targetOverwriteBits = static_cast<uint64_t>(targetOverwrite);
//
//
//			targetOverwriteBits = targetOverwriteBits & attachedTargets & other.attachedTargets;
//
//			const std::bitset<32> overwriteColorBit = static_cast<uint32_t>(targetOverwriteBits.to_ullong());
//
//
//			
//			enum Operation
//			{
//				Keep,
//				NoColor,
//				Overwrite
//			};
//			const Operation operation = overwriteColorBit.all() ? Keep : overwriteColorBit.none() ? NoColor : Overwrite;
//
//			const bool useColor = operation == Keep||operation == Overwrite;
//			const bool useDepth = targetOverwriteBits.to_ullong() > overwriteColorBit.to_ulong();
//
//			GLenum mask = useColor ? GL_COLOR_BUFFER_BIT : 0;
//			mask |= useDepth ? GL_DEPTH_BUFFER_BIT : 0;
//			//todo stencil buffer blitting
//
//			if(operation == Overwrite)
//			{
//				const FrameBufferTarget target = static_cast<FrameBufferTarget>(overwriteColorBit.to_ulong());
//				SetReadBuffer(target);
//				other.SetDrawBuffer(target);
//			}
//
//
//			if (filterMode == FilterMode::Automatic)
//			{
//				const bool useNearest = (srcArea.width() % dstArea.width() == 0 || dstArea.width() % srcArea.width() == 0) &&
//					(srcArea.height() % dstArea.height() == 0 || dstArea.height() % srcArea.height() == 0);
//				filterMode = useNearest||useDepth ?
//					FilterMode::Nearest : FilterMode::Linear;
//			}
//
//
//
//
//			glBlitNamedFramebuffer(
//				object.GetID(),
//				other.object.GetID(),
//				srcArea.x0,
//				srcArea.y0,
//				srcArea.x1,
//				srcArea.y1,
//				dstArea.x0,
//				dstArea.y0,
//				dstArea.x1,
//				dstArea.y1,
//				mask,
//				gl_enum_cast(filterMode)
//			);
//		}
//	
//
//		void SetLod(int32_t _lod) {
//			if (object) {
//				if (_lod > numMipMaps) {
//					for (size_t i = 0; i < 64; i++) {
//						if (attachedTargets[i]) {
//							bindTexture(FrameBufferTarget::Albedo<<i);
//						}
//					}
//				}
//			}
//
//		}
//		void Resize(int32_t _w, int32_t _h, int32_t _d) {
//			w = _w;
//			h = _h;
//			d = _d;
//			if (object) {
//				for (auto& entity : std::views::values(attachments)) {
//					auto& txt = entity.GetStoredComponent<TextureComponent>();
//					bool isLayered = TextureComponent::IsTargetLayered(txt.GetTextureTarget());
//					if (isLayered)
//					{
//						txt.resize(w, h, txt.depth);
//					}
//					if (!isLayered)
//					{
//						txt.resize(w, h, d);
//					}
//				}
//			}
//		}
//
//		bool operator<(const FrameBufferObject& other) const {
//			return name < other.name;
//		}
//		bool operator==(const FrameBufferObject& other) const {
//			return name == other.name;
//		}
//		const std::string& GetName()const
//		{
//			return name;
//		}
//
//		
//	private:
//
//		template<typename... Targets>
//			requires (std::conjunction_v<std::is_same<FrameBufferTarget, Targets>...>)
//		void SymbolicAttachTexture(Targets... targetBits) const {
//			auto applyHelper = [this](FrameBufferTarget targetBit) {
//				const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
//				attachedTargets |= 1ULL << target;
//				};
//
//			(applyHelper(targetBits), ...);
//		}
//		
//
//		
//		static bool bindGlobal(const FrameBufferObject* renderTexture) {
//			static uint32_t current = 0;
//			uint32_t New = renderTexture->object.GetID();
//			bool isNew = current != New;
//			if (isNew) {
//				current = New;
//				glBindFramebuffer(GL_FRAMEBUFFER, New);
//			}
//
//			return isNew;
//		}
//		void drawBuffers(FrameBufferTarget _targets = FrameBufferTarget::ALL)const {
//			if (object) {
//				std::bitset<32> bits = static_cast<uint32_t>(_targets);
//				bits &= attachedTargets.to_ullong();
//				if (bits.any())
//				{
//					const bool changed = bits == latestDrawBuffers;
//					latestDrawBuffers = bits;
//
//					if (changed) {
//						drawBufferList.clear();
//						for (size_t i = 0; i < 32; i++) {
//							if (latestDrawBuffers[i]) {
//								drawBufferList.emplace_back(GL_COLOR_ATTACHMENT0 + i);
//							}
//						}
//
//						glNamedFramebufferDrawBuffers(object.GetID(), drawBufferList.size(), drawBufferList.data());
//					}
//				}
//			}
//
//		}
//		void readBuffer(FrameBufferTarget _target)const {
//
//			uint32_t readBuffer = std::bit_width(static_cast<uint64_t>(_target));
//			if (readBuffer == latestReadBuffer) {
//				latestReadBuffer = readBuffer;
//				glNamedFramebufferReadBuffer(object.GetID(), GL_COLOR_ATTACHMENT0 + readBuffer - 1);
//			}
//		}
//	public:
//		int32_t width()const
//		{
//			return w;
//		}
//		int32_t height()const
//		{
//			return h;
//		}
//	};
//
//
//
//
//	class FrameBuffer {
//		friend class Window;
//
//		using SharedTexture = std::shared_ptr<Texture>;
//		using SharedTexture2D = std::shared_ptr<Texture2D>;
//		using SharedCubeMap = std::shared_ptr<TextureCubeMap>;
//		using SharedTexture2 = std::tuple<SharedTexture, SharedTexture>;
//		using SharedConfiguration = std::shared_ptr<RenderConfiguration>;
//
//
//
//	public:
//
//
//		int32_t width;
//		int32_t height;
//		int32_t depth;
//		int32_t mipMaps;
//		int32_t activeLod = 0;
//
//		SharedConfiguration textureFactory;
//		FrameBufferObjectBase fbo;
//
//		mutable std::vector<SharedTexture2> textures;
//		mutable std::array<uint8_t, 64> indices;
//		mutable std::bitset<64> emplacedIndices;
//
//		mutable std::vector<uint32_t> drawBufferList;
//		mutable std::bitset<32> latestDrawBuffers = false;
//		mutable uint32_t latestReadBuffer = false;
//
//		mutable bool DrawBufferFlag = true;
//
//
//	public:
//		FrameBuffer()
//			:width(0), height(0), depth(0), mipMaps(0) {
//
//
//		}
//		FrameBuffer(SharedConfiguration _textureFactory, int32_t w, int32_t h, int32_t d = 1, int32_t _mipMaps = 1, FrameBufferObjectBase&& fbo = FrameBufferObjectBase::CreateStack())
//			:width(w), height(h), depth(d), mipMaps(_mipMaps), textureFactory(std::move(_textureFactory)), fbo(std::move(fbo)) {
//
//		}
//		template<typename T = graphics::Texture2D>
//		T& GetTexture(FrameBufferTarget target, T** second = nullptr)
//		{
//			const auto& [f, s] = getTarget(target);
//			T* first = reinterpret_cast<T*>(f.get());
//			if (second)
//			{
//				*second = reinterpret_cast<T*>(s.get());
//			}
//			return *first;
//		}
//
//		void Resize(int32_t w, int32_t h, int32_t d) {
//			width = w;
//			height = h;
//			depth = d;
//			if (fbo) {
//				for (auto& [first, second] : textures) {
//					first->resize(width, height, depth);
//					if (second) {
//						second->resize(width, height, depth);
//					}
//				}
//			}
//		}
//
//
//
//
//		template<typename T>
//		void emplace(FrameBufferTarget _targets, int32_t _mipMaps = 1, FrameBufferTarget _doubleBufferingTargets = FrameBufferTarget(0), std::shared_ptr<Sampler> sampler = nullptr) {}
//
//		template<>
//		void emplace<Texture2D>(FrameBufferTarget _targets, int32_t _mipMaps, FrameBufferTarget _doubleBufferingTargets, std::shared_ptr<Sampler> sampler) {
//			std::bitset<64> first_bits = static_cast<uint64_t>(_targets);
//			std::bitset<64> second_bits = static_cast<uint64_t>(_doubleBufferingTargets);
//			if (fbo) {
//				for (size_t i = 0; i < 64; i++) {
//					if (first_bits[i]) {
//						auto target = FrameBufferTarget::Albedo << i;
//						auto& [first, second] = getTarget(target);
//						first = textureFactory->Build2D(target, width, height, std::max(_mipMaps, mipMaps));
//						first->SetSampler(sampler);
//						if (second_bits[i]) {
//							second = textureFactory->Build2D(target, width, height, std::max(_mipMaps, mipMaps));
//							second->SetSampler(sampler);
//						}
//						attachTexture(i);
//					}
//				}
//			}
//			else {
//				for (size_t i = 0; i < 64; i++) {
//					if (first_bits[i]) {
//						auto target = FrameBufferTarget::Albedo << i;
//						emplacedIndices[i] = true;
//					}
//				}
//			}
//		}
//		template<>
//		void emplace<TextureCubeMap>(FrameBufferTarget _targets, int32_t _mipMaps, FrameBufferTarget _doubleBufferingTargets, std::shared_ptr<Sampler> sampler) {
//
//			std::bitset<64> first_bits = static_cast<uint64_t>(_targets);
//			std::bitset<64> second_bits = static_cast<uint64_t>(_doubleBufferingTargets);
//
//			if (fbo) {
//				for (size_t i = 0; i < 64; i++) {
//					if (first_bits[i]) {
//						auto target = FrameBufferTarget::Albedo << FrameBufferTarget(i);
//						auto& [first, second] = getTarget(target);
//						first = textureFactory->BuildCubeMap(target, width, height, std::max(_mipMaps, mipMaps));
//						first->SetSampler(sampler);
//						if (second_bits[i]) {
//							second = textureFactory->BuildCubeMap(target, width, height, std::max(_mipMaps, mipMaps));
//							second->SetSampler(sampler);
//						}
//						attachTexture(i);
//					}
//				}
//			}
//		}
//		
//
//		template<>
//		void emplace<Texture2DArray>(FrameBufferTarget _targets, int32_t _mipMaps, FrameBufferTarget _doubleBufferingTargets, std::shared_ptr<Sampler> sampler) {
//
//			std::bitset<64> first_bits = static_cast<uint64_t>(_targets);
//			std::bitset<64> second_bits = static_cast<uint64_t>(_doubleBufferingTargets);
//
//			if (fbo) {
//				for (size_t i = 0; i < 64; i++) {
//					if (first_bits[i]) {
//						auto target = FrameBufferTarget::Albedo << FrameBufferTarget(i);
//						auto& [first, second] = getTarget(target);
//						first = textureFactory->BuildCubeMap(target, width, height, std::max(_mipMaps, mipMaps));
//						first->SetSampler(sampler);
//						if (second_bits[i]) {
//							second = textureFactory->BuildCubeMap(target, width, height, std::max(_mipMaps, mipMaps));
//							second->SetSampler(sampler);
//						}
//						attachTexture(i);
//					}
//				}
//			}
//		}
//
//
//
//		void ClearColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255, FrameBufferTarget targets = FrameBufferTarget::ALL) {
//
//			glClearColor(
//				static_cast<float>(r) / 255.0f,
//				static_cast<float>(g) / 255.0f,
//				static_cast<float>(b) / 255.0f,
//				static_cast<float>(a) / 255.0f
//			);
//
//
//			targets = targets & static_cast<FrameBufferTarget>(emplacedIndices.to_ullong());
//			std::bitset<32> colorBits = static_cast<uint32_t>(targets);
//			bool depth = static_cast<bool>(targets & FrameBufferTarget::Depth);
//			bool stencil = static_cast<bool>(targets & FrameBufferTarget::Stencil);
//
//			bindFramebuffer(this);
//			drawBuffers(targets);
//			GLenum bit = colorBits.any() ? GL_COLOR_BUFFER_BIT : 0;
//			bit |= depth ? GL_DEPTH_BUFFER_BIT : bit;
//			bit |= stencil ? GL_STENCIL_BUFFER_BIT : bit;
//
//			glClear(bit);
//		}
//		void SetLod(int32_t _lod) {
//			if (fbo) {
//				if (_lod > mipMaps) {
//					for (size_t i = 0; i < 64; i++) {
//						if (emplacedIndices[i]) {
//							attachTexture(i);
//						}
//					}
//				}
//			}
//
//		}
//
//		void Blit(const FrameBuffer& other, math::Rectangle src, math::Rectangle dst,
//			FrameBufferTarget read, FrameBufferTarget write,
//			FilterMode filterMode = FilterMode::Linear
//		)const {
//
//			bindFramebuffer(this);
//			read = read & static_cast<FrameBufferTarget>(emplacedIndices.to_ullong());
//			write = write & static_cast<FrameBufferTarget>(other.emplacedIndices.to_ullong());
//
//			std::bitset<32> srcColorBits = static_cast<uint32_t>(read);
//			std::bitset<32> dstColorbits = static_cast<uint32_t>(write);
//
//			uint32_t color = srcColorBits.any() + dstColorbits.any();
//			uint32_t depth = (static_cast<bool>(read & FrameBufferTarget::Depth)) + (static_cast<bool>(write & FrameBufferTarget::Depth));
//			uint32_t stencil = (static_cast<bool>(read & FrameBufferTarget::Stencil)) + (static_cast<bool>(write & FrameBufferTarget::Stencil));
//
//
//			if (color) {
//				readBuffer(read);
//				if (other.fbo)
//				{
//					other.drawBuffers(write);
//				}
//			}
//
//
//			
//
//			GLenum mask = color == 2 ? GL_COLOR_BUFFER_BIT : 0;
//			mask |= depth == 2 ? GL_DEPTH_BUFFER_BIT : 0;
//			mask |= stencil == 2 ? GL_STENCIL_BUFFER_BIT : 0;
//
//			glBlitNamedFramebuffer(fbo.GetID(), other.fbo.GetID(), src.x0, src.y0, src.x1, src.y1, dst.x0, dst.y0, dst.x1, dst.y1, mask, gl_enum_cast(filterMode));
//		}
//		void Blit(const FrameBuffer& other,
//			FrameBufferTarget read, FrameBufferTarget write,
//			FilterMode filterMode = FilterMode::Linear
//		)const {
//			Blit(other, GetSize(), other.GetSize(), read, write, filterMode);
//		}
//		void Blit(const FrameBuffer& other,
//			math::Bounds2D<float> src, math::Bounds2D<float> dst,
//			FrameBufferTarget read, FrameBufferTarget write,
//			FilterMode filterMode = FilterMode::Linear
//		)const {
//			Blit(other, GetSize(src), other.GetSize(dst), read, write, filterMode);
//		}
//
//		math::Rectangle GetSize() const {
//			return math::Rectangle(0, 0, width, height);
//		}
//		math::Rectangle GetSize(math::Bounds2D<float_t> s) const {
//			return math::Rectangle(width * s.x0, height * s.y0, width * s.x1, height * s.y1);
//		}
//
//
//
//		void Bind(math::Bounds2D<float_t> viewport = math::Bounds2D<float_t>::Unit(), FrameBufferTarget _targets = FrameBufferTarget::ALL) {
//
//			if (bindFramebuffer(this)) {
//				glViewport(width * viewport.x0, height * viewport.y0, width * viewport.x1, height * viewport.y1);
//			}
//			drawBuffers(_targets);
//		}
//
//
//
//	private:
//		static bool bindFramebuffer(const FrameBuffer* renderTexture) {
//			static uint32_t current = 0;
//			uint32_t New = renderTexture->fbo.GetID();
//			bool isNew = current != New;
//			if (isNew) {
//				current = New;
//				glBindFramebuffer(GL_FRAMEBUFFER, New);
//			}
//
//			return isNew;
//		}
//
//		void drawBuffers(FrameBufferTarget _targets)const {
//			if (fbo) {
//				std::bitset<32> bits = static_cast<uint32_t>(_targets);
//				bits &= emplacedIndices.to_ullong();
//				if(bits.any())
//				{
//					bool changed = bits == latestDrawBuffers;
//					latestDrawBuffers = bits;
//
//					if (changed) {
//						drawBufferList.clear();
//						for (size_t i = 0; i < 32; i++) {
//							if (latestDrawBuffers[i]) {
//								drawBufferList.emplace_back(GL_COLOR_ATTACHMENT0 + i);
//							}
//						}
//
//						glNamedFramebufferDrawBuffers(fbo.GetID(), drawBufferList.size(), drawBufferList.data());
//					}
//				}
//			}
//
//		}
//		void readBuffer(FrameBufferTarget _target)const {
//
//			uint32_t readBuffer = std::bit_width(static_cast<uint64_t>(_target));
//			if (readBuffer == latestReadBuffer) {
//				latestReadBuffer = readBuffer;
//				glNamedFramebufferReadBuffer(fbo.GetID(), GL_COLOR_ATTACHMENT0 + readBuffer-1);
//			}
//		}
//
//
//		void attachCubeMap(int32_t target, CubeMapFace face) {
//			if (fbo) {
//				auto& [first, second] = textures[indices[target]];
//				const TextureObject& base = *first;
//				glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int32_t>(face), base.GetID(), activeLod);
//			}
//		}
//		void attachTexture(int32_t target)const {
//			if (fbo) {
//				const auto& [first, second] = textures[indices[target]];
//				const TextureObject& base = *first;
//				glNamedFramebufferTexture(fbo.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod);
//			}
//		}
//
//		SharedTexture2& getTarget(FrameBufferTarget target) {
//			if (fbo) {
//				uint32_t log = std::bit_width(static_cast<uint64_t>(target)) - 1u;
//				if (!emplacedIndices[log]) {
//					emplacedIndices[log] = true;
//					indices[log] = static_cast<uint8_t>(textures.size());
//					textures.emplace_back(nullptr, nullptr);
//				}
//				return textures[indices[log]];
//			}
//			else {
//				throw std::exception("FrameBuffer is not initialized");
//			}
//		}
//		void eraseTarget(FrameBufferTarget target) {
//			if (fbo) {
//				uint32_t log = std::bit_width(static_cast<uint64_t>(target)) - 1u;
//				std::swap(textures[log], textures.back());
//				indices[log] = indices[textures.size() - 1];
//				textures.pop_back();
//			}
//		}
//
//	private:
//		std::string DEBUGFBOINFO() const {
//			GLuint framebufferID = fbo.GetID();
//			std::string info;
//			GLint maxColorAttachments;
//			glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
//
//			// Bind the framebuffer to query its properties
//			glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
//
//			for (int i = 0; i <= maxColorAttachments; ++i) {
//				GLenum attachment = (i == maxColorAttachments) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0 + i;
//				GLint objectType;
//				glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &objectType);
//
//				if (objectType == GL_TEXTURE) {
//					GLint textureID, textureFormat, textureWidth, textureHeight;
//					glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachment, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
//					glBindTexture(GL_TEXTURE_2D, textureID);
//					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &textureFormat);
//					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
//					glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
//					glBindTexture(GL_TEXTURE_2D, 0);
//
//					info += "Attachment " + std::to_string(i) + ": Texture " + std::to_string(textureID) +
//						", Format " + std::to_string(textureFormat) +
//						", Width " + std::to_string(textureWidth) +
//						", Height " + std::to_string(textureHeight) + "\n";
//				}
//				// Other cases such as GL_RENDERBUFFER can be handled similarly
//			}
//
//			// Unbind the framebuffer
//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//			return info;
//		}
//
//		std::string DEBUG_FRAMEBUFFER_STATE() const {
//			std::string stateInfo;
//			GLuint framebufferID = fbo.GetID();
//
//			glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
//			GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//			// ... [handle different cases as before]
//
//			if (framebufferStatus == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
//				GLint maxColorAttachments;
//				glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
//
//				for (int attachment = 0; attachment <= maxColorAttachments; ++attachment) {
//					GLenum attachmentEnum = (attachment == maxColorAttachments) ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0 + attachment;
//					GLint attachmentType;
//					glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachmentEnum, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &attachmentType);
//
//					// If something is attached, get more info
//					if (attachmentType != GL_NONE) {
//						GLint name, width, height, componentType;
//						glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachmentEnum, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
//						glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachmentEnum, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &width);
//						glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, attachmentEnum, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &height);
//						// ... and other queries you might be interested in
//
//						// Append to stateInfo about this attachment
//						stateInfo += "Attachment " + std::to_string(attachment) + ": Type=" + std::to_string(attachmentType) + ", Name=" + std::to_string(name) + "\n";
//						// ... include other details you've gathered
//					}
//					else {
//						stateInfo += "Attachment " + std::to_string(attachment) + " is missing.\n";
//					}
//				}
//			}
//
//			glBindFramebuffer(GL_FRAMEBUFFER, 0);
//			return stateInfo;
//		}
//
//	};
//
//
//
//
//
//}
//
//
//
