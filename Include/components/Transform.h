#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "IComponent.h"
#include "components/IComponent.h"

namespace component
{
	class Transform
	{
		glm::quat m_rotation;
		glm::vec3 m_scale;
		mutable glm::mat4 m_matrix;
		mutable uint32_t m_latestUpdate;
		mutable bool m_matrixValid;

		void updateMatrix()const
		{
			auto newMat = glm::mat4(1.0f);
			newMat = glm::translate(newMat, position());
			newMat *= glm::mat4_cast(m_rotation);
			m_matrix = glm::scale(newMat, m_scale);

		}

	public:
		Transform() : 
			m_rotation(1, 0, 0, 0), m_scale(1.0f), m_matrix(1.0f), m_latestUpdate(0), m_matrixValid(false)
		{

		}


		void Move(glm::vec3 localCoordinates)
		{
			position() += RotatePoint(localCoordinates);
		}


		glm::vec3 RotatePoint(glm::vec3 point) const {
			glm::quat pointQuat(0.0f, point.x, point.y, point.z);

			glm::quat rotatedPointQuat = m_rotation * pointQuat * glm::conjugate(m_rotation);

			return reinterpret_cast<const glm::vec3&>(rotatedPointQuat);
		}
		glm::vec3 ToWorldSpace(glm::vec3 point)
		{
			const glm::vec4 pos = model() * glm::vec4(point, 1.0);
			return reinterpret_cast<const glm::vec3&>(pos);
		}
		glm::vec3 ToLocalSpace(glm::vec3 point)
		{
			const glm::vec4 pos = glm::inverse(model()) * glm::vec4(point, 1.0);
			return reinterpret_cast<const glm::vec3&>(pos);
		}

		void LookAt(glm::vec3 point)
		{
			glm::vec3 forward = glm::normalize(point - position());

			glm::vec3 up(0.0f, 1.0f, 0.0f);

			glm::vec3 right = glm::cross(forward, up);

			up = glm::cross(right, forward);

			glm::mat4 rotationMatrix(1.0f);
			rotationMatrix[0] = glm::vec4(right, 0.0f);
			rotationMatrix[1] = glm::vec4(up, 0.0f);
			rotationMatrix[2] = glm::vec4(-forward, 0.0f);

			m_rotation = glm::quat_cast(rotationMatrix);
			m_latestUpdate++;
			m_matrixValid = false;
		}

		const glm::mat4& model()const
		{
			if (!m_matrixValid)
			{
				updateMatrix();
			}
			return m_matrix;
		}

		glm::vec3& position()const
		{
			m_latestUpdate++;
			m_matrixValid = false;
			return reinterpret_cast<glm::vec3&>(m_matrix[3]);
		}
		glm::vec3 forward()const
		{
			if (!m_matrixValid)
			{
				updateMatrix();
			}
			return m_matrix[0];
		}
		glm::vec3 right()const
		{
			if (!m_matrixValid)
			{
				updateMatrix();
			}
			return m_matrix[1];
		}
		glm::vec3 up()const
		{
			if (!m_matrixValid)
			{
				updateMatrix();
			}
			return m_matrix[2];
		}
		
		glm::vec3& scale()
		{
			m_latestUpdate++;
			m_matrixValid = false;
			return m_scale;
		}
		glm::quat& rotation()
		{
			m_latestUpdate++;
			m_matrixValid = false;
			return m_rotation;
		}
		uint32_t getUpdateCount()const
		{
			return m_latestUpdate;
		}

		Transform(const Transform& other) = default;
		Transform& operator=(const Transform& other) = default;
		~Transform() = default;
		Transform(Transform&& other) = delete;
		Transform& operator=(Transform&& other) = delete;
	};


}
