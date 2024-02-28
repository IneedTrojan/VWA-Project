#pragma once
#include "components/MeshFilter.h"
#include "Math/Layermask.h"
#include "gl/GLObject.h"



namespace component
{
	class MeshRenderer :public renderer::LayerMask
	{
		std::shared_ptr<shader::Material> m_material;

	public:
		MeshRenderer()
			: renderer::LayerMask(1), m_material(nullptr)
		{

		}

		std::shared_ptr<shader::Material>& GetMaterial()
		{
			return m_material;
		}

		void SetMaterial(std::shared_ptr<shader::Material> material)
		{
			m_material = std::move(material);
		}


	};


}