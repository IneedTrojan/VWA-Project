#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace math
{
    template<typename T = float_t>
    struct Bounds2D
    {
        T x0;
        T y0;
        T x1;
        T y1;
        Bounds2D(const glm::vec<2, T>& min, const glm::vec<2, T>& max) :
			x0(min.x), y0(min.y), x1(max.x), y1(max.y)
        {

        }
        Bounds2D() : x0(std::numeric_limits<T>::max()), y0(std::numeric_limits<T>::max()),
            x1(std::numeric_limits<T>::min()), y1(std::numeric_limits<T>::min())
        {

        }
        Bounds2D(int32_t _x0, int32_t _y0, int32_t _x1, int32_t _y1) : x0(static_cast<T>(_x0)), y0(static_cast<T>(_y0)),
            x1(static_cast<T>(_x1)), y1(static_cast<T>(_y1))
        {

        }
        

        template<typename U = T>
        Bounds2D(T x0, T y0, T x1, T y1,
            typename std::enable_if<!std::is_same<U, int>::value>::type* = 0) : x0(x0), y0(y0), x1(x1), y1(y1)
        {

        }
       
        [[nodiscard]] bool Intersects(const Bounds2D& other) const
        {
            return x0 <= other.x1 && x1 >= other.x0 &&
                y0 <= other.y1 && y1 >= other.y0;
        }

        Bounds2D Scale(const glm::vec<2, T>& scaleFactor) const
        {
            glm::vec<2, T> center = GetCenter(); // Use the existing GetCenter method to find the center
            T widthHalf = width() * scaleFactor.x / static_cast<T>(2);
            T heightHalf = height() * scaleFactor.y / static_cast<T>(2);

            T newX0 = center.x - widthHalf;
            T newY0 = center.y - heightHalf;
            T newX1 = center.x + widthHalf;
            T newY1 = center.y + heightHalf;

            return Bounds2D(newX0, newY0, newX1, newY1);
        }

        [[nodiscard]] static Bounds2D Union(const Bounds2D& a, const Bounds2D& b)
        {
            return Bounds2D(std::min(a.x0, b.x0), std::min(a.y0, b.y0),
                std::max(a.x1, b.x1), std::max(a.y1, b.y1));
        }
        [[nodiscard]] static Bounds2D Intersection(const Bounds2D& a, const Bounds2D& b)
        {
            return Bounds2D(std::max(a.x0, b.x0), std::max(a.y0, b.y0),
                std::min(a.x1, b.x1), std::min(a.y1, b.y1));
        }
        bool Contains(const Bounds2D& other) const
        {
            return x0 <= other.x0 && x1 >= other.x1 &&
                y0 <= other.y0 && y1 >= other.y1;
        }

        [[nodiscard]] glm::vec<2, T> Overflow(const glm::vec<2, T>& point) const
        {
            return {
            	(point.x - x0) * static_cast<T>(point.x < x0) + (point.x - x1) * static_cast<T>(point.x > x1) ,
            	(point.y - y0) * static_cast<T>(point.y < y0) + (point.y - y1) * static_cast<T>(point.y > y1)
            };
        }

        [[nodiscard]] glm::bvec2 Contains2D(const glm::vec<2, T>& point) const
        {
            return { point.x >= x0 && point.x <= x1,
                point.y >= y0 && point.y <= y1 };
        }

        [[nodiscard]] bool Contains(const glm::vec<2, T>& point) const
        {
            return point.x >= x0 && point.x <= x1 &&
                point.y >= y0 && point.y <= y1;
        }

        void Encapsulate(const glm::vec<2, T>& point)
        {
            x0 = std::min(x0, point.x);
            y0 = std::min(y0, point.y);
            x1 = std::max(x1, point.x);
            y1 = std::max(y1, point.y);
        }
        void Encapsulates(const glm::vec<2, T>& point)
        {
            return point.x >= x0 && point.x <= x1
        	&& point.y >= y0 && point.y <= y1;
        }
        [[nodiscard]] glm::vec<2, T> GetCenter() const
        {
            return glm::vec<2, T>((x0 + x1) / 2, (y0 + y1) / 2);
        }

        [[nodiscard]] glm::vec<2, T> size() const
        {
            return { width(), height() };
        }
        glm::vec<2, T> fit(const glm::vec<2, T>& v)const
        {
            return { glm::clamp(v.x, x0, x1), glm::clamp(v.y, y0, y1) };
        }
        T width() const
        {
            return x1 - x0;
        }
        bool valid()const
        {
            return x0 < x1 && y0 < y1;
        }
        Bounds2D sort() {
            return Bounds2D(glm::min(x0,x1), glm::min(y0, y1), glm::max(x0, x1), glm::max(y0, y1));
        }
        Bounds2D Transpose(const glm::vec<2, T>& v)
        {
            return { v.x + x0, v.y + y0, v.x + x1, v.y + y1 };
        }
        glm::vec<2, T>& btm_l()
        {
            return *reinterpret_cast<glm::vec<2, T>*>(&x0);
        }
        glm::vec<2, T>& top_r()
        {
            return *reinterpret_cast<glm::vec<2, T>*>(&x1);
        }
        T height() const
        {
            return y1 - y0;
        }
        float GetAspect() const{
            return static_cast<float_t>(width()) / static_cast<float_t>(height());
        }
        operator bool() {
            return x0.x < x1 && y0 < y1;
        }
        operator const glm::vec<4, T>()const
        {
            return glm::vec<4, T>(x0, y0, x1, y1);
        }

        constexpr static Bounds2D Maximum() {
            return Bounds2D(static_cast<T>(std::numeric_limits<T>::min()), std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
        }
        constexpr static Bounds2D Unit() {
            return Bounds2D(static_cast<T>(0), static_cast<T>(0), static_cast<T>(1), static_cast<T>(1));
        }
        template<typename NewType>
    	Bounds2D<NewType> convert()const
        {
            return Bounds2D<NewType>{static_cast<NewType>(x0), static_cast<NewType>(y0), static_cast<NewType>(x1), static_cast<NewType>(y1)};
        }
        
    };
    using Rectangle = Bounds2D<int>;

	inline Rectangle UseBounds2DAsTexSize(Bounds2D<float> bounds, int32_t w, int32_t h)
	{
        const float scaleX = bounds.width() < 1.9f?static_cast<float>(w): 1.0f;
        const float scaleY = bounds.height() < 1.9f? static_cast<float>(h) : 1.0f;
        return Rectangle(
            static_cast<int32_t>(bounds.x0* scaleX),
            static_cast<int32_t>(bounds.y0*scaleY),
            static_cast<int32_t>(bounds.x1* scaleX),
                static_cast<int32_t>(bounds.y1*scaleY)
        );
	}


    template<typename T = float>
    struct Bounds3D {
        glm::vec<3, T> min;
        glm::vec<3, T> max;

        // Default constructor initializes bounds to maximum size
        Bounds3D() : min(std::numeric_limits<T>::max()), max(std::numeric_limits<T>::lowest()) {}

        // Constructor with initial values for 2D and 3D bounds
        Bounds3D(T x0, T y0, T z0, T x1, T y1, T z1)
            : min(x0, y0, z0), max(x1, y1, z1) {}

        // Constructor using glm::vec<3, T> for min and max
        Bounds3D(const glm::vec<3, T>& minVal, const glm::vec<3, T>& maxVal)
            : min(minVal), max(maxVal) {}

        // Check if this bounds intersects with another bounds
        bool Intersects(const Bounds3D& other) const {
            return min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z;
        }

		glm::vec<3, T> fit(const glm::vec<3, T>& in)const
        {
            return glm::min(max, glm::max(min, in));
        }

        static Bounds3D Union(const Bounds3D& a, const Bounds3D& b) {
            return Bounds3D(glm::min(a.min, b.min), glm::max(a.max, b.max));
        }

        // Checks if this bounds completely contains another bounds
        bool Contains(const Bounds3D& other) const {
            return min.x <= other.min.x && max.x >= other.max.x &&
                min.y <= other.min.y && max.y >= other.max.y &&
                min.z <= other.min.z && max.z >= other.max.z;
        }

        // Encapsulates a point into the bounds, expanding it if necessary
        void Encapsulate(const glm::vec<3, T>& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }

        // Get the center point of the bounds
        glm::vec<3, T> GetCenter() const {
            return (min + max) * static_cast<T>(0.5);
        }

        // Get the size of the bounds as a vector
        glm::vec<3, T> GetSize() const {
            return max - min;
        }

        // Check if the bounds is valid (has volume)
        bool IsValid() const {
            return min.x < max.x && min.y < max.y && min.z < max.z;
        }

        // Transform the bounds by a given matrix
        static Bounds3D Transform(const Bounds3D& bounds, const glm::mat<4, 4, T>& transform, bool dividePerspective = false) {
            std::array<glm::vec<4, T>, 8> vertices = {
                glm::vec<4, T>(bounds.min.x, bounds.min.y, bounds.min.z, 1.0f),
                glm::vec<4, T>(bounds.max.x, bounds.min.y, bounds.min.z, 1.0f),
                glm::vec<4, T>(bounds.min.x, bounds.max.y, bounds.min.z, 1.0f),
                glm::vec<4, T>(bounds.max.x, bounds.max.y, bounds.min.z, 1.0f),
                glm::vec<4, T>(bounds.min.x, bounds.min.y, bounds.max.z, 1.0f),
                glm::vec<4, T>(bounds.max.x, bounds.min.y, bounds.max.z, 1.0f),
                glm::vec<4, T>(bounds.min.x, bounds.max.y, bounds.max.z, 1.0f),
                glm::vec<4, T>(bounds.max.x, bounds.max.y, bounds.max.z, 1.0f),
            };

            glm::vec<3, T> newMin = glm::vec<3, T>(std::numeric_limits<T>::max());
            glm::vec<3, T> newMax = glm::vec<3, T>(std::numeric_limits<T>::lowest());

            for (auto& vertex : vertices) {
                auto transformedVertex = transform * vertex;
                if (dividePerspective) {
                    transformedVertex /= transformedVertex.w;
                }
                newMin = glm::min(newMin, glm::vec<3, T>(transformedVertex));
                newMax = glm::max(newMax, glm::vec<3, T>(transformedVertex));
            }

            return Bounds3D(newMin, newMax);
        }
    };



    struct AABB32
    {
    public:
        glm::vec3 min;
        glm::vec3 max;

        AABB32() : min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::min())
        {

        }
        AABB32(float _min, float _max) : min(_min), max(_max)
        {

        }
        AABB32(glm::vec3 Min, glm::vec3 Max) :
            min(Min), max(Max)
        {
        }
        bool Contains(glm::vec3 point) const
        {
            return point.x > min.x && point.y > min.y && point.z > min.z &&
                point.x < max.x&& point.y < max.y&& point.z < max.z;
        }

        bool Contains(const AABB32& other) const
        {
            return
                other.min.x >= min.x && other.min.y >= min.y && other.min.z >= min.z &&
                other.max.x <= max.x && other.max.y <= max.y && other.max.z <= max.z;
        }

        bool Intersects(const AABB32& other) const
        {
            return
                other.min.x <= max.x && other.max.x >= min.x &&
                other.min.y <= max.y && other.max.y >= min.y &&
                other.min.z <= max.z && other.max.z >= min.z;
        }
        void Encapsulate(const glm::vec3& point) {
            min = glm::min(min, point);
            max = glm::max(max, point);
        }


        static AABB32 Union(const AABB32& a, const AABB32& b)
        {
            // Calculate the union (merged) AABB of two AABBs
            glm::vec3 newMin(
                std::min(a.min.x, b.min.x),
                std::min(a.min.y, b.min.y),
                std::min(a.min.z, b.min.z)
            );

            glm::vec3 newMax(
                std::max(a.max.x, b.max.x),
                std::max(a.max.y, b.max.y),
                std::max(a.max.z, b.max.z)
            );

            return AABB32((newMin + newMax) * 0.5f, (newMax - newMin) * 0.5f);
        }
        [[nodiscard]] static AABB32 Transform(const AABB32& bounds, const glm::mat4& transformMatrix, bool dividePerspective = true)
        {
            std::array vertices{
                glm::vec4(bounds.min.x, bounds.min.y, bounds.min.z, 1.0f),
            	glm::vec4(bounds.max.x, bounds.min.y, bounds.min.z, 1.0f),
            	glm::vec4(bounds.min.x, bounds.max.y, bounds.min.z, 1.0f),
            	glm::vec4(bounds.max.x, bounds.max.y, bounds.min.z, 1.0f),
            	glm::vec4(bounds.min.x, bounds.min.y, bounds.max.z, 1.0f),
            	glm::vec4(bounds.max.x, bounds.min.y, bounds.max.z, 1.0f),
            	glm::vec4(bounds.min.x, bounds.max.y, bounds.max.z, 1.0f),
            	glm::vec4(bounds.max.x, bounds.max.y, bounds.max.z, 1.0f),
            };
            if(dividePerspective)
            {
                for (auto& vertex : vertices) {
                    vertex = transformMatrix * vertex;
                    vertex /= vertex.w;
                }
            }else
            {
                for (auto& vertex : vertices) {
                    vertex = transformMatrix * vertex;
                }
            }
            

            auto newMin = glm::vec3(vertices[0]);
            auto newMax = glm::vec3(vertices[0]);

            for (const auto& vertex : vertices) {
                newMin = glm::min(newMin, glm::vec3(vertex));
                newMax = glm::max(newMax, glm::vec3(vertex));
            }

            return AABB32{ newMin, newMax };
        }
        [[nodiscard]] static glm::mat4 GLM_Ortho(const AABB32& bounds)
        {
            return glm::ortho(bounds.min.x, bounds.max.x, bounds.min.y, bounds.max.y, bounds.max.z, bounds.min.z);
        }

		

        float GetRadius() {
            return glm::length((max - min) / 2.0f);
        }
        bool operator ==(const AABB32& other) {
            return other.min == min && other.max == max;
        }
        bool operator !=(const AABB32& other) {
            return other.min != min && other.max != max;
        }

        glm::vec3& Min() {
            return min;
        }
        glm::vec3& Max() {
            return max;
        }

        glm::vec3 GetMin()const
        {
            return min;
        }
        glm::vec3 GetMax()const
        {
            return max;
        }
        glm::vec3 GetExtends()const
        {
            return (max - min) / 2.0f;
        }
        glm::vec3 GetCenter()const
        {
            return (max + min) / 2.0f;
        }

       
        void Scale(glm::vec3 factor) {
            glm::vec3 center = GetCenter();
            glm::vec3 extends = GetExtends();

            min = center - extends * factor;
            max = center + extends * factor;
        }
    };

}