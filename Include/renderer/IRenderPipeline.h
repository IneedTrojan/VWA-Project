#pragma once
#include "gl/GLObject.h"
#include "components/Entity.h"
#include "opengl/BufferObject.h"
#include "opengl/FrameBufferComponent.h"

namespace scene
{
	class SceneClass;
}

namespace renderer
{

	


	class IRenderPipeline
	{
	public:
		IRenderPipeline()
		{
			
		}
		virtual void Render() = 0;
		virtual void Initialize(scene::SceneClass* _scene) = 0;
		virtual std::shared_ptr<graphics::FrameBufferObject> GetRenderTexture(const std::string& str) = 0;

		virtual ~IRenderPipeline() = default;

	};
}