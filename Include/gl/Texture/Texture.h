/*
 * #pragma once
#include <glm/common.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "TextureEnum.h"
#include "gl/Enums/EnumWrapper.h"
#include "utility/openglError.h"
#include "Sampler.h"
#include "gl/GLObject.h"

namespace graphics
{

	struct TextureProperties
	{
		int32_t width = 1;
		int32_t height = 1;
		int32_t depth = 1;
		int32_t numMipMaps = 1;
		int32_t numSamples = 1;


		TextureTarget textureTarget = TextureTarget::Texture2D;
		TextureFormat textureFormat = TextureFormat::Auto;

		TextureProperties(TextureTarget _textureTarget, TextureFormat _textureFormat, int32_t _w, int32_t _h, int32_t _d, int32_t _mipMaps, int32_t _samples)
			:
			width(_w),
			height(_h),
			depth(_d),
			numMipMaps(_mipMaps),
			numSamples(_samples),
			textureTarget(_textureTarget),
			textureFormat(_textureFormat)
		{
			
		}

	};


	struct Texture : public utility::SharedObject
	{
	public:
		TextureObject object;
		TextureProperties settings;
		mutable uint64_t textureHandle;
		ManagedPointer<Sampler> sampler;




		Texture(const TextureProperties& _settings, ManagedPointer<Sampler> _sampler = nullptr)
			:settings(_settings),textureHandle(0), sampler(std::move(_sampler))
		{
			m_size = glm::max(m_size, glm::ivec3(1, 1, 1));

		}


		void assign(TextureObject&& textureObject)
		{
			
		}



		void Init() override
		{
			free();
			reallocate(static_cast<uint32_t>(m_size.x), static_cast<uint32_t>(m_size.y), static_cast<uint32_t>(m_size.z), static_cast<uint32_t>(m_mipMaps));
		}

		void reallocate(uint32_t width, uint32_t height, uint32_t depth, uint32_t numMipmaps)
		{
			if(ID == 0)
			{
				glCreateTextures(gl_enum_cast(m_target), 1, &ID);
			}

			m_size = glm::ivec3(width, height, depth);
			m_mipMaps = static_cast<int>(
				std::min(numMipmaps, std::bit_width(
					std::max(width, std::max(depth, height)))
				)
				);

			switch (m_target)
			{
			case TextureTarget::Texture1D:
				glTextureStorage1D(ID, m_mipMaps, gl_enum_cast(m_target), m_size.x);
			case TextureTarget::Texture2D:
			case TextureTarget::TextureRectangle:
			case TextureTarget::Texture1DArray:
			case TextureTarget::TextureCubeMap:
				glTextureStorage2D(ID, m_mipMaps, gl_enum_cast(m_target), m_size.x, m_size.y);
				break;
			case TextureTarget::Texture3D:
			case TextureTarget::Texture2DArray:
			case TextureTarget::TextureCubeMapArray:
				glTextureStorage3D(ID, m_mipMaps, gl_enum_cast(m_target), m_size.x, m_size.y, m_size.z);
				break;
			case TextureTarget::Texture2DMultisample:
				glTextureStorage2DMultisample(ID, m_numSamples, gl_enum_cast(m_target), m_size.x, m_size.y, true);
				break;
			case TextureTarget::Texture2DMultisampleArray:
				glTextureStorage3DMultisample(ID, m_numSamples, gl_enum_cast(m_target), m_size.x, m_size.y, m_size.z, true);

			case TextureTarget::TextureBuffer:
				break;
			default:
				break;
			}
		}

		void makeBindLess()const
		{
			if(m_textureHandle == 0)
			{
				m_textureHandle = m_sampler ? glGetTextureSamplerHandleARB(ID, m_sampler->samplerID) : glGetTextureHandleARB(ID);
				glMakeTextureHandleResidentARB(m_textureHandle);
			}
		}
		glm::uvec2& getTextureHandle()const
		{
			return reinterpret_cast<glm::uvec2&>(m_textureHandle);
		}

		gl::TextureTarget getTarget()const { return m_target; }

		void Bind()const
		{
			glBindTexture(gl_enum_cast(m_target), ID);
		}

		~Texture()override
		{
			free();
		}
		GLuint GetID()const
		{
			return ID;
		}
		const ManagedPointer<Sampler>& getSampler()const
		{
			return m_sampler;
		}
		void free() {
			if (ID != 0) {
				glDeleteTextures(1, &ID);
				ID = 0;
			}
		}
	};
	


	class Texture2D : public Texture
	{
	public:
		Texture2D(math::Number<int32_t> w, math::Number<int32_t> h, TextureFormat textureFormat, ManagedPointer<Sampler> sampler, math::Number<int32_t> mipmaps = 1) :
			Texture(TextureTarget::Texture2D, w, h, 1, textureFormat, mipmaps, 1, std::move(sampler))
		{
		}
		void TextureData(
			utility::span<void> data,
			ExternalTextureFormat dataFormat,
			DataType dataType,
			math::Number<int32_t> x = 0,
			math::Number<int32_t> y = 0,
			math::Number<int32_t> width = std::numeric_limits<int32_t>::max(),
			math::Number<int32_t> height = std::numeric_limits<int32_t>::max()
		)const
		{
			width = std::min<int32_t>(width, m_size.x - *x);
			height = std::min<int32_t>(height, m_size.y - *y);

			glTextureSubImage2D(ID ,0, x, y, width, height, gl_enum_cast(dataFormat), gl_enum_cast(dataType), data.begin_ptr);
		}
	};
	class Texture2DArray : public Texture
	{
	public:
		Texture2DArray(math::Number<int32_t> w, math::Number<int32_t> h, math::Number<int32_t> numLayers, TextureFormat textureFormat, ManagedPointer<Sampler> sampler, math::Number<int32_t> mipmaps = 1) :
			Texture(TextureTarget::Texture2DArray, w, h, numLayers, textureFormat, mipmaps, 1, std::move(sampler))
		{

		}
		void TextureData(
			utility::span<void> data,
			ExternalTextureFormat dataFormat,
			DataType dataType,
			math::Number<int32_t> layer = 0,
			math::Number<int32_t> numLayers = 1,
			math::Number<int32_t> x = 0,
			math::Number<int32_t> y = 0,
			math::Number<int32_t> width = std::numeric_limits<int32_t>::max(),
			math::Number<int32_t> height = std::numeric_limits<int32_t>::max())
		{
			width = std::min<int32_t>(width, m_size.x - *x);
			height = std::min<int32_t>(height, m_size.y - *y);
			numLayers = std::min<int32_t>(numLayers, m_size.z - *layer);

			glTextureSubImage3D(ID, 0, x, y, layer, width, height, numLayers, gl_enum_cast(dataFormat), gl_enum_cast(dataType), data.begin_ptr);
		}
	};

	class CubeMap : public Texture
	{
	public:
		

		CubeMap(math::Number<int32_t> w, math::Number<int32_t> h, TextureFormat textureFormat, ManagedPointer<Sampler> sampler, math::Number<int32_t> mipmaps = 1) :
			Texture(TextureTarget::TextureCubeMap, w, h, 1, textureFormat, mipmaps, 1, std::move(sampler))
		{

		}
		void TextureData(
			utility::span<void> data,
			ExternalTextureFormat dataFormat,
			DataType dataType,
			Face face,
			math::Number<int32_t> x = 0,
			math::Number<int32_t> y = 0,
			math::Number<int32_t> width = std::numeric_limits<int32_t>::max(),
			math::Number<int32_t> height = std::numeric_limits<int32_t>::max()
		)const
		{
			width = std::min<int32_t>(width, m_size.x - *x);
			height = std::min<int32_t>(height, m_size.y - *y);
			Bind();
			glTextureSubImage2D(face, 0, x, y, width, height, gl_enum_cast(dataFormat), gl_enum_cast(dataType), data.begin_ptr);
		}
	};
	class RenderTexture : public Texture
	{
	public:
		RenderTexture(math::Number<int32_t> w, math::Number<int32_t> h = 1, math::Number<int32_t> d = 1, math::Number<int32_t> mipmaps = 1, math::Number<int32_t> numSamples = 0) :
			Texture(
				( *h <= 1 ?
				TextureTarget::Texture1D : *h <= 1 ?
				*numSamples ? TextureTarget::Texture2DMultisample : TextureTarget::Texture2D
				: TextureTarget::Texture3D),
				w,
				h,
				d,
				TextureFormat::Auto,
				mipmaps,
				1,
				nullptr
			)
		{
		}
		void Init(TextureFormat _textureFormat)
		{
			m_textureFormat = _textureFormat;
			Texture::Init();
		}
		




	};
}

 */