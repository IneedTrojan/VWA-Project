#pragma once
#include <bitset>
#include <glad/glad.h>
#include <cinttypes>
#include <memory>
#include <unordered_set>

#include "Enum/SamplerEnum.h"
#include "Enum/TextureTarget.h"
#include "Enum/TextureFormat.h"
#include "math/BoundingBox.h"
#include "utility/Disposable.h"
#include "utility/Span.h"
#include "utility/Pooling/Pool.h"
#include "components/Entity.h"

namespace graphics
{

	

	struct TexParam {
		DepthStencilMode depthStencilMode = DepthStencilMode::LUMINANCE;
		SRGBDecodeExt sRGBDecodeExt = SRGBDecodeExt::SKIP_DECODE_EXT;
		GLfloat maxAnisotropy = 1.0f;
		GLint baseLevel = 0;
		GLint maxLevel = 1000;

		SwizzleChannel swizzleR = SwizzleChannel::RED;
		SwizzleChannel swizzleG = SwizzleChannel::GREEN;
		SwizzleChannel swizzleB = SwizzleChannel::BLUE;
		SwizzleChannel swizzleA = SwizzleChannel::ALPHA;


	protected:
		TexParam() {
		}
		void Sync(GLuint ID) {

			std::array defaultSwizzle = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
			std::array params = {static_cast<int32_t>(swizzleR), static_cast<int32_t>(swizzleG) , static_cast<int32_t>(swizzleB) , static_cast<int32_t>(swizzleA)};
			if (memcmp(defaultSwizzle.data(), params.data(), params.size())) {
				glTextureParameteriv(ID, GL_TEXTURE_SWIZZLE_RGBA, params.data());
			}

			if (depthStencilMode != DepthStencilMode::LUMINANCE) {
				glTextureParameteri(ID, GL_DEPTH_STENCIL_TEXTURE_MODE, static_cast<GLenum>(depthStencilMode));
			}

			if (sRGBDecodeExt != SRGBDecodeExt::SKIP_DECODE_EXT) {
				glTextureParameteri(ID, GL_TEXTURE_SRGB_DECODE_EXT, static_cast<GLenum>(sRGBDecodeExt));
			}

			if (maxAnisotropy != 1.0f) {
				glTextureParameterf(ID, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
			}

			if (baseLevel != 0) {
				glTextureParameteri(ID, GL_TEXTURE_BASE_LEVEL, baseLevel);
			}

			if (maxLevel != 1000) {
				glTextureParameteri(ID, GL_TEXTURE_MAX_LEVEL, maxLevel);
			}
		}
	};


	struct Sampler {

		WrapMode wrapS;               // GL_TEXTURE_WRAP_S
		WrapMode wrapT;               // GL_TEXTURE_WRAP_T
		WrapMode wrapR;               // GL_TEXTURE_WRAP_R
		MinFilter minFilter;           // GL_TEXTURE_MIN_FILTER
		MagFilter magFilter;           // GL_TEXTURE_MAG_FILTER
		CompareMode compareMode;       // GL_TEXTURE_COMPARE_MODE
		CompareFunc compareFunc;       // GL_TEXTURE_COMPARE_FUNC
		
		SeamlessCubemap cubeMapSeamless;      // GL_TEXTURE_CUBE_MAP_SEAMLESS
		GLfloat minLod;             // GL_TEXTURE_MIN_LOD
		GLfloat maxLod;             // GL_TEXTURE_MAX_LOD
		GLfloat lodBias;
		GLfloat maxAnisotropyF;     // GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
		GLfloat borderColor[4];     // GL_TEXTURE_BORDER_COLOR (RGBA)
		mutable GLuint ID = 0;


		Sampler() {
			wrapS = WrapMode::REPEAT;
			wrapT = WrapMode::REPEAT;
			wrapR = WrapMode::REPEAT;
			minFilter = MinFilter::LINEAR;
			magFilter = MagFilter::LINEAR;
			compareMode = CompareMode::NONE;
			compareFunc = CompareFunc::LEQUAL;
			cubeMapSeamless = SeamlessCubemap::DISABLED;
			
			lodBias = 0.0f;
			minLod = -1000.0f;
			maxLod = 1000.0f;
			maxAnisotropyF = 1.0f;
			borderColor[0] = 0.0f;
			borderColor[1] = 0.0f;
			borderColor[2] = 0.0f;
			borderColor[3] = 0.0f;
		}

