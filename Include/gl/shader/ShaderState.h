#pragma once
#include "glad/glad.h"
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utility/EnumLinker.h"
#include "utility/Template.h"

namespace shader
{
    inline GLuint& getCurrentShaderProgramID()
    {
        static GLuint programID = 0;
        return programID;
    }

    inline void useShaderProgram(GLuint programID)
    {
        auto& currentID = getCurrentShaderProgramID();
        if (programID != currentID)
        {
            glUseProgram(programID);
            currentID = programID;
        }
    }
    inline void confirmCurrentProgram(GLuint programID)
    {
        GLuint current = getCurrentShaderProgramID();
	    if(current != programID)
	    {
            throw std::exception("Shader operation on wrong m_shader, activate the m_shader program before operating on it");
	    }
    }
    inline GLuint createShaderProgram()
    {
        return glCreateProgram();
    }
    inline void deleteShaderProgram(GLuint programID)
    {
        if (programID != 0)
        {
            useShaderProgram(0);
            glDeleteProgram(programID);
        }
    }

    inline GLuint CompileShader(GLenum shaderType, const std::string& source) {
        const GLuint shaderID = glCreateShader(shaderType);
        const char* sourceCStr = source.c_str();
        glShaderSource(shaderID, 1, &sourceCStr, nullptr);
        glCompileShader(shaderID);

        // Error checking
        GLint isCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> errorLog(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

            glDeleteShader(shaderID);

            std::cerr << "Shader compilation error: " << &errorLog[0] << std::endl;
            return 0;
        }

        return shaderID;
    }


   



    enum class UniformType {
        Float = 0,
        FloatVec2,
        FloatVec3,
        FloatVec4,
        Double,
        DoubleVec2,
        DoubleVec3,
        DoubleVec4,
        Int,
        IntVec2,
        IntVec3,
        IntVec4,
        UnsignedInt,
        UnsignedIntVec2,
        UnsignedIntVec3,
        UnsignedIntVec4,
        Bool,
        BoolVec2,
        BoolVec3,
        BoolVec4,
        FloatMat2,
        FloatMat3,
        FloatMat4,
        FloatMat2x3,
        FloatMat2x4,
        FloatMat3x2,
        FloatMat3x4,
        FloatMat4x2,
        FloatMat4x3,
        DoubleMat2,
        DoubleMat3,
        DoubleMat4,
        DoubleMat2x3,
        DoubleMat2x4,
        DoubleMat3x2,
        DoubleMat3x4,
        DoubleMat4x2,
        DoubleMat4x3,
        Unknown
    };


  


    struct Alignment140
    {
        GLint gl_enum;
        size_t usedSize;
        size_t stride;
        Alignment140(GLint glEnum = 0, size_t stride = 0, size_t padding = 0):
        gl_enum(glEnum), usedSize(stride-padding), stride(stride)
        {
	        
        }

        bool operator==(const Alignment140& other) const
        {
            return gl_enum == other.gl_enum;
        }
    };

