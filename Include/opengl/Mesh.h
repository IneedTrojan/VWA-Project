#pragma once
#include "VertexArrayObject.h"
#include "BufferObject.h"
#include "math/BoundingBox.h"
#include "opengl/Enum/Cullface.h"
#include <type_traits>

namespace graphics
{


	template<typename T>
	concept HasPosition = requires(T a) {
		// Check for member 'position'
		{ a.position } -> std::convertible_to<decltype(a.position)>;
	};

	template<typename T>
	concept HasTangent = requires(T a) {
		// Check for member 'position'
		{ a.tangent } -> std::convertible_to<decltype(a.tangent)>;
	};
	template<typename T>
	concept HasNormal = requires(T a) {
		// Check for member 'position'
		{ a.normal } -> std::convertible_to<decltype(a.normal)>;
	};
	template<typename T>
	concept HasBitangent = requires(T a) {
		// Check for member 'position'
		{ a.bitangent } -> std::convertible_to<decltype(a.bitangent)>;
	};
	template<typename T>
	concept HasUV = requires(T a) {
		// Check for member 'position'
		{ a.uv } -> std::convertible_to<decltype(a.uv)>;
	};

	class Mesh
	{


	public:
		MultiVertexArray multiVAO;
		std::shared_ptr<Buffer> vbo;
		std::shared_ptr<Buffer> ebo;
		DataType eboDataType;
		size_t vertexSize;

		math::AABB32 boundingBox;
		size_t instanceCount = 1;

		std::vector<char> vertices;
		std::vector<char> indices;


		static size_t& TriangleCount()
		{
			static size_t count = 0;
			return count;
		}

		
	public:
		TriangleMode triangleMode = TriangleMode::Triangle;
		FillMode fillMode = FillMode::Fill;
		CullFace cullFace = CullFace::Back;

		

		static std::shared_ptr<Mesh> Create(uint64_t&& structSize, utility::const_span<LayoutDescriptor> _layouts)
		{
			return std::shared_ptr<Mesh>(new Mesh(structSize, _layouts));
		}


		void SetPolygonMode(TriangleMode _triangleMode, FillMode _fillMode, CullFace _cullFace = CullFace::Back)
		{
			triangleMode = _triangleMode;
			fillMode = _fillMode;
			cullFace = _cullFace;
		}

		template<typename T>
		void push_back_vertex(const T& data) {
			for (size_t i = 0; i < sizeof(T); i++) {
				vertices.emplace_back(reinterpret_cast<const char*>(&data)[i]);
			}
			assert(vertices.size() / sizeof(T) * sizeof(T) == vertices.size());
		}

		template<typename T>
			requires std::is_arithmetic_v<T>
		void push_back_index(T index) {
			for (size_t i = 0; i < sizeof(T); i++) {
				indices.emplace_back(reinterpret_cast<const char*>(&index)[i]);
			}
			assert(vertices.size() / sizeof(T) * sizeof(T) == vertices.size());
		}

		size_t vertexCount() {
			return vertices.size() / vertexSize;
		}

		void clear_vertices() {
			vertices.clear();
		}
		void clear_indices() {
			indices.clear();
		}

		void SetVertexData(utility::span<void> data)
		{
			vertices.resize(data.byteSize());
			std::memcpy(vertices.data(), data.begin(), data.byteSize());
		}

		void SetIndexData(utility::span<void> data, DataType _eboDataType = DataType::uint32_t)
		{
			eboDataType = _eboDataType;
			indices.resize(data.byteSize());
			std::memcpy(indices.data(), data.begin(), data.byteSize());
		}
		template<typename Vertex>
		requires HasPosition<Vertex>
		void RecalculateBounds()
		{
			math::AABB32 bounds;

			const utility::span<Vertex> vertices = this->vertices;

			for(const auto vertex:vertices)
			{
				bounds.Encapsulate(vertex.position);
			}
			boundingBox = bounds;
		}
		
