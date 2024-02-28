#pragma once
#include "opengl/Mesh.h"
#include "renderer/IRenderSystem.h"

#include <queue>
#include <variant>

#include "components/MeshRenderer.h"
#include "gl/glsl/uniformNames.h"
#include "renderer/IRenderSystem.h"
#include "math/CullFrustum.h"
#include "opengl/ShaderFactory.h"
#include "components/CameraComponent.h"
#include <glm/vec2.hpp>
#include <glm/mat3x3.hpp>


constexpr glm::vec3 positive_x{ 1, 0, 0 };
constexpr glm::vec3 positive_y{ 0, 1, 0 };
constexpr glm::vec3 positive_z{ 0, 0, 1 };
constexpr glm::vec3 negative_x{ -1, 0, 0 };
constexpr glm::vec3 negative_y{ 0, -1, 0 };
constexpr glm::vec3 negative_z{ 0, 0, -1 };



namespace renderer
{
	using namespace glm;
	using namespace graphics;
	using namespace component;


	static dvec3 cubeSphereProjection(dvec3 worldPosition, double radius)
	{
		worldPosition = worldPosition / radius;

		dvec3 squared = { worldPosition.x * worldPosition.x, worldPosition.y * worldPosition.y, worldPosition.z * worldPosition.z };

		double x = worldPosition.x * sqrt(1.0 - (squared.y - squared.z) / 2.0 + (squared.y * squared.z) / 3.0) * radius;
		double y = worldPosition.y * sqrt(1.0 - (squared.z - squared.x) / 2.0 + (squared.z * squared.x) / 3.0) * radius;
		double z = worldPosition.z * sqrt(1.0 - (squared.x - squared.y) / 2.0 + (squared.x * squared.y) / 3.0) * radius;


		return dvec3(x, y, z);
	}

	


	static dvec2 cubeSphereProjectionInverseR(dvec3 cameraWorldPosition, dmat4 pixelWorldSpace, double radius, i64vec4 bounds) {



		bounds = glm::i64vec4(std::min(bounds.x, bounds.z), std::min(bounds.y, bounds.w), std::max(bounds.x, bounds.z), std::max(bounds.y, bounds.w));

		auto spherePos = [radius, pixelWorldSpace](i64vec2 p)->dvec3 {
			return dvec3(cubeSphereProjection(pixelWorldSpace * dvec4(p, 0, 1), radius));
			};

		auto center = [](i64vec4 bounds) {
			return i64vec2((bounds.x + bounds.z) / 2, (bounds.y + bounds.w) / 2);
			};

		auto pos_in_bounds = [](dvec3 point, dvec3 min, dvec3 max) -> bool
			{
				dvec3 trueMin = dvec3(std::min(min.x, max.x), std::min(min.y, max.y), std::min(min.z, max.z));
				dvec3 trueMax = dvec3(std::max(min.x, max.x), std::max(min.y, max.y), std::max(min.z, max.z));

				trueMax.z += 10;
				trueMin.z -= 10;

				bool isInBounds = true;
				isInBounds &= point.x >= trueMin.x && point.x <= trueMax.x;
				isInBounds &= point.y >= trueMin.y && point.y <= trueMax.y;
				isInBounds &= point.z >= trueMin.z && point.z <= trueMax.z;

				return isInBounds;
			};


		i64vec2 middle = center(bounds);
		i64vec2 extends = i64vec2(bounds.z - bounds.x, bounds.w - bounds.y) / 2ll;

		glm::dvec3 centerPos = spherePos(middle);

		for (size_t i = 0; i < 4 && (bounds.z - bounds.x && bounds.w - bounds.y); i++)
		{
			i64 sx = (i % 2) ? 1 : -1;
			i64 sy = (i / 2) % 2 ? 1 : -1;

			glm::i64vec2 child_top_pixel = middle + extends * i64vec2(sx, sy);
			glm::dvec3 child_top = spherePos(child_top_pixel);
			if (pos_in_bounds(cameraWorldPosition, centerPos, child_top))
			{
				return cubeSphereProjectionInverseR(cameraWorldPosition, pixelWorldSpace, radius, glm::i64vec4(middle, child_top_pixel));
			}
		}



		return center(bounds);
	}