		static void Sync(const Sampler& self)
		{
			glCreateSamplers(1, &self.ID);
			glSamplerParameteri(self.ID, GL_TEXTURE_WRAP_S, static_cast<GLenum>(self.wrapS));
			glSamplerParameteri(self.ID, GL_TEXTURE_WRAP_T, static_cast<GLenum>(self.wrapT));
			glSamplerParameteri(self.ID, GL_TEXTURE_WRAP_R, static_cast<GLenum>(self.wrapR));

			glSamplerParameteri(self.ID, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(self.minFilter));
			glSamplerParameteri(self.ID, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(self.magFilter));

			glSamplerParameteri(self.ID, GL_TEXTURE_COMPARE_MODE, static_cast<GLenum>(self.compareMode));
			glSamplerParameteri(self.ID, GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(self.compareFunc));

			glSamplerParameteri(self.ID, GL_TEXTURE_CUBE_MAP_SEAMLESS, static_cast<GLenum>(self.cubeMapSeamless));

			glSamplerParameterf(self.ID, GL_TEXTURE_LOD_BIAS, self.lodBias);
			glSamplerParameterf(self.ID, GL_TEXTURE_MIN_LOD, self.minLod);
			glSamplerParameterf(self.ID, GL_TEXTURE_MAX_LOD, self.maxLod);
		}
		
		bool operator ==(const Sampler & other) const
		{
			return memcmp(this, &other, offsetof(Sampler, ID));
		}


		~Sampler() {
			if (ID) {

				glDeleteSamplers(1, &ID);
				ID = 0;
			}
		}
	};


	struct SamplerHash {
		std::size_t operator()(const Sampler& sampler) const {
			std::size_t seed = 0;

			seed ^= static_cast<std::size_t>(sampler.wrapS) +
				static_cast<std::size_t>(sampler.wrapT) +
				static_cast<std::size_t>(sampler.wrapR) +
				static_cast<std::size_t>(sampler.minFilter) +
				static_cast<std::size_t>(sampler.magFilter) +
				static_cast<std::size_t>(sampler.compareMode) +
				static_cast<std::size_t>(sampler.compareFunc) +
				static_cast<std::size_t>(sampler.cubeMapSeamless);

			seed ^= std::hash<float>{}(sampler.lodBias) +
				std::hash<float>{}(sampler.minLod) +
				std::hash<float>{}(sampler.maxLod) +
				std::hash<float>{}(sampler.maxAnisotropyF);

			return seed;
		}
	};


	class SamplerRegistry
	{
		std::unordered_set<Sampler, SamplerHash> samplers;

	public:

		static const Sampler* GetSampler(const Sampler& key) {
			auto it = instance().samplers.find(key);
			if (it == instance().samplers.end()) {
				it = instance().samplers.emplace(key).first;
				it->Sync(*it);
			}
			return &*it;
		}
	private:
		static SamplerRegistry& instance() {
			static SamplerRegistry inst;
			return inst;
		}
	};
	
}

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

		TextureProperties() = default;

