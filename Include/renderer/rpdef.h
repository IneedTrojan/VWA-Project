#pragma once
#include "components/IComponent.h"
#include "opengl/BufferObject.h"

namespace renderer
{
	using CurrentCamera = component::named_component<component::EntityView, "currentCamera">;
	using CameraBuffer = component::named_component<const graphics::Buffer*, "cameraBuffer">;

}