	static std::pair<i64vec2, double> cubeSphereProjectionInversed(dvec3 cameraWorldPosition, dmat4 pixelWorldSpace, double radius, i64vec4 bounds)
	{
		dvec3 sphereCoords = normalize(cameraWorldPosition) * radius;
		double distance = glm::distance(cameraWorldPosition, sphereCoords);
		return { cubeSphereProjectionInverseR(cameraWorldPosition, pixelWorldSpace, radius, bounds),distance };

	}






	inline i64 intFract(i64 value, i64 modulus) {
		int remainder = value % modulus;
		remainder += i64(remainder < 0) * modulus;
		return remainder;
	}
	inline i64vec2 intFract(i64vec2 value, i64 modulus) {
		return { intFract(value.x, modulus), intFract(value.y, modulus) };
	}
	
	struct IndexVertex
	{
		uint32_t index;
	};

	

	struct GpuClipMap
	{
		glm::mat4 relativeTransform;
		ivec4 activeIndices;
		glm::ivec2 cameraScroll;
		i32 face;
		i32 textureLayer;
	};
	

	struct Face
	{
		Face()
			:TBN(1.0), AJ_RIGHT_TBN(1), AJ_TOP_TBN(1)
		{
		}
		Face(ivec3 T, ivec3 B, ivec3 N)
			:TBN(T, B, N)
		{
			glm::dmat4 rotationMatrixRight = glm::rotate(glm::dmat4(1.0), glm::half_pi<double>(), glm::dvec3(TBN[2]));
			AJ_RIGHT_TBN = glm::dmat3(rotationMatrixRight) * TBN;

			glm::dmat4 rotationMatrixTop = glm::rotate(glm::dmat4(1.0), -glm::half_pi<double>(), glm::dvec3(TBN[1]));
			AJ_TOP_TBN = glm::dmat3(rotationMatrixTop) * TBN;
		}

		dmat3 TBN;
		dmat3 AJ_RIGHT_TBN;
		dmat3 AJ_TOP_TBN;

	};

	struct ClipMapStatic
	{
		std::array<glm::dmat4, 3>pixelWorldSpace;
		std::array<glm::dmat4, 3>worldPixelSpace;

		Bounds2D<i64> scrollBounds;
		Bounds2D<i64> pixelBounds;

		Face* face;
		double size;
		double worldSize;
		double radius;
		Bounds2D<i64> usedTexture;
		Bounds2D<i64> textureBounds;
		i32 textureLayer;
		i32 faceIndex;



		ClipMapStatic(Face* face,i32 faceIndex, i32 textureLayer, double size, double worldSize, double radius, i32 resolution, ivec4 border)
			:face(face), size(size), worldSize(worldSize), radius(radius), textureLayer(textureLayer), faceIndex(faceIndex)
		{
			textureBounds = Bounds2D<i64>{ 0,0,resolution, resolution };
			usedTexture = Bounds2D<i64>{border.x, border.y, resolution-border.z, resolution-border.w};

			dvec2 pixelSize = size/(dvec2(usedTexture.size()-1ll));

			std::array<dvec2, 3> center_shift;
			center_shift[0] = pixelSize * dvec2(usedTexture.convert<f64>().GetCenter());
			center_shift[1] = pixelSize * dvec2(usedTexture.convert<f64>().GetCenter() * 2.0);
			center_shift[2] = pixelSize * dvec2(usedTexture.convert<f64>().GetCenter() * 2.0);


			std::array<dvec3, 3> BtmL;

			BtmL[0] = face->TBN * -dvec3(center_shift[0], radius / 2.0);
			BtmL[1] = face->AJ_RIGHT_TBN * -dvec3(center_shift[1], radius / 2.0);
			BtmL[2] = face->AJ_TOP_TBN * -dvec3(center_shift[2], radius / 2.0);


			pixelWorldSpace[0] = face->TBN;
			pixelWorldSpace[0][0] *= pixelSize.x;
			pixelWorldSpace[0][1] *= pixelSize.y;
			pixelWorldSpace[0][3] = dvec4(BtmL[0], 1);

			pixelWorldSpace[1] = face->AJ_RIGHT_TBN;
			pixelWorldSpace[1][0] *= pixelSize.x;
			pixelWorldSpace[1][1] *= pixelSize.y;
			pixelWorldSpace[1][3] = dvec4(BtmL[1], 1);

			pixelWorldSpace[2] = face->AJ_TOP_TBN;
			pixelWorldSpace[2][0] *= pixelSize.x;
			pixelWorldSpace[2][1] *= pixelSize.y;
			pixelWorldSpace[2][3] = dvec4(BtmL[2], 1);


			worldPixelSpace[0] = inverse(pixelWorldSpace[0]);
			worldPixelSpace[1] = inverse(pixelWorldSpace[1]);
			worldPixelSpace[2] = inverse(pixelWorldSpace[2]);

			
			i64vec2 max = i64vec2(dvec2(worldSize) / pixelSize/2.0);
			i64vec2 min = -i64vec2(dvec2(worldSize) / pixelSize / 2.0);

			scrollBounds = {min + usedTexture.size() / 2ll, max - usedTexture.size() /2ll };

			scrollBounds = scrollBounds.Transpose(usedTexture.size() / 2ll + 1ll + usedTexture.btm_l());

			

			pixelBounds = scrollBounds;
			pixelBounds.top_r() += usedTexture.size();

		}
		

	};