		TextureProperties(TextureTarget _textureTarget, TextureFormat _textureFormat, glm::ivec2 wh, int32_t _mipMaps = 1, int32_t _samples = 1)
			:
			width(wh.x),
			height(wh.y),
			depth(1),
			numMipMaps(_mipMaps),
			numSamples(_samples),
			textureTarget(_textureTarget),
			textureFormat(_textureFormat)
		{

		}
		TextureProperties(TextureTarget _textureTarget, TextureFormat _textureFormat, glm::ivec3 whd, int32_t _mipMaps = 1, int32_t _samples = 1)
			:
			width(whd.x),
			height(whd.y),
			depth(whd.z),
			numMipMaps(_mipMaps),
			numSamples(_samples),
			textureTarget(_textureTarget),
			textureFormat(_textureFormat)
		{

		}
	};



	struct TextureObject
	{
	private:
		GLuint m_ID = 0;
		mutable int32_t TextureUnit = 0;
		uint64_t handle = 0;

	public:
		GLuint GetID()const
		{
			return m_ID;
		}
		int32_t GetTextureUnit()const
		{
			return TextureUnit;
		}
		void SetTextureUnit(int32_t unit)const
		{
			TextureUnit = unit;
		}
		TextureObject() = default;

		static std::unique_ptr<TextureObject> Create(TextureTarget textureTarget)
		{
			TextureObject texture;
			glCreateTextures(gl_enum_cast(textureTarget), 1, &texture.m_ID);
			return std::make_unique<TextureObject>(std::move(texture));
		}
		static TextureObject CreateStack(TextureTarget textureTarget)
		{
			TextureObject texture;
			glCreateTextures(gl_enum_cast(textureTarget), 1, &texture.m_ID);
			return texture;
		}

		TextureObject(TextureObject&& other)noexcept
			:m_ID(std::exchange(other.m_ID, 0)),
			handle(std::exchange(other.m_ID, 0))
		{

		}
		TextureObject& operator=(TextureObject&& other)noexcept
		{
			if (&other != this)
			{
				free();
				m_ID = std::exchange(other.m_ID, 0);
				handle = std::exchange(other.handle, 0);
			}
			return *this;
		}

		TextureObject(const TextureObject& other) = delete;
		TextureObject& operator =(const TextureObject& other) = delete;

		~TextureObject()
		{
			free();
		}


		void free()
		{
			if (handle) {
				glMakeTextureHandleNonResidentARB(handle);
			}
			if (m_ID) {
				glDeleteTextures(1, &m_ID);
				m_ID = 0;
			}
		}

		void textureStorage1D(int32_t _w, TextureFormat textureFormat, int32_t mipMaps = 1)const
		{
			assert(mipMaps > 0);
			glTextureStorage1D(
				m_ID,
				std::min(mipMaps, maxMipMaps(_w)),
				gl_enum_cast(textureFormat),
				_w
			);
		}
		void cubeMapSubImage2D(
			CubeMapFace face,
			utility::const_span<void> pixels,
			int32_t _x, int32_t _y,
			int32_t _w, int32_t _h,
			ExternalTextureFormat dataformat, DataType dataType, int32_t mipMap = 1)const
		{
			Bind(TextureTarget::TextureCubeMap);
			glTexSubImage2D(
				gl_enum_cast(face),
				mipMap,
				_x, _y, _w, _h,
				gl_enum_cast(dataformat),
				gl_enum_cast(dataType),
				pixels.begin()
			);
		}
		void textureStorage2D(int32_t _w, int32_t _h, TextureFormat textureFormat, int32_t mipMaps = 1)const
		{
			int mipmaps = std::min(mipMaps, maxMipMaps(_w, _h));
			assert(mipMaps > 0);
			glTextureStorage2D(
				m_ID,
				mipmaps,
				gl_enum_cast(textureFormat),
				_w,
				_h
			);
		}
		void textureStorage3D(int32_t _w, int32_t _h, int32_t _d, TextureFormat textureFormat, int32_t mipMaps = 1)const
		{
			assert(mipMaps > 0);
			glTextureStorage3D(
				m_ID,
				std::min(mipMaps, maxMipMaps(_w, _h, _d)),
				gl_enum_cast(textureFormat),
				_w,
				_h,
				_d
			);
		}
		void textureStorage2DMultiSample(int32_t _w, int32_t _h, TextureFormat textureFormat, int32_t numSamples, bool fixedSampleLocations = true)const
		{
			glTextureStorage2DMultisample(
				m_ID,
				numSamples,
				gl_enum_cast(textureFormat),
				_w,
				_h,
				fixedSampleLocations
			);
		}
		void textureStorage3DMultiSample(int32_t w, int32_t h, int32_t d, TextureFormat textureFormat, int32_t numSamples, bool fixedSampleLocations = true)const
		{
			glTextureStorage3DMultisample(
				m_ID,
				numSamples,
				gl_enum_cast(textureFormat),
				w,
				h,
				d,
				fixedSampleLocations
			);
		}
		void textureSubImage1D(utility::const_span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, int32_t x, int32_t w, int32_t level = 0)const
		{
			glTextureSubImage1D(m_ID, level, x, w, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
		}
		void textureSubImage2D(utility::const_span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, int32_t x, int32_t y, int32_t w, int32_t h, int32_t level = 0)const
		{
			glTextureSubImage2D(m_ID, level, x, y, w, h, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
		}


		void getTextureSubImage2D(utility::span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, int32_t x, int32_t y, int32_t w, int32_t h, int32_t level = 0) const
		{
			glGetTextureSubImage(m_ID, level, x, y, 0, w, h, 1, gl_enum_cast(dataFormat), gl_enum_cast(dataType), static_cast<int32_t>(pixels.byteSize()), pixels.begin_ptr);
		}

		void textureSubImage3D(utility::const_span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
		{
			glTextureSubImage3D(m_ID, level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), pixels.begin_ptr);
		}
		
		void getTextureSubImage3D(utility::span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
		{
			glGetTextureSubImage(m_ID, level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), static_cast<int32_t>(pixels.byteSize()), pixels.begin_ptr);
		}
		void getTextureSubImage3DForCubeMap(utility::span<void> pixels, ExternalTextureFormat dataFormat, DataType dataType, CubeMapFace face, int32_t x, int32_t y, int32_t z, int32_t w, int32_t h, int32_t d, int32_t level = 0)const
		{
			Bind(TextureTarget::TextureCubeMap);
			glGetTextureSubImage(gl_enum_cast(face), level, x, y, z, w, h, d, gl_enum_cast(dataFormat), gl_enum_cast(dataType), static_cast<int32_t>(pixels.byteSize()), pixels.begin_ptr);
		}

		void BindImageTexture(TextureFormat format, int32_t ImageUnit, WriteAccess writeAccess) const {
			glBindImageTexture(ImageUnit, m_ID, 0, GL_FALSE, 0, static_cast<int32_t>(writeAccess), gl_enum_cast(format));
		}
		void Bind(TextureTarget target)const
		{
			glBindTexture(gl_enum_cast(target), m_ID);
		}
		static void ActiveUnit(int32_t unit)
		{
			glActiveTexture(GL_TEXTURE0 + unit);
		}

		const uint64_t& getNVIDIAHandle()const
		{
			assert(handle != 0);
			return handle;
		}
		void GenerateMipmaps() {
			glGenerateTextureMipmap(m_ID);
		}
	private:
		static int32_t maxMipMaps(int32_t width)
		{
			return static_cast<int32_t>(std::bit_width(static_cast<uint32_t>(width)));
		}
		static int32_t maxMipMaps(int32_t width, int32_t height)
		{
			return maxMipMaps(std::max(width, height));
		}
		static int32_t maxMipMaps(int32_t width, int32_t height, int32_t depth)
		{
			return maxMipMaps(std::max(width, std::max(depth, height)));
		}
	};


	class Texture :private TextureProperties, utility::DisposableSink, public TexParam
	{
		friend class FrameBufferObject;
		TextureObject object;
		const Sampler* sampler;
		std::atomic_bool is_allocated = false;

	public:

		Texture(TextureTarget target, TextureFormat textureFormat)
		{
			this->textureTarget = target;
			this->textureFormat = textureFormat;
			sampler = nullptr;
			object = TextureObject::CreateStack(textureTarget);
		}
		void TextureParams(glm::ivec3 whd, int32_t mipmaps = 1)
		{
			TextureParams(whd.x, whd.y, whd.z, mipmaps);
		}
		
		void TextureParams(int32_t w, int32_t h, int32_t d = 1, int32_t mipmaps = 1)
		{
			width = w;
			height = h;
			depth = d;
			numMipMaps = mipmaps;
		}
		void Allocate(const Sampler& sampler)
		{
			this->sampler = SamplerRegistry::GetSampler(sampler);
			allocate();
			Sync(object.GetID());
			is_allocated.store(true);
		}
		void Resize(int32_t w, int32_t h = 1, int32_t d = 1)
		{
			if (w != width || h != height || d != depth)
			{
				width = w;
				height = h;
				depth = d;
				allocate();
			}
		}

		static void GetTexture2DSubImage(const Texture& texture, utility::span<void> write, ExternalTextureFormat dataFormat, DataType dataType,  math::Rectangle area = math::Rectangle::Maximum(), int32_t lod = 0)
		{
			area = math::Rectangle::Intersection(area, { 0,0,texture.width, texture.height });
			texture.object.getTextureSubImage2D(write, dataFormat, dataType, area.x0, area.y0, area.x1, area.y1, lod);
		}
		static void GetTexture3DSubImage(Texture& texture, utility::span<void> write, ExternalTextureFormat dataFormat, DataType dataType, int32_t layer0 = 0, int32_t numLayers = std::numeric_limits<int32_t>::max(), math::Rectangle area = math::Rectangle::Maximum(), int32_t lod = 0)
		{
			numLayers = glm::min(texture.depth - layer0, numLayers);
			area = math::Rectangle::Intersection(area, { 0,0,texture.width, texture.height });
			texture.object.getTextureSubImage3D(write, dataFormat, dataType, area.x0, area.y0, layer0, area.x1, area.y1, layer0 + numLayers, lod);
		}

		static void TextureSubImage3D(Texture& texture, utility::const_span<void>data, ExternalTextureFormat dataFormat, DataType pixelType, int32_t layer = 0, int32_t numLayers = std::numeric_limits<int32_t>::max(), math::Rectangle area = math::Rectangle::Maximum(), int32_t level = 0)
		{
			numLayers = std::min(numLayers, texture.depth - layer);
			area = math::Rectangle::Intersection(area, { 0,0,texture.width, texture.height });
			texture.object.textureSubImage3D(data, dataFormat, pixelType, area.x0, area.y0, layer, area.width(), area.height(), numLayers, level);
		}

		static void TextureSubImage2D(Texture& texture, utility::const_span<void>data, ExternalTextureFormat dataFormat, DataType pixelType, math::Rectangle area = math::Rectangle::Maximum(), int32_t level = 0)
		{
			area = math::Rectangle::Intersection(area, { 0,0,texture.width, texture.height });
			texture.object.textureSubImage2D(data, dataFormat, pixelType, area.x0, area.y0, area.width(), area.height(), level);
		}

		static void CubeMapSubImage2D(Texture& texture, CubeMapFace face, utility::const_span<void>data, ExternalTextureFormat dataFormat, DataType pixelType, math::Rectangle area = math::Rectangle::Maximum(), int32_t level = 0)
		{
			area = math::Rectangle::Intersection(area, { 0,0,texture.width, texture.height });
			texture.object.cubeMapSubImage2D(face, data, area.x0, area.y0, area.width(), area.height(), dataFormat, pixelType, level);
		}
		void Bind()const
		{
			object.Bind(textureTarget);
		}
		void Bind(int32_t unit)const
		{
			glBindSampler(unit, sampler ? sampler->ID : 0);
			glBindTextureUnit(unit, object.GetID());
		}
		void GenerateMipmaps() {
			object.GenerateMipmaps();
		}

		TextureTarget GetTextureTarget()const {
			return textureTarget;
		}
		TextureFormat GetTextureFormat()const {
			return textureFormat;
		}
		const Sampler* GetSampler()const {
			return sampler;
		}


		const TextureObject& GetBase()const
		{
			return object;
		}

		void BindToImageUnit(int32_t unit, WriteAccess access, int32_t level = 0, int32_t layer = 0)const
		{
			glBindImageTexture(unit, object.GetID(), level, IsTargetLayered(textureTarget), layer, static_cast<int32_t>(access), gl_enum_cast(GetTextureFormat()));
		}

		int32_t getWidth()const
		{
			return width;
		}
		int32_t getHeight()const
		{
			return height;
		}
		glm::ivec2 getSize()const
		{
			return { width, height };
		}
		glm::ivec2 getDepth()const
		{
			return { depth, 1 };
		}
		bool IsAllocated()const
		{
			return is_allocated.load();
		}
		operator const TextureObject& ()const
		{
			return object;
		}
		operator const TextureObject* ()const
		{
			return &object;
		}




	private:
		void allocate()const
		{
			switch (textureTarget)
			{
			case TextureTarget::Texture1D:
				object.textureStorage1D(width, textureFormat, numMipMaps);
				break;
			case TextureTarget::Texture1DArray:
			case TextureTarget::Texture2D:
			case TextureTarget::TextureCubeMap:
				object.textureStorage2D(width, height, textureFormat, numMipMaps);
				break;
			case TextureTarget::Texture3D:
			case TextureTarget::TextureCubeMapArray:
			case TextureTarget::Texture2DArray:
				object.textureStorage3D(width, height, depth, textureFormat, numMipMaps);
				break;
			case TextureTarget::Texture2DMultisample:
				object.textureStorage2DMultiSample(width, height, textureFormat, numSamples);
				break;
			case TextureTarget::Texture2DMultisampleArray:
				object.textureStorage3DMultiSample(width, height, depth, textureFormat, numSamples);
			default:
				break;
			}
		}


		static constexpr bool IsTargetLayered(TextureTarget target)
		{
			switch (target)
			{
			case TextureTarget::Texture1D:
			case TextureTarget::Texture2D:
			case TextureTarget::Texture3D:
			case TextureTarget::TextureRectangle:
			case TextureTarget::TextureCubeMap:
			case TextureTarget::TextureBuffer:
			case TextureTarget::Texture2DMultisample:
				return false;
			case TextureTarget::Texture1DArray:
			case TextureTarget::Texture2DArray:
			case TextureTarget::TextureCubeMapArray:
			case TextureTarget::Texture2DMultisampleArray:
				return true;
			default:
				return false;
			}
		}
	};


	struct TextureBatch
	{
		std::vector<uint32_t> textures;
		std::vector<uint32_t> samplers;
		bool canStore(size_t size, size_t maxSize)const
		{
			return textures.size() + size != maxSize;
		}
		size_t emplace(uint32_t texture, uint32_t sampler)
		{
			textures.push_back(texture);
			samplers.push_back(sampler);
			return samplers.size() - 1;
		}

		void Flush()const
		{
			if (!textures.empty())
			{
				glBindTextures(0, static_cast<int>(textures.size()), textures.data());
				glBindSamplers(0, static_cast<int>(samplers.size()), samplers.data());
			}
		}
		void Clear()
		{
			textures.clear();
			samplers.clear();
		}
	};
	struct TextureBatcher
	{

		std::vector<TextureBatch> batches;
		size_t maxSize;

		struct TextureBindParameters
		{
			int32_t TextureID;
			int32_t SamplerID;
			int32_t outUnit;
		};
		static TextureBatch& BindTextures(utility::span<TextureBindParameters> params)
		{
			auto& instance = TextureBatcher::instance();

			size_t count = params.size();
			if (instance.batches.empty() || !instance.batches.back().canStore(count, instance.maxSize)) {
				instance.batches.emplace_back();
			}
			for (auto& param : params) {
				param.outUnit = instance.batches.back().emplace(param.TextureID, param.SamplerID);
			}
			return instance.batches.back();
		}
		static bool Flush()
		{
			auto& instance = TextureBatcher::instance();
			const bool _continue = !instance.batches.empty();
			if (_continue)
			{
				instance.batches.front().Flush();
				instance.batches.pop_back();
			}
			return _continue;
		}
		static void Reset()
		{
			auto& instance = TextureBatcher::instance();
			instance.batches.clear();
		}

		static TextureBatcher& instance()
		{
			static TextureBatcher _instance;
			return _instance;
		}
	};
}

