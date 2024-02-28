#pragma once
#include <bitset>
#include <glm/glm.hpp>
#include "math/BoundingBox.h"
#include "utility/shared.h"
#include "Event/event.h"




namespace component {

	class Transform2D {
	public:
		struct Update {
			glm::mat3 model = glm::mat3(1.0f);
			glm::mat3 scale = glm::mat3(1.0f);
			glm::mat3 rotation = glm::mat3(1.0f);
			glm::mat3 translation = glm::mat3(1.0f);
		};

		using Event = event::Event<const Update&>;

		enum PositionType {
			RelativePosition = 1,
			RelativeSize = 1 << 1,
			RelativeRotation = 1 << 2,
		};
	private:
		std::bitset<3> positionType;

		Transform2D* parent;
		std::vector<Transform2D*> children;

		mutable Update globalTransform;

		glm::mat3 localScale = glm::mat3(1.0f);
		glm::mat3 localTranslation = glm::mat3(1.0f);
		glm::mat3 localRotation = glm::mat3(1.0f);

		Event eve;

	public:

		static std::shared_ptr<Transform2D> CreateRoot(int32_t w, int32_t h) {
			auto ptr = std::make_unique<Transform2D>();
			ptr->SetLocalTransform(math::Bounds2D<float>(0.0f,0.0f,static_cast<float>(w), static_cast<float>(h)),0.0f);
			return ptr;
		}

		Transform2D()
			:positionType(0b000), parent(nullptr)
		{
			eve.Reset();
		}

		Transform2D(std::bitset<3> _positionType, Transform2D* _parent)
			:positionType(_positionType), parent(_parent)
		{
			eve.Reset();
		}

		void SetLocalTransform(math::Bounds2D<float> bounds, float angle)
		{
			localTranslation = glm::mat3({ 0,0,0 }, { 0,0,0 }, glm::vec3(bounds.x0, bounds.y0, 1.0f));
			localScale = glm::mat3(glm::vec3(bounds.width(), 0, 0), glm::vec3(0, bounds.height(), 0), glm::vec3(bounds.x0, bounds.y0, 1.0f));
			float sin = glm::sin(angle);
			float cos = glm::cos(angle);
			localRotation = glm::mat3({
					glm::vec3(cos,-sin,0),
					glm::vec3(sin, cos,0),
					glm::vec3(0.0f, 0.0f,1),
				});

			onParentTransformChanged();
		}
		
		std::shared_ptr<Transform2D> MakeChild(math::Bounds2D<float_t> _localBounds, std::bitset<3> _positionType) {

			auto ptr = children.emplace_back(new Transform2D(_positionType, this));
			ptr->SetLocalTransform(_localBounds, 1.0f);
			return std::shared_ptr<Transform2D>(ptr);
		}

		~Transform2D() {
			children.clear();
			if (parent) {
				parent->removeChild(this);
			}
		}

		const Event& OnTransformChanged() {
			return eve;
		}
		glm::ivec2 wh()const
		{
			return glm::ivec3(1,1, 1) * localScale;
		}

		Event::Handle Subscribe(Event::Action function) {
			function(globalTransform);
			return eve.Subscribe(function);
		}

	private:

		void onParentTransformChanged()const {

			glm::mat3 _translation = positionType[0] ? parent->globalTransform.translation : glm::mat3(1.0f);
			glm::mat3 _scale = positionType[1] ? parent->globalTransform.scale : glm::mat3(1.0f);
			glm::mat3 _rotation = positionType[2] ? parent->globalTransform.rotation : glm::mat3(1.0f);

			globalTransform.translation = _translation * localTranslation;
			globalTransform.scale = _scale * localScale;
			globalTransform.rotation = _rotation * localRotation;
			globalTransform.model = globalTransform.translation * globalTransform.scale * globalTransform.rotation;

			eve.Emit(globalTransform);

			for (const auto& child : children) {
				child->onParentTransformChanged();
			}
		}
		

		void removeChild(Transform2D* child) {
			children.erase(std::ranges::find_if(children.begin(), children.end(), [child](const Transform2D* _child) {
				return child == _child;
			}));
		}
	};
	using SharedTransform2D = std::shared_ptr<Transform2D>;
}