	struct ClipMap: ClipMapStatic
	{
		std::array<dmat4, 3> model;
	

		i64vec2 scroll;

		bool IsActive = false;
		bool IsInitialized = false;

		ClipMap(const ClipMapStatic& base) : ClipMapStatic(base)
		{
			
		}
		i64vec2 get_scroll_cube_sphere(dvec3 cameraPos, double* distance = nullptr)const
		{
			std::pair<i64vec2, double> pixelPos = cubeSphereProjectionInversed(cameraPos, pixelWorldSpace[0], radius, pixelBounds);

			if (distance)*distance = pixelPos.second;

			return pixelPos.first;
		};
		i64vec2 get_scroll(dvec3 cameraPos)const
		{
			
			i64vec2 pixelPos = i64vec2(worldPixelSpace[0] * dvec4(cameraPos, 1));
			pixelPos = pixelPos / 2ll * 2ll;
			pixelPos = scrollBounds.fit(pixelPos);
			return pixelPos;
		};
		
		void set_scroll(i64vec2 pixelScroll)
		{
			
			scroll = pixelScroll;
			const dvec3 position = pixelWorldSpace[0] * dvec4(pixelScroll, 0, 1);
			model[0] = pixelWorldSpace[0];
			model[0][3] += dvec4(position, 1);
		}

		template<i64 coordinate>
		requires (coordinate == 0 || coordinate == 1)
		dmat4& get_corner_matrix(i64 pixel)
		{
			const i64 overflow = get_pixel_overflow(i64vec2(pixel * coordinate, pixel * (1ll - coordinate)))[coordinate];
			if(overflow > 0)
			{
				if constexpr(coordinate == 0)
				{
					return model[1];
				}
				if constexpr (coordinate == 1)
				{
					return model[2];
				}
			}
			return model[0];
		}

		i64vec2 get_pixel_overflow(i64vec2 pixel)const
		{
			pixel += scroll;
			return pixelBounds.Overflow(pixel);
		}

	};
	


	class TerrainRenderSystem :public IRenderSystem
	{
		std::shared_ptr<shader::ComputeShader> terrainShader;
		std::shared_ptr<shader::ComputeShader> worldGenShader;

		static constexpr double findNearestExp(double value, double search)
		{
			while(value>=search*2)
			{
				value *= 0.5;
			}
			return value;
		}
		static constexpr i32 findNumIterations(double value, double search)
		{
			i32 it = 0;
			while (value >= search * 2)
			{
				value *= 0.5;
				it++;
			}
			return it;
		}

		std::shared_ptr<shader::Material> vertexArrayTerrainShader;

		scene::SceneClass* m_scene = nullptr;


		static constexpr i32 numFaces = 6;

		double worldSize = 10000;
		double radius = worldSize/2;

		double minChunk = findNearestExp(worldSize, 2000);

		

		i32 clipMapCount = findNumIterations(worldSize, minChunk) + 1;//pow(minChunk, clipMapCount)==worldSize
		i32 layerCount = clipMapCount * numFaces;

		std::shared_ptr<Texture> heightMap;

		std::shared_ptr<Buffer> vertexFetchBuffer;
		int32_t resolution = 512;
		int32_t border_left = 8;
		int32_t border_right = 9;
		

		std::array<Face, numFaces> faces;

		



		

		float terrainHeight = 100;
		CameraComponent* camera;

		Entity entity;

		std::shared_ptr<graphics::Buffer> debugBuffer;


		std::vector<ClipMap> clipMaps;

		


	public:


		

