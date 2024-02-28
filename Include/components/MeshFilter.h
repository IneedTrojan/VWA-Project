#pragma once
#include "components/IComponent.h"

#include "components/Transform.h"
#include "math/BoundingBox.h"

namespace component
{
	using namespace graphics;
	using namespace math;
	class MeshFilter
	{

		std::shared_ptr<Mesh> m_mesh;
		AABB32 boundingBox;
		uint32_t transformUpdateCount;

	public:
		MeshFilter() :
			m_mesh(nullptr),
			transformUpdateCount()
		{

		}
		void SetMesh(const std::shared_ptr<Mesh>& mesh)
		{
			m_mesh = mesh;
		}

		Mesh* GetMesh()const
		{
			return m_mesh.get();
		}
		std::shared_ptr<Mesh>GetSharedMesh()const
		{
			return m_mesh;
		}

		const AABB32& GetWorldSpaceBoundingBox(const Transform& transform)
		{
			const uint32_t updateCount = transform.getUpdateCount();
			if (transformUpdateCount != updateCount)
			{
				boundingBox = AABB32::Transform(m_mesh->boundingBox, transform.model());
				transformUpdateCount = updateCount;
			}
			return boundingBox;
		}
	};
}