		template<typename Vertex>
		requires (HasPosition<Vertex> && HasNormal<Vertex> && HasTangent<Vertex>)
		void RecalculateNormals()
		{
			utility::const_span<uint32_t> indices = this->indices;
			utility::span<Vertex> vertices = this->vertices;

			for (size_t i = 0; i < indices.size(); i += 3) {
				Vertex& v0 = vertices[indices[i]];
				Vertex& v1 = vertices[indices[i + 1]];
				Vertex& v2 = vertices[indices[i + 2]];

				glm::vec3 edge1 = v1.position - v0.position;
				glm::vec3 edge2 = v2.position - v0.position;

				glm::vec3 normal = abs(glm::cross(edge1, edge2));
				glm::vec3 tangent = glm::cross(normal, glm::vec3(normal.y, normal.x, normal.z)); // This won't be aligned with any UV map

				v0.tangent += tangent;
				v1.tangent += tangent;
				v2.tangent += tangent;

				if constexpr(HasBitangent<Vertex>)
				{
					glm::vec3 bitangent = glm::cross(normal, tangent); // Orthogonal to both
					v0.bitangent += bitangent;
					v1.bitangent += bitangent;
					v2.bitangent += bitangent;
				}
				
			}

			for (auto& vertex : vertices) {
				vertex.tangent = glm::normalize(vertex.tangent);
				if constexpr (HasBitangent<Vertex>)
				{
					vertex.bitangent = glm::normalize(vertex.bitangent);
				}
				vertex.normal = glm::normalize(vertex.normal);

				vertex.tangent = glm::normalize(vertex.tangent - vertex.normal * glm::dot(vertex.normal, vertex.tangent));
				if constexpr (HasBitangent<Vertex>)
				{
					vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
				}
			}
		}
		void Sync(StorageHint storage = StorageHint::PRESETStaticDraw)
		{
			if (vertices.size() != 0) {
				if (vertices.size() > vbo->byteSize()) {
					vbo->BufferData(vertices, storage);
				}
				else {
					vbo->BufferSubData(vertices);
				}
			}
			
			if (indices.size() != 0) {
				if (indices.size() > ebo->byteSize()) {
					ebo->BufferData(indices, storage);
				}
				else {
					ebo->BufferSubData(indices);
				}
			}
			
			
		}

		void Bind(uint32_t layoutMask)const
		{
			multiVAO.ActivateVAO(layoutMask);
			ebo->Bind();
		}


		void Draw(uint32_t layoutMask)const
		{
			if(instanceCount>1)
			{
				DrawInstanced(layoutMask, instanceCount);
			}else
			{
				DrawSingle(layoutMask);
			}
		}
		void DrawVertices( uint32_t layoutMask, uint32_t offset = 0, uint32_t count = std::numeric_limits<uint32_t>::max() )const
		{
			Bind(layoutMask);
			const GLenum drawMode = activate();
			count = glm::min<glm::uint32_t>(vbo->byteSize() / vertexSize - offset, count);

			glDrawArrays(
				drawMode,
				offset,
				count
			);
		}
		

		void DrawSingle(uint32_t layoutMask)const {
			Bind(layoutMask);
			const GLenum drawMode = activate();


			size_t indexCount = ebo->byteSize() / size_multiplier(eboDataType);
			TriangleCount() += indexCount/3;
			glDrawElements(
				drawMode,
				indexCount,
				gl_enum_cast(eboDataType),
				nullptr
			);

		}

		void DrawInstanced(uint32_t layoutMask, size_t count)const {
			Bind(layoutMask);
			const GLenum drawMode = activate();
			

			int32_t indexCount = ebo->byteSize() / size_multiplier(eboDataType);
			TriangleCount() += indexCount / 3 * count;
			glDrawElementsInstanced(
				drawMode,
				indexCount,
				gl_enum_cast(eboDataType),
				nullptr,
				count
			);

		}
		
	private:

		GLenum activate()const
		{
			GLenum drawMode = GL_TRIANGLES;
			switch (fillMode)
			{
			case FillMode::Line:
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				break;
			case FillMode::Fill:
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				break;
			}
			switch (triangleMode)
			{
			case TriangleMode::Point:
				drawMode = GL_POINTS;
				break;
			case TriangleMode::Line:
				drawMode = GL_LINES;
				break;
			case TriangleMode::Triangle:
				drawMode = GL_TRIANGLES;
				break;
			}
			switch (cullFace) {
			case CullFace::Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			case CullFace::Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;
			case CullFace::None:
				glDisable(GL_CULL_FACE);
				break;
			}
			return drawMode;
		}



		Mesh(uint64_t structSize, utility::const_span<LayoutDescriptor> _layouts) :
			eboDataType(DataType::uint32_t)
		{
			vbo = Buffer::Create(BufferTarget::ArrayBuffer);
			ebo = Buffer::Create(BufferTarget::ElementArrayBuffer);


			std::vector<VertexAttributePointer> vertexAttributeLayout;
			vertexAttributeLayout.reserve(_layouts.size());
			for (const auto& descriptor : _layouts)
			{
				vertexAttributeLayout.emplace_back(descriptor, BufferDescriptor(*vbo, 0, static_cast<int32_t>(structSize), 0));
			}
		

			multiVAO.initialize(vertexAttributeLayout);

			vertexSize = structSize;
		}
	};






















}
