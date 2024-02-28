#pragma once
#include <bitset>
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "SystemBase.h"

#include "Transform.h"
#include "components/FrameBufferResizeComponent.h"
#include "Math/Layermask.h"
#include "opengl/BufferObject.h"
#include "opengl/Enum/BufferTarget.h"
#include "scene/Scene.h"

namespace component
{


	template<typename Component>
	class ComponentQueue
	{
		std::vector<EntityView> enqueue;
		entt::registry* registry = nullptr;
		entt::connection handle;

	public:

		ComponentQueue() = default;

		void Initialize(entt::registry* registry)
		{
			this->registry = registry;
			handle = registry->on_construct<Component>().connect<&ComponentQueue<Component>::push_back>(&enqueue);
		}

		template<typename Function>
		void Welcome(Function func)
		{
			for(auto component: enqueue)
			{
				func(component);
			}
			enqueue.clear();
		}

		auto begin() { return enqueue.begin(); }
		auto end() { return enqueue.end(); }
		auto begin() const { return enqueue.cbegin(); }
		auto end() const { return enqueue.cend(); }
		auto cbegin() const { return enqueue.cbegin(); }
		auto cend() const { return enqueue.cend(); }

	public:
		static void push_back(std::vector<EntityView>* queue, entt::registry& registry, entt::entity entity) {
			queue->push_back(EntityView(&registry, entity));
		}
	};



	class CameraComponent : public renderer::LayerMask
	{
	public:

		class MatrixData
		{
		public:
			MatrixData() :
				modelSpaceMatrix(0.0f),
				viewSpaceMatrix(0.0f),
				camSpaceMatrix(0.0f),
				projection(0.0f)
			{

			}
			MatrixData(const glm::mat4& projection, const glm::mat4& model) :
				modelSpaceMatrix(model),
				viewSpaceMatrix(glm::inverse(model)),
				camSpaceMatrix(projection* viewSpaceMatrix),
				projection(projection)
			{

			}
		private:
			glm::mat4 modelSpaceMatrix;
			glm::mat4 viewSpaceMatrix;
			glm::mat4 camSpaceMatrix;
			glm::mat4 projection;


		public:

			const glm::mat4& getModel()const
			{
				return modelSpaceMatrix;
			}
			const glm::mat4& getView()const
			{
				return viewSpaceMatrix;
			}
			const glm::mat4& getFrustum()const
			{
				return camSpaceMatrix;
			}
			const glm::mat4& getProjection()const
			{
				return projection;
			}
		};
	private:

		MatrixData matrices;
		std::string cameraTag;
		std::shared_ptr<graphics::FrameBufferObject> renderTexture;
		int32_t renderOrder;

		float fov = 90;
		float near = 0.1f;
		float far = 1.0f;
	public:
		CameraComponent() = default;

		const glm::mat4& getFrustumMat()const
		{
			return matrices.getFrustum();
		}
		const glm::mat4& getModelMat()const
		{
			return matrices.getModel();
		}
		const glm::mat4& getViewMat()const
		{
			return matrices.getView();
		}
		const glm::mat4& getProjectionMat()const
		{
			return matrices.getProjection();
		}
	

		void initialize(float _fov, float _near, float _far, std::string _cameraTag, int32_t _renderOrder, std::shared_ptr<graphics::FrameBufferObject> _renderTexture, renderer::LayerMask layerMask = renderer::LayerMask(1))
		{
			fov = _fov;
			near = _near;
			far = _far;
			renderTexture = std::move(_renderTexture);
			cameraTag = std::move(_cameraTag);
			renderOrder = _renderOrder;
			*static_cast<LayerMask*>(this) = layerMask;
		}

		void UpdateMatrices(const Transform& transform)
		{
			matrices = MatrixData(buildPerspective(), transform.model());
		}
		

	private:
		glm::mat4 buildPerspective()
		{
			return glm::perspective(fov, static_cast<float>(renderTexture->width()) / static_cast<float>(renderTexture->height()), near, far);
		}


