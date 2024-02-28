#pragma once
#include <glm/glm.hpp>


#include "math/DynamicTransform.h"
#include "IComponent.h"
#include "gl/GLObject.h"
#include "utility/Disposable.h"

namespace component {
	
	struct FBOResizeComponent :public utility::Disposable {

		using fbo = graphics::FrameBufferObject;

		std::shared_ptr<component::Transform2D> transform;
		component::Transform2D::Event::Handle subscription;
		fbo* frameBufferObject;
		FBOResizeComponent(graphics::FrameBufferObject* _fbo, std::shared_ptr<component::Transform2D> parent)
		{
			frameBufferObject = _fbo;
			transform = (std::move(parent));
			subscription = transform->Subscribe([this](const component::Transform2D::Update& update) {
				glm::vec3 size = update.scale * glm::vec3(1.0f, 1.0f, 0.0f);
				frameBufferObject->Resize(
					static_cast<int32_t>(size.x),
					static_cast<int32_t>(size.y), 1);
				});
		}
		
	};
}
