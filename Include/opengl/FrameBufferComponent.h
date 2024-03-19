#pragma once
#include <bitset>
#include <cstdint>
#include <iostream>
#include <glad/glad.h>

#include "TextureComponent.h"
#include "components/IComponent.h"
#include "math/BoundingBox.h"
#include "utility/Disposable.h"

namespace scene
{
	class Window;
}

namespace graphics
{

	enum class FrameBufferTarget : uint64_t
	{
		Albedo = 1ULL,
		ScreenNormal = 1ULL << 1u,
		WorldNormal = 1ULL << 2u,
		ScreenPosition = 1ULL << 3u,
		WorldPosition = 1ULL << 4u,
		RoughnessMetallicSpecular = 1ULL << 5u,

		Depth = 1ULL << 32u,
		Stencil = 1ULL << 33u,
		USE_COLOR = std::numeric_limits<uint32_t>::max(),
		NONE = 0,
		ALL = std::numeric_limits<uint64_t>::max()

	};
	ENABLE_ENUM_MATH(FrameBufferTarget);


	class FrameBufferObjectBase
	{
		GLuint ID = 0;
	public:

		FrameBufferObjectBase() = default;
		static FrameBufferObjectBase& Main()
		{
			static FrameBufferObjectBase instance;
			return instance;
		}

		


		static std::unique_ptr<FrameBufferObjectBase> Create()
		{
			FrameBufferObjectBase frameBuffer;
			glCreateFramebuffers( 1, &frameBuffer.ID);

			return std::make_unique<FrameBufferObjectBase>(std::move(frameBuffer));
		}
		static FrameBufferObjectBase CreateStack()
		{
			FrameBufferObjectBase frameBuffer;
			
			glCreateFramebuffers( 1, &frameBuffer.ID);
			return frameBuffer;
		}

		FrameBufferObjectBase(FrameBufferObjectBase&& other)noexcept
			:ID(std::exchange(other.ID, 0))
		{

		}
		FrameBufferObjectBase& operator=(FrameBufferObjectBase&& other)noexcept
		{
			if (&other != this)
			{
				free();
				ID = std::exchange(other.ID, 0);
			}
			return *this;
		}
		FrameBufferObjectBase(const FrameBufferObjectBase& other) = delete;
		FrameBufferObjectBase& operator =(const FrameBufferObjectBase& other) = delete;

		~FrameBufferObjectBase()
		{
			free();
		}

		void free()
		{
			if (ID != 0)
			{
				glDeleteFramebuffers( 1, &ID);
				ID = 0;
			}
		}
		
		uint32_t GetID()const
		{
			return ID;
		}
		operator bool()const {
			return ID != 0;
		}
	public:


		void DEBUG_LOG_TEXTURE_ERROR()const
		{
			if (glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Color attachment incomplete!" << std::endl;
				// Additional debugging or return
			}
		}
		void DEBUG_LOG_FRAME_BUFFER_COMPLETE() const {
			GLenum fbStatus = glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER);
			GLenum drawBuffersStatus = glCheckNamedFramebufferStatus(ID, GL_DRAW_FRAMEBUFFER);

			if (fbStatus != GL_FRAMEBUFFER_COMPLETE || drawBuffersStatus != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "Framebuffer is not complete! Framebuffer Status: " << fbStatus
					<< " DrawBuffers Status: " << drawBuffersStatus << std::endl;

				// Additional error details based on status
				if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
					switch (fbStatus) {
					case GL_FRAMEBUFFER_UNDEFINED:
						std::cerr << "GL_FRAMEBUFFER_UNDEFINED: The specified framebuffer is the default read or draw framebuffer, but the default framebuffer does not exist.\n";
						break;
					case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
						std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: One or more of the framebuffer attachment points are framebuffer incomplete.\n";
						break;
					case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
						std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: The framebuffer does not have at least one image attached to it.\n";
						break;
						// Add cases for other possible statuses...
					default:
						std::cerr << "Unknown framebuffer status.\n";
					}
				}

