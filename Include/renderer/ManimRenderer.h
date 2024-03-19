#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "opengl\\Mesh.h"
#include "opengl\\FrameBufferComponent.h"
#include "opengl\\ShaderFactory.h"
#include "components\\Transform.h"

namespace manim {

	using namespace graphics;

	struct Renderer {



		struct Point {
			glm::vec3 position;
			glm::i32 color;
			glm::f32 thickness;
		};
		

		struct SphereQuat {
			glm::vec3 position;
			glm::vec2 size;
			glm::i32 centerColor;
			glm::i32 emittingColor;
			glm::f32 gradient;
		};


		Renderer() {
			reset_meshes();


		}
		void reset_meshes() {
			std::vector<graphics::LayoutDescriptor> lineLayout;
			lineLayout.emplace_back(attributePointer(0, Point, position));
			lineLayout.emplace_back(attributePointer(1, Point, color));
			lineLayout.emplace_back(attributePointer(2, Point, thickness));


			std::vector<graphics::LayoutDescriptor> dotLayout;
			dotLayout.emplace_back(attributePointer(0, SphereQuat, position));
			dotLayout.emplace_back(attributePointer(1, SphereQuat, size));
			dotLayout.emplace_back(attributePointer(2, SphereQuat, centerColor));
			dotLayout.emplace_back(attributePointer(3, SphereQuat, emittingColor));
			dotLayout.emplace_back(attributePointer(4, SphereQuat, gradient));


			lineMesh = graphics::Mesh::Create(sizeof(Point), lineLayout);
			dotsMesh = graphics::Mesh::Create(sizeof(SphereQuat), dotLayout);
			triangleMesh = graphics::Mesh::Create(sizeof(Point), lineLayout);

			lineMesh->SetPolygonMode(TriangleMode::Line, FillMode::Fill, CullFace::None);
			dotsMesh->SetPolygonMode(TriangleMode::Point, FillMode::Fill, CullFace::None);
			triangleMesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Line, CullFace::None);

		}

		std::shared_ptr<graphics::Mesh> lineMesh;
		std::shared_ptr<graphics::Mesh> dotsMesh;
		std::shared_ptr<graphics::Mesh> triangleMesh;

		std::shared_ptr<shader::Material> lineMaterial;
		std::shared_ptr<shader::Material> dotMaterial;
		std::shared_ptr<shader::Material> triangleMaterial;

		component::Transform cameraTransform;



		void refreshMeshes() {
			lineMesh->Sync(StorageHint::PRESETDynamic);
			dotsMesh->Sync(StorageHint::PRESETDynamic);
			triangleMesh->Sync(StorageHint::PRESETDynamic);

		}


		


		void render(std::shared_ptr<graphics::FrameBufferObject> fbo) {
			fbo->Bind();
			refreshMeshes();

			glm::vec3 cameraPosition = cameraTransform.position();
			glm::vec3 cameraViewDirection = -cameraTransform.up();

			dotMaterial->SetFloat("cameraPosition", cameraPosition);
			glEnable(GL_DEPTH_TEST);

			if (triangleMesh->vertexCount() != 0) {

				triangleMaterial->Activate();
				triangleMesh->DrawVertices(triangleMaterial->DirectShaderAccess().GetProgram()->GetVertexAttributeMask());
			}

			if (dotsMesh->vertexCount() != 0) {
				dotMaterial->SetFloat("cameraPosition", cameraPosition);
				dotMaterial->SetFloat("cameraViewDirection", cameraViewDirection);

				dotMaterial->Activate();
				dotsMesh->DrawVertices(dotMaterial->DirectShaderAccess().GetProgram()->GetVertexAttributeMask());
			}

			if (lineMesh->vertexCount() != 0) {
				lineMaterial->SetFloat("cameraPosition", cameraPosition);

				lineMaterial->Activate();
				lineMesh->DrawVertices(lineMaterial->DirectShaderAccess().GetProgram()->GetVertexAttributeMask());
			}
			
			
			reset_meshes();
		}
	


		void drawLine(glm::vec3 a, glm::vec3 b, glm::u8 red, glm::u8 green, glm::u8 blue, float thickness) {
			
			glm::u8vec4 colorA = { red, green, blue, 255 };

			Point A;
			A.position = a;
			A.color = *reinterpret_cast<glm::i32*>(&colorA);
			A.thickness = thickness;
			Point B;
			B.position = b;
			B.color = *reinterpret_cast<glm::i32*>(&colorA);
			B.thickness = thickness;

			lineMesh->push_back_vertex(A);
			lineMesh->push_back_vertex(B);
		}


	
	
		void drawDot(glm::vec3 pos, float radius = 1.0, glm::u8 r = 120, glm::u8 g = 20, glm::u8 b = 50, glm::u8 a = 255) {

			drawDot(pos, radius, glm::u8vec4(r, g, b, a), glm::u8vec4(r * 2, g * 2, b * 2, a), 1.0);
		}

		void drawDot(glm::vec3 pos, float radius, glm::u8vec4 color, glm::u8vec4 glowColor, float gradient) {




			SphereQuat billboard;
			billboard.position = pos;
			billboard.size = glm::vec2(radius, radius);
			billboard.centerColor = *reinterpret_cast<const glm::i32*>(&color);
			billboard.emittingColor = *reinterpret_cast<const glm::i32*>(&glowColor);
			billboard.gradient = gradient;

			dotsMesh->push_back_vertex(billboard);
		}
	};


	

	




}