		TerrainRenderSystem()
		{
			if(faces.size() > 0)
			{
				faces[0] = Face(positive_y, negative_z, positive_x);
			}
			if(faces.size()>5)
			{
				faces[1] = Face(positive_y, positive_z, negative_x);
				faces[2] = Face(positive_x, negative_z, positive_y);
				faces[3] = Face(negative_x, negative_z, negative_y);
				faces[4] = Face(positive_x, negative_y, positive_z);
				faces[5] = Face(negative_x, negative_y, negative_z);
			}
			
		}
		static float inverseLerp(float a, float b, float x) {
			return (x - a) / (b - a);
		}


		static std::shared_ptr<Mesh> CreateIndexMesh(i32 min, i32 max, int32_t heightmapRes)
		{

			std::vector<LayoutDescriptor> layoutDescriptor;
			layoutDescriptor.emplace_back(attributePointer(0, IndexVertex, index));

			
			std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(IndexVertex), layoutDescriptor);
			std::vector<IndexVertex> vertices;
			std::vector<uint32_t> indices;

			i32 meshRes = max - min;

			size_t currentVertex = 0;

			for (i32 y = 0; y < heightmapRes; y++)
			{
				for (i32 x = 0; x < heightmapRes; x++)
				{
					 
					if(x >= min && y >= min && x< max && y < max)
					{
						if (x < max-1 && y < max -1) {
							indices.emplace_back(vertices.size());
							indices.emplace_back(vertices.size() + static_cast<int>(meshRes) + 1);
							indices.emplace_back(vertices.size() + 1);
							indices.emplace_back(vertices.size());
							indices.emplace_back(vertices.size() + static_cast<int>(meshRes));
							indices.emplace_back(vertices.size() + static_cast<int>(meshRes) + 1);
						}

						vertices.emplace_back(currentVertex);
					}
					currentVertex++;
				}
			}
			mesh->SetVertexData(vertices);
			mesh->SetIndexData(indices);
			mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::None);
			mesh->Sync();
			return mesh;
		}

		
		void Initialize(scene::SceneClass* scene) override
		{
			m_scene = scene;


			auto border = ivec4(border_left, border_left, border_right, border_right);
			i32 textureIndex = 0;
			for(i32 face = 0; face < numFaces; face++)
			{
				for (i32 i = 0; i < clipMapCount; i++)
				{
					double chunkSize = minChunk * pow(2, i);
					clipMaps.emplace_back(ClipMapStatic(&faces[face], face,textureIndex++, chunkSize, worldSize, radius, resolution, border));
				}
			}
			
			
			worldGenShader = shader::ShaderFactory::MakeComputeShader(
				"$Resources\\ComputeShaders\\HeightmapShader.vert"
			);
			
			Sampler sampler;
			sampler.minFilter = MinFilter::LINEAR; // Use linear filtering for minification
			sampler.magFilter = MagFilter::LINEAR; // Use linear filtering for magnification
			sampler.wrapS = WrapMode::REPEAT;      // Repeat wrap mode on S (X) axis
			sampler.wrapT = WrapMode::REPEAT;      // Repeat wrap mode on T (Y) axis



			heightMap = std::make_shared<Texture>(TextureTarget::Texture2DArray, TextureFormat::R16);
			heightMap->TextureParams(glm::ivec3(resolution, resolution, layerCount));
			heightMap->Allocate(sampler);

		
			vertexArrayTerrainShader = shader::ShaderFactory::MakeMaterial(
				shader::ShaderType::Vertex, "$Resources\\Shaders\\BufferTerrain.vert",
				shader::ShaderType::Fragment, "$Resources\\Shaders\\Terrain.frag",
				shader::ShaderType::Geometry, "$Resources\\Shaders\\terrain.geo"
			);

			auto mesh = CreateIndexMesh(border_left, resolution-border_right, resolution);
			

			entity = m_scene->CreateEntity();
			entity.emplace<MeshFilter>().SetMesh(mesh);
			entity.emplace<MeshRenderer>().SetMaterial(vertexArrayTerrainShader);
			entity.emplace<Transform>();



			levelBuffer = Buffer::Create(BufferTarget::ShaderStorageBuffer);
			levelBuffer->BufferData(sizeof(GpuClipMap) * clipMapCount* numFaces, StorageHint::DynamicStorage);



			taskBuffer = Buffer::Create(BufferTarget::ShaderStorageBuffer);

		
			vertexArrayTerrainShader->BindBuffer("LevelSSBO", levelBuffer);
			vertexArrayTerrainShader->SetInt("resolution", ivec2(resolution));
			vertexArrayTerrainShader->SetFloat("radius", radius);
			vertexArrayTerrainShader->SetFloat("elevation", terrainHeight);
			vertexArrayTerrainShader->BindTexture("heightmapTexture", heightMap);

		}

		std::shared_ptr<Buffer> taskBuffer;

		struct Task {
			std::array<dmat4, 1> model;

			ivec2 pixelMin;
			ivec2 pixelMax;
			ivec2 cameraPixelPosition;
			int layer;
			rgba8 debug_color;


		};
		
		std::vector<GpuClipMap> levels;
		std::shared_ptr<Buffer> levelBuffer;

		float centerHeight = 0;

		

		void TextureUpdate(utility::const_span<Task> areas, glm::dmat4 cameraModel, glm::dmat4 cameraMatrix)
		{

			levels.clear();
			auto mat = glm::mat3((worldSize/2.0));
			mat[2] = glm::vec3(0, 0, 2);
			//Gizmos::Instance().DrawCube(mat);

			for(int32_t face = 0;face< numFaces;face++)
			{
				bool empty = true;
				

				for (int32_t i = 0; i < clipMapCount; i++)
				{
					const i32 current = face * clipMapCount + i;
					ClipMap& cm = clipMaps[current];
					if (cm.IsActive && cm.IsInitialized)
					{
						const mat4& model = cm.model[0];

						Bounds2D<i64> culledArea = { 0,0,0,0 };
						if (!empty)
						{
							auto& child = clipMaps[current - 1];
							i64vec2 dif = cm.scroll - child.scroll / 2ll;

							//Gizmos::Instance().DrawCube(model, rgba8(255, 0, 0, 255));

							culledArea.x0 = -dif.x + resolution / 4 + resolution / 4 + border_left/2 + 2;
							culledArea.y0 = -dif.y + resolution / 4 + resolution / 4 + border_left / 2 + 2;
							culledArea.x1 = -dif.x + resolution * 3 / 4 + resolution / 4 - border_right / 2 - 1;
							culledArea.y1 = -dif.y + resolution * 3 / 4 + resolution / 4 - border_right / 2 - 1;
							culledArea = culledArea.Transpose(i64vec2(-border_right/2ll +2));
							//culledArea = cm.textureBounds.convert<f64>().Scale(dvec2(0.5)).convert<i64>().Transpose(-dif+ cm.textureBounds.size());
						}

						GpuClipMap gpuClipMap;
						gpuClipMap.relativeTransform = model;
						gpuClipMap.cameraScroll = cm.scroll;
						gpuClipMap.activeIndices = culledArea.convert<i32>();
						gpuClipMap.textureLayer = cm.textureLayer;
						gpuClipMap.face = cm.faceIndex;
						levels.emplace_back(gpuClipMap);
						empty = false;
					}
				}
			}
			

			entity.get<MeshFilter>().GetMesh()->instanceCount = levels.size();
			vertexArrayTerrainShader->SetMatrix("cameraMatrix", cameraMatrix);
			vertexArrayTerrainShader->SetFloat("cameraPosition", cameraModel[3]);
			levelBuffer->BufferSubData(levels);

			if (areas.size() > 0)
			{
				size_t byteSize = areas.byteSize();
				if (taskBuffer->byteSize() < byteSize)
				{
					taskBuffer = Buffer::Create(BufferTarget::ShaderStorageBuffer);
					taskBuffer->BufferData(areas.byteSize(), StorageHint::PRESETDynamic);
				}
				taskBuffer->BufferSubData(areas, 0);

				
				glm::ivec3 tempRes = ivec3(resolution, resolution, clipMapCount);
				worldGenShader->Bind();
				worldGenShader->BindBufferRange("Tasks", *taskBuffer);
				//worldGenShader->BindBufferRange("VertexFetchSSBO", *vertexFetchBuffer);
				worldGenShader->Uniform3iv("resolution", &tempRes[0]);
				worldGenShader->Uniform1dv("radius", &radius);
				heightMap->BindToImageUnit(1, WriteAccess::Writeonly);

				worldGenShader->DispatchCompute(areas.size(), 1, 1);
				
				glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
				glFinish();

			}

		}

		


		void UpdateTextures(component::CameraComponent& camera)
		{
			const glm::mat4& cameraModel = camera.getModelMat();
			const glm::mat4& cameraMatrix = camera.getFrustumMat();


			glm::dvec3 cameraPosition = cameraModel[3];

			
			std::vector<Task> tasks;

			for(size_t face = 0;face<numFaces;face++)
			{
				i64 offset = face * clipMapCount;
				ClipMap first = clipMaps[offset];
				i64vec2 clip_map_scroll = first.get_scroll_cube_sphere(cameraPosition);

				for (int32_t clipmap_index = 0; clipmap_index < clipMapCount; clipmap_index++)
				{
					int32_t i = offset + clipmap_index;
					ClipMap& clipmap = clipMaps[i];
					clipmap.IsActive = true;
					//(cameraPosition.z - centerHeight) < (clipmap.IsActive ? clipmap.maxDistance  : clipmap.maxDistance / 1.2

					i64vec2 old_scroll = clipmap.scroll;
					i64vec2 i64_scroll = clipmap.scrollBounds.fit((clip_map_scroll / 2ll) * 2ll);//clipmap.get_scroll(cameraPosition, false).first;

					clip_map_scroll /= 2;

					clipmap.set_scroll(i64_scroll);


					auto red = rgba8(255, 0, 0, 255);
					if (clipmap.IsActive)
					{
						Task task;

						task.layer = i;
						task.debug_color = red;
						task.model[0] = clipmap.model[0];

						ivec2 delta = i64_scroll - i64vec2(old_scroll);
						ivec2 wh = abs(delta);
						wh = clipmap.IsInitialized ? clamp(wh, 0, resolution) : glm::ivec2(resolution, 0);


						for (int32_t row = 0; row < wh.y; row++)
						{
							Task _task = task;
							int32_t y = delta.y < 0 ? row : resolution - row - 1;



							_task.cameraPixelPosition = ivec2(i64_scroll.x, i64_scroll.y);
							_task.pixelMin = vec2(0, y);
							_task.pixelMax = vec2(resolution, y);
							_task.pixelMax = vec2(resolution, y);
							tasks.push_back(_task);
						}

						for (int32_t col = 0; col < wh.x; col++)
						{
							Task _task = task;
							int32_t x = delta.x < 0 ? col : resolution - col - 1;


							_task.cameraPixelPosition = ivec2(i64_scroll.x, i64_scroll.y);
							_task.pixelMin = vec2(x, 0);
							_task.pixelMax = vec2(x, resolution);
							tasks.push_back(_task);
						}

						clipmap.IsInitialized = true;


					}
				}
			}

			
			TextureUpdate(tasks, cameraModel, cameraMatrix);
			
		}


		entt::connection createEvent;
		entt::connection destroyEvent;


		virtual void CullCamera(EntityView in)override {

			CameraComponent& camera = in.get<CurrentCamera>().get.get<CameraComponent>();
			this->camera = &camera;
			UpdateTextures(camera);


		}
		virtual void CullForLight(EntityView in)override {


		}


		virtual void RenderColor(EntityView in) override {

			mat4 model = glm::mat4(1.0);
			//Gizmos::Instance().DrawCube(model, rgba8(255,0,0,255));
		}

		virtual void RenderGeometry(EntityView in) override {

		}

		~TerrainRenderSystem() override {}
	};


	namespace renderer
	{


	}




}