				// Additional debugging or return
			}
		}

		void DEBUG_LOG_ATTACHMENTS() const {
			GLint maxAttachments;
			glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments); // Get the maximum number of color attachments

			for (int attachmentPoint = 0; attachmentPoint < maxAttachments; ++attachmentPoint) {
				GLenum attachment;
				glGetNamedFramebufferAttachmentParameteriv(ID, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, reinterpret_cast<GLint*>(&attachment));

				if (attachment == GL_NONE) {
					// No attachment at this attachment point
					std::cerr << "Color Attachment Point " << attachmentPoint << ": No attachment\n";
				}
				else if (attachment == GL_RENDERBUFFER) {
					// Renderbuffer attachment
					GLint renderbufferID;
					glGetNamedFramebufferAttachmentParameteriv(ID, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &renderbufferID);
					std::cerr << "Color Attachment Point " << attachmentPoint << ": Renderbuffer ID " << renderbufferID << "\n";
				}
				else if (attachment == GL_TEXTURE) {
					// Texture attachment
					GLint textureID;
					glGetNamedFramebufferAttachmentParameteriv(ID, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
					std::cerr << "Color Attachment Point " << attachmentPoint << ": Texture ID " << textureID << "\n";
				}
			}

			// Log depth attachment
			GLenum depthAttachment;
			glGetNamedFramebufferAttachmentParameteriv(ID, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, reinterpret_cast<GLint*>(&depthAttachment));

			if (depthAttachment == GL_NONE) {
				// No depth attachment
				std::cerr << "Depth Attachment: No attachment\n";
			}
			else if (depthAttachment == GL_RENDERBUFFER) {
				// Renderbuffer depth attachment
				GLint renderbufferID;
				glGetNamedFramebufferAttachmentParameteriv(ID, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &renderbufferID);
				std::cerr << "Depth Attachment: Renderbuffer ID " << renderbufferID << "\n";
			}
			else if (depthAttachment == GL_TEXTURE) {
				// Texture depth attachment
				GLint textureID;
				glGetNamedFramebufferAttachmentParameteriv(ID, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &textureID);
				std::cerr << "Depth Attachment: Texture ID " << textureID << "\n";
			}
		}




	};

	

	class FrameBufferObject :public utility::DisposableSink
	{
		friend class scene::Window;
		std::unordered_map<FrameBufferTarget, std::shared_ptr<Texture>> attachments;

		FrameBufferObjectBase object;
		mutable std::bitset<64> attachedTargets = static_cast<uint64_t>(FrameBufferTarget::Albedo | FrameBufferTarget::Depth);
		int32_t activeLod = 0;
		int32_t numMipMaps = 1;

		mutable std::vector<uint32_t> drawBufferList;
		mutable std::bitset<32> latestDrawBuffers = false;
		mutable uint32_t latestReadBuffer = false;

		int32_t w, h, d;


		std::string name;

	public:
		FrameBufferObject() :activeLod(0), numMipMaps(1), w(1), h(1), d(1), name("Default")
		{

		}
		

		void initialize(std::string _name, int32_t _w, int32_t _h, int32_t _d = 1, int32_t mipMaps = 1, FrameBufferObjectBase&& base = FrameBufferObjectBase::CreateStack())
		{
			object = std::move(base);
			w = _w;
			h = _h;
			d = _d;
			numMipMaps = mipMaps;
			attachedTargets = 0ULL;
			name = std::move(_name);
			bindGlobal(this);

		}


		void EmplaceTexture(FrameBufferTarget targetBit, TextureFormat _textureFormat, TextureTarget _textureTarget = TextureTarget::Texture2D, const Sampler& sampler = Sampler(), int32_t numLayers = 0, int32_t _samples = 1)
		{
			std::shared_ptr<Texture> tex;
			if(object)
			{
				auto size = glm::ivec3(w, h, numLayers > 0 ? numLayers : d);
				tex = std::make_shared<Texture>(_textureTarget, _textureFormat);
				tex->TextureParams(size, numMipMaps);
				tex->Allocate(sampler);
			}
			AttachTexture(targetBit, tex);
			
		}
		const Texture& operator[](FrameBufferTarget target)
		{
			return *attachments[target];
		}


		void AttachTexture(FrameBufferTarget targetBit, const std::shared_ptr<Texture> entity)
		{
			auto it = attachments.find(targetBit);

			if (it != attachments.end())
			{
				throw std::runtime_error("TargetAlreadyAttached");
			}
			if (it == attachments.end() && object)
			{
				it = attachments.emplace(targetBit, entity).first;
			}


			const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
			attachedTargets |= 1ULL << target;
			bindTexture(targetBit);
		}
		std::shared_ptr<Texture> DetachTexture(FrameBufferTarget targetBit)
		{
			const auto it = attachments.find(targetBit);
			if (it == attachments.end())
			{
				throw std::runtime_error("Target was not present");
			}

			const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
			attachedTargets &= ~(1ULL << target);

			std::shared_ptr<Texture> temp = std::move(it->second);
			attachments.erase(it);
			return temp;
		}

		void bindTexture(FrameBufferTarget targetBit, int32_t layer = 0, CubeMapFace face = CubeMapFace::NEGATIVE_X)
		{
			if (object)
			{
				const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
				attachedTargets |= (1ULL << target);//todo error prone

				const auto& texture = *attachments[targetBit];
				const auto& base = texture.GetBase();

				switch (texture.GetTextureTarget())
				{
				case TextureTarget::Texture1D:
				case TextureTarget::Texture2D:
					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod);
					break;
				case TextureTarget::Texture3D:
				case TextureTarget::Texture1DArray:
				case TextureTarget::Texture2DArray:
				case TextureTarget::Texture2DMultisampleArray:
				case TextureTarget::TextureCubeMapArray:
					glNamedFramebufferTextureLayer(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod, layer);
					break;
				case TextureTarget::TextureRectangle:
					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), 0);
					break;
				case TextureTarget::TextureCubeMap:
					glNamedFramebufferTextureLayer(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), activeLod, gl_enum_cast(face));
					break;
				case TextureTarget::TextureBuffer:
				case TextureTarget::Texture2DMultisample:
					glNamedFramebufferTexture(object.GetID(), static_cast<int32_t>(GL_COLOR_ATTACHMENT0 + target), base.GetID(), 0);
					break;
				case TextureTarget::END:
					break;
				}
			}
		}

		void Bind(math::Bounds2D<float> size = math::Bounds2D<float>::Unit(), FrameBufferTarget targets = FrameBufferTarget::ALL)const
		{
			if (bindGlobal(this))
			{
				const math::Rectangle dim = math::UseBounds2DAsTexSize(size, w, h);
				glViewport(
					dim.x0,
					dim.y0,
					dim.x1 - dim.x0,
					dim.y1 - dim.y0
				);
			}
			drawBuffers(targets);
		}

		void ClearColor(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255, FrameBufferTarget targets = FrameBufferTarget::ALL)const
		{
			glClearColor(
				static_cast<float>(r) / 255.0f,
				static_cast<float>(g) / 255.0f,
				static_cast<float>(b) / 255.0f,
				static_cast<float>(a) / 255.0f
			);

			targets = targets & static_cast<FrameBufferTarget>(attachedTargets.to_ullong());
			std::bitset<32> colorBits = static_cast<uint32_t>(targets);

			bool color = colorBits.any();
			bool depth = static_cast<bool>(targets & FrameBufferTarget::Depth);
			bool stencil = static_cast<bool>(targets & FrameBufferTarget::Stencil);
			bindGlobal(this);
			drawBuffers(targets);
			GLenum bit = color * GL_COLOR_BUFFER_BIT;
			bit |= depth * GL_DEPTH_BUFFER_BIT;
			bit |= stencil * GL_STENCIL_BUFFER_BIT;
			glClear(bit);
		}

		void SetReadBuffer(FrameBufferTarget read)const
		{
			if (read != static_cast<FrameBufferTarget>(0) && object)
			{
				readBuffer(read);
			}
		}
		void SetDrawBuffer(FrameBufferTarget src)const
		{
			if (src != static_cast<FrameBufferTarget>(0) && object)
			{
				drawBuffers(src);
			}
		}



		void Blit(const FrameBufferObject& other, FrameBufferTarget targetOverwrite = FrameBufferTarget::USE_COLOR | FrameBufferTarget::Depth, math::Bounds2D<float> dst = math::Bounds2D<float>::Unit(), math::Bounds2D<float> src = math::Bounds2D<float>::Unit(), MagFilter filterMode = MagFilter::AUTOMATIC_BLIT)const
		{
			const math::Rectangle srcArea = math::UseBounds2DAsTexSize(src, w, h);
			const math::Rectangle dstArea = math::UseBounds2DAsTexSize(dst, w, h);





			bindGlobal(this);
			std::bitset<64> targetOverwriteBits = static_cast<uint64_t>(targetOverwrite);


			targetOverwriteBits = targetOverwriteBits & attachedTargets & other.attachedTargets;

			const std::bitset<32> overwriteColorBit = static_cast<uint32_t>(targetOverwriteBits.to_ullong());

			enum Operation
			{
				Keep,
				NoColor,
				Overwrite
			};
			const Operation operation = overwriteColorBit.all() ? Keep : overwriteColorBit.none() ? NoColor : Overwrite;

			const bool useColor = operation == Keep || operation == Overwrite;
			const bool useDepth = targetOverwriteBits.to_ullong() > overwriteColorBit.to_ulong();

			GLenum mask = useColor ? GL_COLOR_BUFFER_BIT : 0;
			mask |= useDepth ? GL_DEPTH_BUFFER_BIT : 0;
			//todo stencil buffer blitting

			if (operation == Overwrite)
			{
				const FrameBufferTarget target = static_cast<FrameBufferTarget>(overwriteColorBit.to_ulong());
				SetReadBuffer(target);
				other.SetDrawBuffer(target);
			}


			if (filterMode == MagFilter::AUTOMATIC_BLIT)
			{
				const bool useNearest = (srcArea.width() % dstArea.width() == 0 || dstArea.width() % srcArea.width() == 0) &&
					(srcArea.height() % dstArea.height() == 0 || dstArea.height() % srcArea.height() == 0);
				filterMode = useNearest || useDepth ?
					MagFilter::NEAREST : MagFilter::LINEAR;
			}




			glBlitNamedFramebuffer(
				object.GetID(),
				other.object.GetID(),
				srcArea.x0,
				srcArea.y0,
				srcArea.x1,
				srcArea.y1,
				dstArea.x0,
				dstArea.y0,
				dstArea.x1,
				dstArea.y1,
				mask,
				static_cast<int32_t>(filterMode)
			);
		}


		void SetLod(int32_t _lod) {
			if (object) {
				if (_lod > numMipMaps) {
					for (size_t i = 0; i < 64; i++) {
						if (attachedTargets[i]) {
							bindTexture(FrameBufferTarget::Albedo << i);
						}
					}
				}
			}

		}
		void Resize(int32_t _w, int32_t _h, int32_t _d) {
			w = _w;
			h = _h;
			d = _d;
			if (object) {
				for (auto& texture : std::views::values(attachments)) {
					auto& txt = *texture;
					const bool isLayered = Texture::IsTargetLayered(txt.GetTextureTarget());
					if (isLayered)
					{
						txt.Resize(w, h, txt.depth);
					}
					if (!isLayered)
					{
						txt.Resize(w, h, d);
					}
				}
			}
		}

		bool operator<(const FrameBufferObject& other) const {
			return name < other.name;
		}
		bool operator==(const FrameBufferObject& other) const {
			return name == other.name;
		}
		const std::string& GetName()const
		{
			return name;
		}


	private:

		template<typename... Targets>
			requires (std::conjunction_v<std::is_same<FrameBufferTarget, Targets>...>)
		void SymbolicAttachTexture(Targets... targetBits) const {
			auto applyHelper = [this](FrameBufferTarget targetBit) {
				const uint32_t target = std::bit_width(static_cast<uint64_t>(targetBit)) - 1;
				attachedTargets |= 1ULL << target;
				};

			(applyHelper(targetBits), ...);
		}



		static bool bindGlobal(const FrameBufferObject* renderTexture) {
			static uint32_t current = 0;
			uint32_t New = renderTexture->object.GetID();
			bool isNew = current != New;
			if (isNew) {
				current = New;
				glBindFramebuffer(GL_FRAMEBUFFER, New);
			}

			return isNew;
		}
		void drawBuffers(FrameBufferTarget _targets = FrameBufferTarget::ALL)const {
			if (object) {
				std::bitset<32> bits = static_cast<uint32_t>(_targets);
				bits &= attachedTargets.to_ullong();
				if (bits.any())
				{
					const bool changed = bits == latestDrawBuffers;
					latestDrawBuffers = bits;

					if (changed) {
						drawBufferList.clear();
						for (size_t i = 0; i < 32; i++) {
							if (latestDrawBuffers[i]) {
								drawBufferList.emplace_back(GL_COLOR_ATTACHMENT0 + i);
							}
						}

						glNamedFramebufferDrawBuffers(object.GetID(), drawBufferList.size(), drawBufferList.data());
					}
				}
			}

		}
		void readBuffer(FrameBufferTarget _target)const {

			uint32_t readBuffer = std::bit_width(static_cast<uint64_t>(_target));
			if (readBuffer == latestReadBuffer) {
				latestReadBuffer = readBuffer;
				glNamedFramebufferReadBuffer(object.GetID(), GL_COLOR_ATTACHMENT0 + readBuffer - 1);
			}
		}
	public:
		int32_t width()const
		{
			return w;
		}
		int32_t height()const
		{
			return h;
		}
	};




}