    inline utility::EnumLinker<UniformType, Alignment140>& UniformTypeAlignment140() {
        static utility::EnumLinker<UniformType, Alignment140> instance{
            {UniformType::Float, {GL_FLOAT, sizeof(float)}},
            {UniformType::FloatVec2, {GL_FLOAT_VEC2, sizeof(glm::vec2)}},
            {UniformType::FloatVec3, {GL_FLOAT_VEC3, sizeof(glm::vec4), sizeof(float)}},
            {UniformType::FloatVec4, {GL_FLOAT_VEC4, sizeof(glm::vec4)}},
            {UniformType::Double, {GL_DOUBLE, sizeof(double)}},
            {UniformType::DoubleVec2, {GL_DOUBLE_VEC2, sizeof(glm::dvec2)}},
            {UniformType::DoubleVec3, {GL_DOUBLE_VEC3, sizeof(glm::dvec3)}},
            {UniformType::DoubleVec4, {GL_DOUBLE_VEC4, sizeof(glm::dvec4)}},
            {UniformType::Int, {GL_INT, sizeof(int)}},
            {UniformType::IntVec2, {GL_INT_VEC2, sizeof(glm::ivec2)}},
            {UniformType::IntVec3, {GL_INT_VEC3, sizeof(glm::ivec4), sizeof(int)}},
            {UniformType::IntVec4, {GL_INT_VEC4, sizeof(glm::ivec4)}},
            {UniformType::UnsignedInt, {GL_UNSIGNED_INT, sizeof(unsigned int)}},
            {UniformType::UnsignedIntVec2, {GL_UNSIGNED_INT_VEC2, sizeof(glm::uvec2)}},
            {UniformType::UnsignedIntVec3, {GL_UNSIGNED_INT_VEC3, sizeof(glm::uvec4), sizeof(uint32_t)}},
            {UniformType::UnsignedIntVec4, {GL_UNSIGNED_INT_VEC4, sizeof(glm::uvec4)}},
            {UniformType::Bool, {GL_BOOL, sizeof(bool)}},
            {UniformType::BoolVec2, {GL_BOOL_VEC2, sizeof(glm::bvec2)}},
            {UniformType::BoolVec3, {GL_BOOL_VEC3, sizeof(glm::bvec4)}},
            {UniformType::BoolVec4, {GL_BOOL_VEC4, sizeof(glm::bvec4)}},
            {UniformType::FloatMat2, {GL_FLOAT_MAT2, sizeof(glm::mat2)}},
            {UniformType::FloatMat3, {GL_FLOAT_MAT3, sizeof(glm::mat3)}},
            {UniformType::FloatMat4, {GL_FLOAT_MAT4, sizeof(glm::mat4)}},
            {UniformType::FloatMat2x3, {GL_FLOAT_MAT2x3, sizeof(glm::mat2x3)}},
            {UniformType::FloatMat2x4, {GL_FLOAT_MAT2x4, sizeof(glm::mat2x4)}},
            {UniformType::FloatMat3x2, {GL_FLOAT_MAT3x2, sizeof(glm::mat3x2)}},
            {UniformType::FloatMat3x4, {GL_FLOAT_MAT3x4, sizeof(glm::mat3x4)}},
            {UniformType::FloatMat4x2, {GL_FLOAT_MAT4x2, sizeof(glm::mat4x2)}},
            {UniformType::FloatMat4x3, {GL_FLOAT_MAT4x3, sizeof(glm::mat4x3)}},
            {UniformType::DoubleMat2, {GL_DOUBLE_MAT2, sizeof(glm::dmat2)}},
            {UniformType::DoubleMat3, {GL_DOUBLE_MAT3, sizeof(glm::dmat3)}},
            {UniformType::DoubleMat4, {GL_DOUBLE_MAT4, sizeof(glm::dmat4)}},
            {UniformType::DoubleMat2x3, {GL_DOUBLE_MAT2x3, sizeof(glm::dmat2x3)}},
            {UniformType::DoubleMat2x4, {GL_DOUBLE_MAT2x4, sizeof(glm::dmat2x4)}},
            {UniformType::DoubleMat3x2, {GL_DOUBLE_MAT3x2, sizeof(glm::dmat3x2)}},
            {UniformType::DoubleMat3x4, {GL_DOUBLE_MAT3x4, sizeof(glm::dmat3x4)}},
            {UniformType::DoubleMat4x2, {GL_DOUBLE_MAT4x2, sizeof(glm::dmat4x2)}},
            {UniformType::DoubleMat4x3, {GL_DOUBLE_MAT4x3, sizeof(glm::dmat4x3)}},
			{UniformType::Unknown, {0}},

        };
        return instance;
    }

    inline Alignment140 alignment_of(UniformType uniformType)
    {
        return UniformTypeAlignment140()[uniformType];
    }
    inline GLint gl_enum_cast(UniformType uniformType)
    {
        return UniformTypeAlignment140()[uniformType].gl_enum;
    }

   

