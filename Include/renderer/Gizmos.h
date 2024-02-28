#pragma once
#include <cstdint>
#include <vector>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include "gl/GLObject.h"
#include "opengl/ShaderFactory.h"
#include "opengl/VertexArrayObject.h"

namespace glm
{
	using rgba8 = glm::vec<4, uint8_t, glm::defaultp>;
}
namespace renderer
{
	struct GizmoVertex
	{
		glm::vec3 position;
	};

	struct GizmoMesh {

		static shader::ComputeShader& GetShader()
		{
			static std::shared_ptr<shader::ComputeShader> instance =
				shader::ShaderFactory::MakeShader(
					shader::ShaderType::Vertex, "$Shaders\\Gizmo.vert",
					shader::ShaderType::Fragment, "$Shaders\\Gizmo.frag"
				);

			return *instance;
		}

		graphics::VertexArray vao;
		std::shared_ptr<graphics::Buffer> transformBuffer;
		std::shared_ptr<graphics::Buffer> positionBuffer;
		std::shared_ptr<graphics::Buffer> colorBuffer;
		std::shared_ptr<graphics::Buffer> elementBuffer;
		mutable std::vector<glm::mat4> transforms;
		mutable std::vector<glm::rgba8> colors;
		GLenum DrawMode;

		GizmoMesh(int32_t maxTransforms, utility::const_span<glm::vec3> positions, utility::const_span<uint32_t> indices, GLenum drawMode = GL_TRIANGLES)
			: vao(graphics::VertexArray::CreateStack()), DrawMode(drawMode) {

			transformBuffer = graphics::Buffer::Create(graphics::BufferTarget::ShaderStorageBuffer);
			positionBuffer = graphics::Buffer::Create(graphics::BufferTarget::ArrayBuffer);
			elementBuffer = graphics::Buffer::Create(graphics::BufferTarget::ElementArrayBuffer);
			colorBuffer = graphics::Buffer::Create(graphics::BufferTarget::ShaderStorageBuffer);



			positionBuffer->BufferData(positions, graphics::StorageHint::PRESETStaticDraw);
			elementBuffer->BufferData(indices, graphics::StorageHint::PRESETStaticDraw);
			transformBuffer->BufferData(maxTransforms * sizeof(glm::mat4), graphics::StorageHint::PRESETDynamic);
			colorBuffer->BufferData(maxTransforms * sizeof(uint32_t), graphics::StorageHint::PRESETDynamic);

			graphics::LayoutDescriptor position = attributePointer(graphics::Position, GizmoVertex, position);
			vao.VertexAttribPointer(position, *positionBuffer, 12, 0);
			vao.Finalize();
		}
		void Draw(const graphics::Buffer& camBuffer)const {
			if (!transforms.empty())
			{
				const auto& shader = GetShader();
				transformBuffer->BufferSubData(transforms, 0);
				transformBuffer->SetBufferRange(0, static_cast<int32_t>(transforms.size() * sizeof(glm::mat4)));

				colorBuffer->BufferSubData(colors, 0);
				colorBuffer->SetBufferRange(0, static_cast<int32_t>(colors.size() * sizeof(glm::rgba8)));


				shader.Bind();
				shader.BindBufferRange("CameraSSBO", camBuffer);
				shader.BindBufferRange("transformBuffer", *transformBuffer);
				shader.BindBufferRange("colorBuffer", *colorBuffer);

				vao.Bind();
				elementBuffer->Bind();

				const int32_t numIndices = elementBuffer->byteSize() / 4;

				glDrawElementsInstanced(DrawMode, numIndices, GL_UNSIGNED_INT, nullptr, static_cast<int32_t>(transforms.size()));
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
		}
		void clear()const
		{
			transforms.clear();
			colors.clear();
		}
		void add(glm::mat4 model, glm::rgba8 color) const {
			transforms.emplace_back(model);
			colors.emplace_back(color);
		}
	};
	class Gizmos
	{
		static glm::mat4 CreateLineMatrix(const glm::vec3& A, const glm::vec3& B) {
			// Direction from A to B
			glm::vec3 direction = B - A;
			float length = glm::length(direction);
			glm::vec3 unitDirection = glm::normalize(direction);

			// Create a rotation matrix to align the line with the direction vector
			glm::vec3 axis = glm::cross(glm::vec3(0, 0, 1), unitDirection);
			float angle = glm::acos(glm::clamp(glm::dot(glm::vec3(0, 0, 1), unitDirection), -1.0f, 1.0f));

			glm::mat4 rotationMatrix = glm::mat4(1.0f);
			if (glm::length(axis) > 0.000001f) { // Check to avoid division by zero
				rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::normalize(axis));
			}
			else if (glm::dot(glm::vec3(0, 0, 1), unitDirection) < 0) {
				// 180 degree rotation around any horizontal axis if the vectors are opposite
				rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1, 0, 0));
			}

