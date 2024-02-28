#pragma once
#include <queue>

#include "components/MeshRenderer.h"
#include "gl/glsl/uniformNames.h"
#include "renderer/IRenderSystem.h"
#include "math/CullFrustum.h"
#include "renderer/rpdef.h"

namespace renderer
{
	using component::MeshRenderer;
	using component::MeshFilter;
	using component::Transform;
	using graphics::BufferObject;


	struct MeshTree
	{
		std::vector<component::EntityView> meshRenderers;
		std::vector<component::EntityView> culledMeshes;
		



		void FilterNone()
		{
			culledMeshes.clear();

			for (auto meshRenderer : meshRenderers)
			{
				culledMeshes.emplace_back(meshRenderer);
			}
		}
		void ClearFilter() {
			culledMeshes.clear();
		}
		void FilterMeshes(const math::CullFrustum& frustum)
		{
			for (auto meshRenderer : meshRenderers)
			{
				MeshFilter& meshFilter = meshRenderer.get<MeshFilter>();
				Transform& _transform = meshRenderer.get<Transform>();
				if (meshFilter.GetMesh())
				{
					math::AABB32 bounds = meshFilter.GetWorldSpaceBoundingBox(_transform);
					if (frustum.IsInFrustum(bounds.GetCenter(), bounds.GetRadius()))
					{
						culledMeshes.emplace_back(meshRenderer);
					}
				}
				else
				{
					//throw std::runtime_error("MeshFilter has no mesh");
				}

			}
		}
		void SortByMaterial()
		{
			std::ranges::sort(meshRenderers.begin(), meshRenderers.end(),
				[](component::EntityView left, component::EntityView right) {
					MeshRenderer& _left = left.get<MeshRenderer>();
					MeshRenderer& _right = left.get<MeshRenderer>();
					return _left.GetMaterial() < _right.GetMaterial();
				});
		}
		void Insert(component::EntityView meshRenderer)
		{
			meshRenderers.emplace_back(meshRenderer);
		}
		void Remove(component::EntityView meshRenderer)
		{
			auto it = std::ranges::find(meshRenderers.begin(), meshRenderers.end(), meshRenderer);
			meshRenderers.erase(it);
		}
	};

	class MeshRenderSystem final :public renderer::IRenderSystem
	{
		using Material = const shader::Material;


		MeshTree meshes;
		std::shared_ptr<shader::ComputeShader> shadowMaterial;
		scene::SceneClass* m_scene;


	public:
		entt::connection createEvent;
		entt::connection destroyEvent;



		MeshRenderSystem()
		{

		}
		void Initialize(scene::SceneClass* scene) override
		{
			m_scene = scene;
			createEvent = m_scene->registry.on_construct<MeshRenderer>().connect<&MeshRenderSystem::EmplaceMeshRenderer>(this);
			destroyEvent = m_scene->registry.on_destroy<MeshRenderer>().connect<&MeshRenderSystem::RemoveMeshRenderer>(this);

			shadowMaterial = shader::ShaderFactory::MakeShader(
				shader::ShaderType::Vertex, "$Shaders\\ShadowMap.vert",
				shader::ShaderType::Fragment, "$Shaders\\ShadowMap.frag"
			);
		}

		virtual void CullCamera(component::EntityView in)override {

			const auto& camera = in.get<CurrentCamera>().get.get<component::CameraComponent>();
			meshes.ClearFilter();
			meshes.FilterMeshes(camera.getFrustumMat());
			meshes.SortByMaterial();

		}
		virtual void CullForLight(component::EntityView in)override {
			meshes.ClearFilter();
			/*for (const auto& frustum : lightFrustum) {
				meshes.FilterMeshes(frustum);
			}*/
			meshes.FilterNone();
		}

		void RenderColor(component::EntityView in) override {

			const Buffer& cameraBuffer = *in.get<CameraBuffer>();

			/*std::queue<Material*> flushStamps;

			TextureBatch* latestBatch = nullptr;

			Material* currentMaterial = nullptr;
			graphics::TextureBatcher::Reset();

			for (const auto entity : meshes.culledMeshes)
			{
				MeshRenderer& meshRenderer = entity.get<MeshRenderer>();
				const shader::Material* newMaterial = meshRenderer.GetMaterial().get();

				if (newMaterial != currentMaterial)
				{

					TextureBatch* batch = &newMaterial->UseTextures();
					if (batch != latestBatch)
					{
						latestBatch = batch;
						flushStamps.emplace(newMaterial);
					}
					currentMaterial = newMaterial;
				}
			}*/

			Material* currentMaterial = nullptr;
			graphics::TextureBatcher::Flush();
			for (const component::EntityView entity : meshes.culledMeshes)
			{
				MeshRenderer& meshRenderer = entity.get<MeshRenderer>();
				MeshFilter& meshFilter = entity.get<MeshFilter>();
				Transform& transform = entity.get<Transform>();
				Material* material = meshRenderer.GetMaterial().get();
				Mesh* mesh = meshFilter.GetMesh();

				/*if (!flushStamps.empty() && (material == flushStamps.front()))
				{
					graphics::TextureBatcher::Flush();
					flushStamps.pop();
				}*/

				if (material && mesh)
				{
					if (currentMaterial != material)
					{
						material->Activate();
						material->UseTexturesManually();
						material->DirectShaderAccess().BindBufferRange(glsl::UniformCamera, cameraBuffer);

						currentMaterial = material;
					}
					const glm::mat4 model = transform.model();
					//renderer::Gizmos::Instance().DrawCube(glm::translate(glm::mat4(1.0f), transform.position()));
					currentMaterial->DirectShaderAccess().UniformMatrix4fv(glsl::ModelMatrix, 1, false, glm::value_ptr(model));
					mesh->Draw(currentMaterial->DirectShaderAccess().GetProgram()->GetVertexAttributeMask());
				}
			}
		}

		virtual void RenderGeometry(component::EntityView in) override {
			shader::ComputeShader* program = shadowMaterial.get();

			const Buffer& cameraBuffer = *in.get<CameraBuffer>();

			program->Bind();
			program->BindBufferRange(glsl::UniformCamera, cameraBuffer);

			for (const auto entity : meshes.culledMeshes)
			{
				MeshRenderer& meshRenderer = entity.get<MeshRenderer>();
				MeshFilter& meshFilter = entity.get<MeshFilter>();
				Transform& transform = entity.get<Transform>();
				Material* material = meshRenderer.GetMaterial().get();
				Mesh* mesh = meshFilter.GetMesh();

				if (mesh)
				{
					const glm::mat4 model = transform.model();
					renderer::Gizmos::Instance().DrawCube(model);
					mesh->Draw(program->GetProgram()->GetVertexAttributeMask());
				}

			}
		}

		~MeshRenderSystem() override = default;


		void EmplaceMeshRenderer(entt::registry& registry, entt::entity entity)
		{
			MeshRenderer* meshRenderer = &registry.get<MeshRenderer>(entity);
			meshes.Insert(component::EntityView(&registry, entity));
		}
		void RemoveMeshRenderer(entt::registry& registry, entt::entity entity)
		{
			const MeshRenderer* meshRenderer = &registry.get<MeshRenderer>(entity);
			meshes.Remove(component::EntityView(&registry, entity));
		}

	};
}