namespace spericalTerrain
{
	using namespace glm;
	using namespace graphics;
	using namespace component;


	inline i64 intFract(i64 value, i64 modulus) {
		int remainder = value % modulus;
		remainder += i64(remainder < 0) * modulus;
		return remainder;
	}
	inline i64vec2 intFract(i64vec2 value, i64 modulus) {
		return { intFract(value.x, modulus), intFract(value.y, modulus) };
	}
	




	class TerrainRenderSystem :public renderer::IRenderSystem
	{

		struct Vertex
		{
			uint index;
		};

		struct LODLevel
		{
			double chunkSize;
			double numChunks;
		};
		

		struct Chunk
		{
			glm::dmat4 model;
			glm::dmat3 tbn;
		};
		struct DrawCall
		{
			glm::mat4 model;
		};

		std::vector<DrawCall> dc;


		std::shared_ptr<shader::ComputeShader> heightmapShader;
		std::shared_ptr<shader::ComputeShader> worldGenShader;

		std::shared_ptr<Texture> heightmapArray;

		Entity terrain;
		std::shared_ptr<Mesh> mesh;
		std::shared_ptr<shader::Material> material;



		i64 heightmapResolution =512ll;
		i64 clipMapCount = 1ll;
		scene::SceneClass* m_scene;