	public:
		
		class System : public SystemBase
		{
		public:
			scene::SceneClass* m_scene = nullptr;

			System() : SystemBase() {}


			
			static void on_destruct(System* system, entt::registry& registry, entt::entity entity) {
				const CameraComponent* camera = &registry.get<CameraComponent>(entity);
				system->removeComponent(EntityView(&registry, entity));
			}

			void Initialize(scene::SceneClass* scene) override
			{
				
				m_scene = scene;
				newComponents.Initialize(&scene->registry);
				matrixBuffer = graphics::Buffer::Create(graphics::BufferTarget::ShaderStorageBuffer);

				handle = m_scene->registry.on_destroy<CameraComponent>().connect<&System::on_destruct>(this);
			}

			void Update() override
			{
				newComponents.Welcome([this](EntityView entity){
					this->registerComponent(entity);
					});


				matrices.clear();
				for (const EntityView camera : cameras)
				{
					auto& cameraComponent = camera.get<CameraComponent>();
					auto& transformComponent = camera.get_or_emplace<Transform>();

					cameraComponent.UpdateMatrices(transformComponent);
					matrices.push_back(cameraComponent.matrices);
				}

				const size_t expectedSize = matrices.size() * sizeof(MatrixData);
				const size_t bufferSize = matrixBuffer->byteSize();

				if(!cameras.empty())
				{
					if (bufferSize < expectedSize || bufferSize >> 2 > expectedSize) {
						matrixBuffer->BufferData(utility::const_span<>(expectedSize + matrices.size() / 2), graphics::StorageHint::PRESETDynamic);
					}
					matrixBuffer->BufferSubData(matrices,0);
					/*
					todo mapping
					auto span = matrixBuffer->MapBufferWrite(matrices);
					std::memcpy(span.begin_index(), matrices.data(), span.byteSize());
					matrixBuffer->UnMap();*/
				}
			}

			EntityView GetCamera(size_t index)const {
				return cameras[index];
			}
			
			utility::const_span<EntityView> GetCameras()const {
				return cameras;
			}
			utility::const_span<MatrixData> GetMatrices()const {
				return matrices;
			}
			const graphics::Buffer& GetMatrixBuffer()const {
				return *matrixBuffer;
			}
			size_t GetMainCameraIndex()const {
				return mainCameraIndex;
			}
			

			void registerComponent(EntityView entity)
			{
				const auto cameraComponent = entity.try_get<CameraComponent>();
				if(cameraComponent)
				{
					int32_t renderLayer = cameraComponent->renderOrder;

					const auto it = std::ranges::find_if(this->cameras.begin(), this->cameras.end(), [renderLayer, entity](const EntityView& camera){
						return camera.get<CameraComponent>().renderOrder>renderLayer;
					});

					cameras.insert(it, entity);
					if (cameras.size() == 1ull)
					{
						SetMainCamera(cameraComponent->cameraTag);
					}
				}
				
			}
			void removeComponent(EntityView _camera)
			{
				const auto it = std::ranges::find_if(this->cameras.begin(), this->cameras.end(), [_camera](const EntityView& camera) {
					return camera == _camera;
					});
				cameras.erase(it);
			}

			void SetMainCamera(std::string _tag)
			{
				const auto it = std::ranges::find_if(this->cameras.begin(), this->cameras.end(), [_tag](const EntityView& camera) {
					return camera.get<CameraComponent>().cameraTag == _tag;
					});

				const size_t index = std::distance(cameras.begin(), it);
				if (index < cameras.size())
				{
					mainCameraIndex = index;
					mainCameraTag = std::move(_tag);
				}
			}

		private:
			std::vector<EntityView> cameras;
			std::shared_ptr<graphics::Buffer> matrixBuffer;
			std::vector<MatrixData> matrices;

			std::string mainCameraTag;
			size_t mainCameraIndex = 0;

			ComponentQueue<CameraComponent> newComponents;
			entt::connection handle;
		};
	};


}