			// Scaling matrix (only scale along the line's length)
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, length));

			// Translation matrix to move the line's center to the midpoint between A and B
			glm::vec3 midpoint = (A + B) / 2.0f;
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), midpoint);

			// Combine the transformations
			return translationMatrix * rotationMatrix * scaleMatrix;
		}


		std::vector<GizmoMesh> meshes;
	public:
		enum Meshes {
			Cube,
			Sphere,
			Line,
			Frustum
		};
		static Gizmos& Instance()
		{
			static Gizmos gizmos;
			return gizmos;
		}

		Gizmos()
		{

			int32_t maxTransforms = 2000;

			std::vector<glm::vec3> positions{};
			std::vector<uint32_t> indices{};

			positions = {
				{-1,-1,-1},
				{1,-1,-1},
				{-1,1,-1},
				{1,1,-1},
				{-1,-1,1},
				{1,-1,1},
				{-1,1,1},
				{1,1,1}
			};
			indices = {
				0, 1, // Bottom back edge
				1, 3, // Right back edge
				3, 2, // Top back edge
				2, 0, // Left back edge
				4, 5, // Bottom front edge
				5, 7, // Right front edge
				7, 6, // Top front edge
				6, 4, // Left front edge
				0, 4, // Bottom left edge
				1, 5, // Bottom right edge
				2, 6, // Top left edge
				3, 7  // Top right edge
			};
			meshes.emplace_back(maxTransforms, positions, indices, GL_LINES);//Cube
			//positions.clear();
			//indices.clear();


			meshes.emplace_back(maxTransforms, positions, indices, GL_LINES);//Sphere(one vertical, and one horizontal circle)
			positions.clear();
			indices.clear();

			positions = { {-1,-1,-1}, {1,1,1} };
			indices = { 0,1 };
			meshes.emplace_back(maxTransforms, positions, indices, GL_LINES);//Line(A, B) 0,0,0 B = 1,1,1
			positions.clear();
			indices.clear();

			positions = {
				{-1,-1,0},
				{1,-1,0},
				{-1,1,0},
				{1,1,0},
				{-1,-1,1},
				{1,-1,1},
				{-1,1,1},
				{1,1,1}
			};
			indices = {
				0, 1, // Bottom back edge
				1, 3, // Right back edge
				3, 2, // Top back edge
				2, 0, // Left back edge
				4, 5, // Bottom front edge
				5, 7, // Right front edge
				7, 6, // Top front edge
				6, 4, // Left front edge
				0, 4, // Bottom left edge
				1, 5, // Bottom right edge
				2, 6, // Top left edge
				3, 7,  // Top right edge
				0,3,
				1,2
			};
			meshes.emplace_back(maxTransforms, positions, indices, GL_LINES);//Frustum consists of 8 points and 6 faces which should be connected correctly
			positions.clear();
			indices.clear();
		}



		void DrawCube(const glm::mat4& model, glm::rgba8 color = glm::rgba8(255, 255, 255, 255)) const {
			meshes[Cube].add(model, color);
		}
		void DrawSphere(const glm::vec3 pos, glm::vec3 scale, glm::rgba8 color = glm::rgba8(255, 255, 255, 255)) const {
			glm::mat4 model = {
				scale.x,0.0f,0.0f,0.0f,
				0.0f,scale.y,0.0f,0.0f,
				0.0f,0.0f,scale.z,0.0f,
				pos.x,pos.y,pos.z,1.0f
			};
			meshes[Sphere].add(model, color);
		}
		void DrawSphere(const glm::mat4& model, glm::rgba8 color = glm::rgba8(255, 255, 255, 255)) const {
			meshes[Sphere].add(model, color);
		}
		void DrawLine(const glm::vec3& A, const glm::vec3& B, glm::rgba8 color = glm::rgba8(255, 255, 255, 255))const {
			meshes[Line].add(CreateLineMatrix(A, B), color);
		}
		void DrawFrustum(const glm::mat4& frustumMat, glm::rgba8 color = glm::rgba8(255, 255, 255, 255))const {
			meshes[Frustum].add(glm::inverse(frustumMat), color);
		}

		void Render(const graphics::Buffer& camBuffer)const
		{
			for (auto& mesh : meshes)
			{
				mesh.Draw(camBuffer);
			}
		}
		void Reset()const
		{
			for (auto& mesh : meshes)
			{
				mesh.clear();
			}
		}



	};
}