		double radius = 500;




	public:




		TerrainRenderSystem()
		{

		}
		static float inverseLerp(float a, float b, float x) {
			return (x - a) / (b - a);
		}


		static std::shared_ptr<Mesh> GenerateHalfCube(int32_t heightmapRes)
		{


			std::vector<LayoutDescriptor> layoutDescriptor;
			layoutDescriptor.emplace_back(attributePointer(0, Vertex, index));


			std::shared_ptr<Mesh> mesh = Mesh::Create(sizeof(Vertex), layoutDescriptor);
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;


			size_t currentVertex = 0;

			for (int32_t y = 0; y < heightmapRes; y++)
			{
				for (int32_t x = 0; x < heightmapRes; x++)
				{

					if (x != heightmapRes-1 && y != heightmapRes -1)
					{
							indices.emplace_back(vertices.size());
							indices.emplace_back(vertices.size() + static_cast<int>(heightmapRes) + 1);
							indices.emplace_back(vertices.size() + 1);
							indices.emplace_back(vertices.size());
							indices.emplace_back(vertices.size() + static_cast<int>(heightmapRes));
							indices.emplace_back(vertices.size() + static_cast<int>(heightmapRes) + 1);

					}
					vertices.emplace_back(currentVertex);

					currentVertex++;
				}
			}
			mesh->SetVertexData(vertices);
			mesh->SetIndexData(indices);
			mesh->SetPolygonMode(TriangleMode::Triangle, FillMode::Fill, CullFace::None);
			mesh->Sync();
			return mesh;
		}


