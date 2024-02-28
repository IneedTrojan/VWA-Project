#pragma once
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace math
{

    struct Plane {
        glm::vec3 normal;
        float d;

        Plane() : normal(0.0f), d(0.0f) {}

        Plane(const glm::vec3& n, float d) : normal(n), d(d) {}

        void normalize() {
            float mag = glm::length(normal);
            normal /= mag;
            d /= mag;
        }

        float distance(const glm::vec3& point) const {
            return glm::dot(normal, point) + d;
        }
    };

    struct CullFrustum {
        std::array<Plane, 6> planes;
        glm::mat4 vpMatrix;
        CullFrustum() = default;

        CullFrustum(const glm::mat4& _vpMatrix): vpMatrix(_vpMatrix) {
            // Left
            planes[0] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][0],
                vpMatrix[1][3] + vpMatrix[1][0],
                vpMatrix[2][3] + vpMatrix[2][0]),
                vpMatrix[3][3] + vpMatrix[3][0]);

            // Right
            planes[1] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][0],
                vpMatrix[1][3] - vpMatrix[1][0],
                vpMatrix[2][3] - vpMatrix[2][0]),
                vpMatrix[3][3] - vpMatrix[3][0]);

            // Bottom
            planes[2] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][1],
                vpMatrix[1][3] + vpMatrix[1][1],
                vpMatrix[2][3] + vpMatrix[2][1]),
                vpMatrix[3][3] + vpMatrix[3][1]);

            // Top
            planes[3] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][1],
                vpMatrix[1][3] - vpMatrix[1][1],
                vpMatrix[2][3] - vpMatrix[2][1]),
                vpMatrix[3][3] - vpMatrix[3][1]);

            // Near
            planes[4] = Plane(glm::vec3(vpMatrix[0][3] + vpMatrix[0][2],
                vpMatrix[1][3] + vpMatrix[1][2],
                vpMatrix[2][3] + vpMatrix[2][2]),
                vpMatrix[3][3] + vpMatrix[3][2]);

            // Far
            planes[5] = Plane(glm::vec3(vpMatrix[0][3] - vpMatrix[0][2],
                vpMatrix[1][3] - vpMatrix[1][2],
                vpMatrix[2][3] - vpMatrix[2][2]),
                vpMatrix[3][3] - vpMatrix[3][2]);

            // Normalize all planes
            for (auto& plane : planes) {
                plane.normalize();
            }
        }
        bool Intersects(const CullFrustum& other) const {
            for (const auto& plane : this->planes) {
                bool allOutside = true;

                for (const auto& otherPlane : other.planes) {
                    glm::vec3 pointOnOtherPlane = otherPlane.normal * -otherPlane.d;

                    if (plane.distance(pointOnOtherPlane) < 0) {
                        allOutside = false;
                        break; 
                    }
                }
                if (allOutside) return false; 
            }
            return true;
        }

        bool IsInFrustum(const glm::vec3& center, float radius) const {
            for (const auto& plane : planes) {
                if (plane.distance(center) <= -radius) {
                    return false;
                }
            }
            return true;
        }
    };
   
}