    template<typename T>
    constexpr UniformType uniform_type() {
        if  (std::is_same_v<T, float_t>) {
            return UniformType::Float;
        }
        if  (std::is_same_v<T, glm::vec2>) {
            return UniformType::FloatVec2;
        }
        if  (std::is_same_v<T, glm::vec3>) {
            return UniformType::FloatVec3;
        }
        if  (std::is_same_v<T, glm::vec4>) {
            return UniformType::FloatVec4;
        }
        if  (std::is_same_v<T, int32_t>) {
            return UniformType::Int;
        }
        if  (std::is_same_v<T, glm::ivec2>) {
            return UniformType::IntVec2;
        }
        if  (std::is_same_v<T, glm::ivec3>) {
            return UniformType::IntVec3;
        }
        if  (std::is_same_v<T, glm::ivec4>) {
            return UniformType::IntVec4;
        }
        if (std::is_same_v<T, uint32_t>) {
            return UniformType::UnsignedInt;
        }
        if (std::is_same_v<T, glm::uvec2>) {
            return UniformType::UnsignedIntVec2;
        }
        if (std::is_same_v<T, glm::uvec3>) {
            return UniformType::UnsignedIntVec3;
        }
        if (std::is_same_v<T, glm::uvec4>) {
            return UniformType::UnsignedIntVec4;
        }
        if  (std::is_same_v<T, double_t>) {
            return UniformType::Double;
        }
        if  (std::is_same_v<T, glm::dvec2>) {
            return UniformType::DoubleVec2;
        }
        if  (std::is_same_v<T, glm::dvec3>) {
            return UniformType::DoubleVec3;
        }
        if  (std::is_same_v<T, glm::dvec4>) {
            return UniformType::DoubleVec4;
        }
        if  (std::is_same_v<T, glm::mat2>) {
            return UniformType::FloatMat2;
        }
        if  (std::is_same_v<T, glm::mat3>) {
            return UniformType::FloatMat3;
        }
        if  (std::is_same_v<T, glm::mat4>) {
            return UniformType::FloatMat4;
        }
        if  (std::is_same_v<T, glm::dmat2>) {
            return UniformType::DoubleMat2;
        }
        if  (std::is_same_v<T, glm::dmat3>) {
            return UniformType::DoubleMat3;
        }
        if  (std::is_same_v<T, glm::dmat4>) {
            return UniformType::DoubleMat4;
        }
       // static_assert(sizeof(T) != 0, "Unsupported uniform tokenType");
        return static_cast<UniformType>(0);
    }

    template<typename... Ts>
    constexpr std::array<UniformType, sizeof...(Ts)> uniform_types() {
        return { uniform_type<Ts>()... };
    }


    template<typename T>
    auto std140_cast(T& value)
    {
        if constexpr (std::is_same_v<T, glm::vec3>) {
            return glm::vec4(value, 1.0f);
        }
        else if constexpr (std::is_same_v<T, glm::dvec3>) {
			return glm::dvec4(value, 1.0);
		}
		else if constexpr (std::is_same_v<T, glm::ivec3>) {
			return glm::ivec4(value, 1);
		}
		else if constexpr (std::is_same_v<T, glm::uvec3>) {
			return glm::uvec4(value, 1u);
		}
		else if constexpr (std::is_same_v<T, glm::mat3x3>) {
			return glm::mat3x4(
				glm::vec4(value[0], 0.0f),
				glm::vec4(value[1], 0.0f),
				glm::vec4(value[2], 0.0f)
			);
		}
		else if constexpr (std::is_same_v<T, glm::dmat3x3>) {
				return glm::dmat3x4(
				glm::dvec4(value[0], 0.0),
				glm::dvec4(value[1], 0.0),
				glm::dvec4(value[2], 0.0)
			);
		}
		else
		return value;
    }
    template<typename T>
    auto std140_cast()
    {
        T value;
        return std140_cast<T>(value);
    }




   




}