		void Initialize(scene::SceneClass* scene) override
		{
			m_scene = scene;

			worldGenShader = shader::ShaderFactory::MakeComputeShader(
				"$Resources\\ComputeShaders\\HeightmapShader.vert"
			);

			Sampler sampler;
			sampler.minFilter = MinFilter::LINEAR; // Use linear filtering for minification
			sampler.magFilter = MagFilter::LINEAR; // Use linear filtering for magnification
			sampler.wrapS = WrapMode::REPEAT;      // Repeat wrap mode on S (X) axis
			sampler.wrapT = WrapMode::REPEAT;      // Repeat wrap mode on T (Y) axis


			heightmapArray = std::make_shared<Texture>(TextureTarget::Texture2DArray, TextureFormat::R32F);
			heightmapArray->TextureParams(glm::ivec3(heightmapResolution, heightmapResolution, clipMapCount));
			heightmapArray->Allocate(sampler);

		
			material = shader::ShaderFactory::MakeMaterial(
				shader::ShaderType::Vertex, "$Resources\\Shaders\\SphericalTerrain.vert",
				shader::ShaderType::Fragment, "$Resources\\Shaders\\SphericalTerrain.frag"

				//,shader::ShaderType::Geometry, "$Resources\\Shaders\\SphericalTerrain.geo"
			);

			mesh = GenerateHalfCube(heightmapResolution);
			mesh->instanceCount = clipMapCount;

			terrain = m_scene->CreateEntity();
			terrain.emplace<MeshFilter>().SetMesh(mesh);
			terrain.emplace<MeshRenderer>().SetMaterial(material);
			terrain.emplace<Transform>();




		}

		void UpdateTextures(component::CameraComponent& camera)
		{
			mat4 cameraModel = camera.getModelMat();
			mat4 model = glm::mat4(1.0);
			mat4 camMatrix = camera.getFrustumMat();


			material->SetMatrix("model", model);
			material->SetMatrix("cameraMatrix", camMatrix);
			material->SetFloat("viewerPosition", vec3(camera.getModelMat()[3]));
			material->SetInt("resolution", heightmapResolution);

		}



		virtual void CullCamera(EntityView in)override {

			UpdateTextures(in.get<renderer::CurrentCamera>().get.get<CameraComponent>());

		}
		virtual void CullForLight(EntityView in)override {


		}


		virtual void RenderColor(EntityView in) override {

			
		}

		virtual void RenderGeometry(EntityView in) override {

		}

		~TerrainRenderSystem() override {}
	};


	namespace renderer
	{


	}




}
