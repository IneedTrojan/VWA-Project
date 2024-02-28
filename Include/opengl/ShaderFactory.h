#pragma once
#include <vector>
#include <stdint.h>
#include <utility>
#include <string_view>
#include <stdexcept>
#include <array>
#include <string>
#include <cassert>
#include <regex>
#include <unordered_map>
#include "BufferObject.h"
#include "TextureComponent.h"
#include "assets/File.h"
#include "gl/shader/ShaderState.h"
#include "utility/Pooling/SpanBatcher.h"
#include "utility/Pooling/SubUpdate.h"
#include "TSpan\\RawFinder.h"
#include <sstream>

#include "VertexArrayObject.h"
#include "utility/ranges.h"
#include "utility/ThreadPool.h"

struct jump_left
{
	size_t begin = 0;
	size_t end = 0;
};
struct jump_right
{
	size_t begin = 0;
	size_t end = 0;
};
struct index_pair
{
	size_t begin_index = 0;
	size_t end_index = 0;
	std::string_view apply(std::string_view view)const
	{
		return std::string_view(view.data()+begin_index,view.data()+ end_index);
	}
	static index_pair gap(index_pair left, index_pair right)
	{
		return { left.end(), right.begin() };
	}
	static index_pair outer_gap(index_pair left, index_pair right)
	{
		return { left.begin(), right.end() };
	}
	size_t begin()const
	{
		return begin_index;
	}
	size_t end()const
	{
		return end_index;
	}
	size_t length()const
	{
		return end_index - begin_index;
	}
	jump_left left()const
	{
		return { begin_index, end_index };
	}
	jump_right right()const
	{
		return { begin_index, end_index };
	}
	bool collides(index_pair other)const
	{
		return (begin() > other.end() && end() < other.begin())||
			(end() > other.begin() && begin() < other.end());
	}
};

struct string_operation
{
	using string_view_pair = std::pair<index_pair, std::string_view>;

	std::stringstream ss;
	std::string_view string;
	std::vector<string_view_pair> replacements;
	std::vector<index_pair> kept;

	void keep(index_pair area)
	{
		kept.push_back(area);
	}
	void remove(index_pair pos)
	{
		if (!replacements.empty() && replacements.data()->first.end() > pos.begin())
		{
			std::cerr << "overlapping string replacement\n";
		}
		replacements.emplace_back(pos, std::string_view(nullptr, 0));
	}
	void replace(index_pair pos, std::string_view area)
	{

		if(!replacements.empty() && replacements.data()->first.end()>pos.begin())
		{
			std::cerr << "overlapping string replacement\n";
		}
		replacements.emplace_back(pos, area);
	}

	void remove_kept()
	{
		if(!kept.empty())
		{
			size_t current = 0;
			std::vector<index_pair> removeList;
			for (const auto pair : kept)
			{
				auto remove = index_pair(current, pair.begin());
				removeList.emplace_back(remove);
				current = remove.end();
			}
			removeList.emplace_back(current, string.size());
			merge(removeList);
		}
	}
	void merge(const std::vector<index_pair>& subtract)
	{
		std::vector<string_view_pair> buf;
		size_t current = 0;
		for(const auto pair: subtract)
		{
			auto& replacement = replacements[current];
			if(pair.end() > replacement.first.begin())
			{
				buf.emplace_back(pair, std::string_view(nullptr, 0));
				continue;
			}
			if (!replacement.first.collides(pair))
			{
				current++;
				buf.push_back(replacement);
			}
		}
		replacements = std::move(buf);
	}

	std::string build()
	{
		size_t current = 0;
		remove_kept();

		for(const auto replacement:replacements)
		{
			ss << std::string_view(string.data() + current, replacement.first.begin());
			ss << replacement.second;
			current = replacement.first.end();
		}
		ss << std::string_view(string.data() + current, string.data()+string.size());
		std::string str = ss.str();
		string = std::string_view(str);

		return std::move(str);
	}


};

class string_search
{
	std::string_view str;
	size_t current = 0;

public:

	string_search(std::string_view view)
	{
		str = view;
	}

	
	template<typename... Functions>
		requires (...&&Template::lambda_signature<Functions, bool, char>)
	size_t search_forward(Functions... func)
	{
		size_t index = this->current;
		while (index != str.length())
		{
			char val = str[index];
			++index;
			if((... | func(val)))
			{
				return index;
			};
		}
		return str.length();
	}
	index_pair search_forward(re::Pattern& sequence, size_t& outIndex)const
	{
		outIndex = sequence.num_sequence;

		size_t index = this->current;
		while (index != str.length())
		{
			char val = str[index];
			size_t i = 0;
			for (auto& p : sequence)
			{
				if (p.Check(val))
				{
					++index;
					outIndex = i;
					return { index - p.max_matches, index };
				}
				i++;
			}
			index++;
		}
		return { str.length(), str.length() };
	}

	index_pair search_forward(re::Pattern& sequence)const
	{
		size_t index = this->current;
		while (index != str.length())
		{
			char val = str[index];
			for (auto& p : sequence)
			{
				if (p.Check(val))
				{
					++index;
					return {  index - p.max_matches, index };
				}
			}
			index++;
		}
		return {str.length(), str.length()};
	}

	size_t jump(size_t index)
	{
		current = index;
		return index;
	}
	index_pair jump(jump_left left)
	{
		current = left.begin;
		return { left.begin, left.end };
	}
	index_pair jump(jump_right left)
	{
		current = left.end;
		return { left.begin, left.end };
	}
	bool done()const
	{
		return str.length() == current;
	}
	operator bool()
	{
		return !done();
	}


private:
	

	



};




namespace shader {

	struct shaderSourceEffect {};

	struct SyntaxToken
	{
		std::string_view view;
	};

	struct name : shaderSourceEffect
	{
		name() :Name("") {}
		name(std::string str) :Name(str)
		{

		}
		name(const char* str) :Name(str)
		{

		}
		std::string Name;
	};

	struct replace : shaderSourceEffect
	{
		std::string sequence;
		std::string replacement;

		replace(const std::string& sequence, const std::string& replacement)
		{
			this->sequence = sequence;
			this->replacement = replacement;
		}

		void Apply(std::string& str)const
		{
			path::Shortcuts::replace(str, sequence, replacement);
		}

	};
	struct macro : shaderSourceEffect
	{
		macro(std::function<void(std::string*)>func)
			:apply(std::move(func))
		{
		}

		void Apply(std::string& source)
		{
			apply(&source);
		}
		std::function<void(std::string*)> apply;
	};





	struct ShaderPreprocessor
	{
		name post_fix;
		std::vector<macro> macros;
		std::vector<replace> replacements;

		void PreprocessSource(
			std::string& source,
			std::array<std::string_view, 6>& stringViews,
			std::string& outName
		)
		{
			for (auto& macro : macros)
			{
				macro.Apply(source);
			}
			for (auto& macro : replacements)
			{
				macro.Apply(source);
			}

			string_operation operation;
			operation.string = source;

			re::Pattern name_pattern = "#name ";
			re::Pattern newLine = "\n";

			for (auto search = string_search(source); search;)
			{
				index_pair enter = search.jump(
					search.search_forward(name_pattern).right()
				);
				index_pair exit = search.jump(
					search.search_forward(newLine).left()
				);
				index_pair gap = index_pair::gap(enter, exit);
				outName = gap.apply(source);

				operation.remove(index_pair::outer_gap(enter, exit));
				break;
			}
			source = operation.build();

			auto suffix_pattern = re::Pattern(
				R"RASW(#program vertex|#program fragment|#program geometry|#program tessControl|#program tessEvaluation|#program compute|\n)RASW");

			std::stringstream ss;
			std::array<index_pair, 6> shaderLocations;

			for (auto search = string_search(source); search;)
			{
				size_t shaderType;
				index_pair enter = search.jump(
					search.search_forward(suffix_pattern, shaderType).right()
				);
				index_pair exit = search.jump(
					search.search_forward(suffix_pattern).left());
				index_pair gap = index_pair::gap(enter, exit);
				if (shaderType != suffix_pattern.num_sequence)
				{
					shaderLocations[shaderType] = gap;
					ss << gap.apply(source);
				}

			}

			source = ss.str();
			for (size_t i = 0; i < 6; i++)
			{
				stringViews[i] = shaderLocations[i].apply(source);
			}
		}

		template<typename T>
		void emplace_filter(T filter)
		{
			throw std::runtime_error("invalid type");
		}
		template<>
		void emplace_filter<macro>(macro filter) {
			macros.emplace_back(filter);
		}

		template<>
		void emplace_filter<replace>(replace filter) {
			replacements.emplace_back(filter);
		}
		template<>
		void emplace_filter<name>(name filter) {
			post_fix = filter;
		}


		template<typename...Args>
		static ShaderPreprocessor Create(Args...macs)
		{
			ShaderPreprocessor preprocessor;
			(preprocessor.emplace_filter(std::forward<Args>(macs)), ...);
			return preprocessor;
		}
		ShaderPreprocessor(){

		}
	};

	 


	using namespace graphics;

	enum class ShaderType: uint32_t
	{
		Vertex = GL_VERTEX_SHADER,
		Fragment = GL_FRAGMENT_SHADER,
		Geometry = GL_GEOMETRY_SHADER,
		TessControl = GL_TESS_CONTROL_SHADER,
		TessEvaluation = GL_TESS_EVALUATION_SHADER,
		Compute = GL_COMPUTE_SHADER,
		None = 0,
	};
	enum class ShaderBit: uint32_t
	{
		Vertex = GL_VERTEX_SHADER_BIT,
		Fragment = GL_FRAGMENT_SHADER_BIT,
		Geometry = GL_GEOMETRY_SHADER_BIT,
		TessControl = GL_TESS_CONTROL_SHADER_BIT,
		TessEvaluation = GL_TESS_EVALUATION_SHADER_BIT,
		Compute = GL_COMPUTE_SHADER_BIT,
		None = 0,
	};
	ENABLE_ENUM_MATH(ShaderBit);
	ENABLE_ENUM_MATH(ShaderType);

	struct ShaderPass
	{
		
		static ShaderType GetShaderType(ShaderBit shaderType)
		{
			switch (std::bit_width(*shaderType) - 1) {
				case 0:
					return ShaderType::Vertex;
				case 1:
					return ShaderType::Fragment;
				case 2:
					return ShaderType::Geometry;
				case 3:
					return ShaderType::TessControl;
				case 4:
					return ShaderType::TessEvaluation;
				case 5:
					return ShaderType::Compute;
				default:
					return ShaderType::None;
			}
		}
		static ShaderBit GetShaderBit(ShaderType shaderType)
		{
			switch (shaderType) {
				case ShaderType::Vertex:
					return ShaderBit::Vertex;
				case ShaderType::Fragment:
					return ShaderBit::Fragment;
				case ShaderType::Geometry:
					return ShaderBit::Geometry;
				case ShaderType::TessControl:
					return ShaderBit::TessControl;
				case ShaderType::TessEvaluation:
					return ShaderBit::TessEvaluation;
				case ShaderType::Compute:
					return ShaderBit::Compute;
				default:
					return ShaderBit(0);
			}
		}

		static std::array<ShaderType, 8> GetShaderTypes(ShaderBit shaderBit)
		{
			int32_t index = 0;
			std::array<ShaderType, 8> shaderType = { ShaderType::None };

			if (*(shaderBit & ShaderBit::Vertex))
			{
				shaderType[index++] = ShaderType::Vertex;
			}
			if (*(shaderBit & ShaderBit::Fragment))
			{
				shaderType[index++] = ShaderType::Fragment;
			}
			if (*(shaderBit & ShaderBit::Geometry))
			{
				shaderType[index++] = ShaderType::Geometry;
			}
			if (*(shaderBit & ShaderBit::TessControl))
			{
				shaderType[index++] = ShaderType::TessControl;
			}
			if (*(shaderBit & ShaderBit::TessEvaluation))
			{
				shaderType[index++] = ShaderType::TessEvaluation;
			}
			if (*(shaderBit & ShaderBit::Compute))
			{
				shaderType[index] = ShaderType::Compute;
			}
			return shaderType;
		}


		static std::shared_ptr<ShaderPass> Create(ShaderType _shaderType, std::string_view _source)
		{
			auto pass = std::make_shared_for_overwrite<ShaderPass>();
			pass->shaderType = _shaderType;
			pass->ID = CompileShader(*_shaderType, _source);
			return pass;
		}

		static GLuint CompileShader(GLenum shaderType, std::string_view source) {
			const std::string finalSource(source);
			const char* ptr = finalSource.c_str();

			GLuint shaderID = glCreateShader(shaderType);
			glShaderSource(shaderID, 1, &ptr, nullptr);
			glCompileShader(shaderID);

			GLint success;
			GLchar infoLog[512];
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
				std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl<< finalSource;
				glDeleteShader(shaderID);
				return 0;
			}

			return shaderID;
		}

		ShaderType shaderType;
		GLuint ID;

	public:
		ShaderPass() :
			shaderType(ShaderType::None),
			ID(0)
		{
		}
	};

	struct TextureUniform
	{
		std::string name;
		std::shared_ptr<Texture> texture;
		int32_t unit;
		void dispose()
		{
			std::string str;
			str.swap(name);
			std::shared_ptr<Texture> tex;
			tex.swap(texture);
		}
		operator bool()const
		{
			return static_cast<bool>(texture);
		}

	};

	class ProgramObject
	{
	public:
		GLuint ID;
		std::vector<std::shared_ptr<ShaderPass>> passes;
		uint32_t PassMask;
		int32_t IsLinked = 0;

		static ProgramObject CreateStack()
		{
			return { shader::createShaderProgram() };
		}




		ProgramObject(ProgramObject&& other)noexcept
			:ID(std::exchange(other.ID, 0)),
			PassMask(other.PassMask)
		{

		}
		ProgramObject& operator=(ProgramObject&& other)noexcept
		{
			if (&other != this)
			{
				free();
				ID = std::exchange(other.ID, 0);
				PassMask = other.PassMask;
			}
			return *this;
		}

		ProgramObject(const ProgramObject& other) = delete;
		ProgramObject& operator =(const ProgramObject& other) = delete;
#define UNIFORM_FUNCTIONS
#ifdef UNIFORM_FUNCTIONS
		void Uniform1iv(int32_t loc, const int32_t* value, int32_t count = 1) const {
			Bind();
			glUniform1iv(loc, count, value);
		}
		void Uniform2iv(int32_t loc, const int32_t* value, int32_t count = 1) const {
			Bind();
			glUniform2iv(loc, count, value);
		}
		void Uniform3iv(int32_t loc, const int32_t* value, int32_t count = 1) const {
			Bind();
			glUniform3iv(loc, count, value);
		}
		void Uniform4iv(int32_t loc, const int32_t* value, int32_t count = 1) const {
			Bind();
			glUniform4iv(loc, count, value);
		}

		// Unsigned integer vector uniforms
		void Uniform1uiv(int32_t loc, const uint32_t* value, int32_t count = 1) const {
			Bind();
			glUniform1uiv(loc, count, value);
		}
		void Uniform2uiv(int32_t loc, const uint32_t* value, int32_t count = 1) const {
			Bind();
			glUniform2uiv(loc, count, value);
		}
		void Uniform3uiv(int32_t loc, const uint32_t* value, int32_t count = 1) const {
			Bind();
			glUniform3uiv(loc, count, value);
		}
		void Uniform4uiv(int32_t loc, const uint32_t* value, int32_t count = 1) const {
			Bind();
			glUniform4uiv(loc, count, value);
		}

		// Float vector uniforms
		void Uniform1fv(int32_t loc, const float_t* value, int32_t count = 1) const {
			Bind();
			glUniform1fv(loc, count, value);
		}
		void Uniform2fv(int32_t loc, const float_t* value, int32_t count = 1) const {
			Bind();
			glUniform2fv(loc, count, value);
		}
		void Uniform3fv(int32_t loc, const float_t* value, int32_t count = 1) const {
			Bind();
			glUniform3fv(loc, count, value);
		}
		void Uniform4fv(int32_t loc, const float_t* value, int32_t count = 1) const {
			Bind();
			glUniform4fv(loc, count, value);
		}

		// Double vector uniforms
		void Uniform1dv(int32_t loc, const double_t* value, int32_t count = 1) const {
			Bind();
			glUniform1dv(loc, count, value);
		}
		void Uniform2dv(int32_t loc, const double_t* value, int32_t count = 1) const {
			Bind();
			glUniform2dv(loc, count, value);
		}
		void Uniform3dv(int32_t loc, const double_t* value, int32_t count = 1) const {
			Bind();
			glUniform3dv(loc, count, value);
		}
		void Uniform4dv(int32_t loc, const double_t* value, int32_t count = 1) const {
			Bind();
			glUniform4dv(loc, count, value);
		}

		// Float matrix uniforms
		void UniformMatrix2fv(int32_t loc, int32_t count, bool transpose, const float_t* value) const {
			Bind();
			glUniformMatrix2fv(loc, count, transpose, value);
		}
		void UniformMatrix3fv(int32_t loc, int32_t count, bool transpose, const float_t* value) const {
			Bind();
			glUniformMatrix3fv(loc, count, transpose, value);
		}
		void UniformMatrix4fv(int32_t loc, int32_t count, bool transpose, const float_t* value) const {
			Bind();
			glUniformMatrix4fv(loc, count, transpose, value);
		}

		// Double matrix uniforms
		void UniformMatrix2dv(int32_t loc, int32_t count, bool transpose, const double_t* value) const {
			Bind();
			glUniformMatrix2dv(loc, count, transpose, value);
		}
		void UniformMatrix3dv(int32_t loc, int32_t count, bool transpose, const double_t* value) const {
			Bind();
			glUniformMatrix3dv(loc, count, transpose, value);
		}
		void UniformMatrix4dv(int32_t loc, int32_t count, bool transpose, const double_t* value) const {
			Bind();
			glUniformMatrix4dv(loc, count, transpose, value);
		}
#endif


		void UniformTexture(int32_t location, const graphics::Texture& texture, int32_t unit)const
		{
			texture.Bind(unit);
			Uniform1iv(location, &unit, 1);
		}






		static void BindImageTexture(
			GLuint unit,
			const graphics::TextureObject& texture,
			GLint level,
			graphics::WriteAccess access,
			graphics::TextureFormat format,
			GLboolean layered = false,
			GLint layer = 0
		) {
			glBindImageTexture(unit, texture.GetID(), level, layered, layer, static_cast<int32_t>(access), gl_enum_cast(format));
		}

		void UniformTexture(int32_t loc, int32_t unit, const TextureObject& texture, const TextureTarget target)const {
			glActiveTexture(GL_TEXTURE0 + unit);
			texture.Bind(target);
			Uniform1iv(loc, &unit);
		}

		void UniformSampler(int32_t loc, int32_t unit, const Sampler& sampler) const {
			glActiveTexture(GL_TEXTURE0 + unit);
			glBindSampler(unit, sampler.ID);
			Uniform1iv(loc, &unit);
		}


		static void UniformSSBO(int32_t unit, const BufferObject& buffer) {
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, buffer.GetID());
		}
		static void UniformSSBORange(int32_t unit, const BufferObject& buffer, int32_t offset, int32_t byteSize) {
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, unit, buffer.GetID(), offset, byteSize);
		}

		void UniformUBO(int32_t blockIndex, int32_t unit, const BufferObject& buffer)const {
			glBindBufferBase(GL_UNIFORM_BUFFER, unit, buffer.GetID());
			glUniformBlockBinding(ID, blockIndex, unit);
		}
		void UniformUBORange(int32_t blockIndex, int32_t unit, const BufferObject& buffer, int32_t offset, int32_t byteSize)const {
			glBindBufferRange(GL_UNIFORM_BUFFER, unit, buffer.GetID(), offset, byteSize);
			glUniformBlockBinding(ID, blockIndex, unit);
		}


		void Bind()const { shader::useShaderProgram(ID); }

		void free()
		{
			if (ID != 0)
			{
				shader::deleteShaderProgram(ID);
				ID = 0;
			}
		}
		~ProgramObject()
		{
			free();
		}

		void AttachShader(std::shared_ptr<ShaderPass> pass)
		{
			PassMask |= *pass->shaderType;
			glAttachShader(ID, pass->ID);
			passes.emplace_back(std::move(pass));
		}
		bool Link()
		{
			glLinkProgram(ID);

			GLint isLinked = 0;
			glGetProgramiv(ID, GL_LINK_STATUS, &isLinked);
			if (isLinked == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(std::max(maxLength,1));
				glGetProgramInfoLog(ID, maxLength, &maxLength, &infoLog[0]);

				std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog.data() << std::endl;
				 //throw std::exception("linking failed");
				// In case of a link failure, it's helpful to check if the shaders compiled successfully.
				// This step is often overlooked but can provide valuable insights into why linking failed.
				// Consider adding checks here to verify shader compilation status before linking.

				return false;
			}
			return true;
		}

		enum UniformType
		{
			UNIFORM_INT,
			UNIFORM_INT2,
			UNIFORM_INT3,
			UNIFORM_INT4,
			UNIFORM_UINT,
			UNIFORM_UINT2,
			UNIFORM_UINT3,
			UNIFORM_UINT4,
			UNIFORM_FLOAT,
			UNIFORM_FLOAT2,
			UNIFORM_FLOAT3,
			UNIFORM_FLOAT4,
			UNIFORM_DOUBLE,
			UNIFORM_DOUBLE2,
			UNIFORM_DOUBLE3,
			UNIFORM_DOUBLE4,
			UNIFORM_MAT2,
			UNIFORM_MAT3,
			UNIFORM_MAT4,
			UNIFORM_D_MAT2,
			UNIFORM_D_MAT3,
			UNIFORM_D_MAT4,
			UNIFORM_IMAGE,
			UNIFORM_SSBO,
			UNIFORM_UBO,
			UNIFORM_TEXTURE_SAMPLER,
			UNIFORM_TYPE_END
		};



		static UniformType GetUniformType(GLint uniformType)
		{
			switch (uniformType) {
			case GL_INT: return UNIFORM_INT;
			case GL_INT_VEC2: return UNIFORM_INT2;
			case GL_INT_VEC3: return UNIFORM_INT3;
			case GL_INT_VEC4: return UNIFORM_INT4;
			case GL_UNSIGNED_INT: return UNIFORM_UINT;
			case GL_UNSIGNED_INT_VEC2: return UNIFORM_UINT2;
			case GL_UNSIGNED_INT_VEC3: return UNIFORM_UINT3;
			case GL_UNSIGNED_INT_VEC4: return UNIFORM_UINT4;
			case GL_FLOAT: return UNIFORM_FLOAT;
			case GL_FLOAT_VEC2: return UNIFORM_FLOAT2;
			case GL_FLOAT_VEC3: return UNIFORM_FLOAT3;
			case GL_FLOAT_VEC4: return UNIFORM_FLOAT4;
			case GL_DOUBLE: return UNIFORM_DOUBLE;
			case GL_DOUBLE_VEC2: return UNIFORM_DOUBLE2;
			case GL_DOUBLE_VEC3: return UNIFORM_DOUBLE3;
			case GL_DOUBLE_VEC4: return UNIFORM_DOUBLE4;
			case GL_FLOAT_MAT2: return UNIFORM_MAT2;
			case GL_FLOAT_MAT3: return UNIFORM_MAT3;
			case GL_FLOAT_MAT4: return UNIFORM_MAT4;
			case GL_DOUBLE_MAT2: return UNIFORM_D_MAT2;
			case GL_DOUBLE_MAT3: return UNIFORM_D_MAT3;
			case GL_DOUBLE_MAT4: return UNIFORM_D_MAT4;

			case GL_IMAGE_1D:
			case GL_IMAGE_2D:
			case GL_IMAGE_3D:
			case GL_IMAGE_2D_RECT:
			case GL_IMAGE_CUBE:
			case GL_IMAGE_BUFFER:
			case GL_IMAGE_1D_ARRAY:
			case GL_IMAGE_2D_ARRAY:
			case GL_IMAGE_CUBE_MAP_ARRAY:
			case GL_IMAGE_2D_MULTISAMPLE:
			case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
				return UNIFORM_IMAGE;

			case GL_SHADER_STORAGE_BUFFER:
			case GL_SHADER_STORAGE_BLOCK:
				return UNIFORM_SSBO;
			case GL_UNIFORM_BLOCK:
			case GL_UNIFORM_BUFFER:
				return UNIFORM_UBO;

			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_1D_ARRAY_SHADOW:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_1D_SHADOW:
			case GL_SAMPLER_2D_SHADOW:
				return UNIFORM_TEXTURE_SAMPLER;
			default:
				return UNIFORM_TYPE_END;
			}
		}
		static int32_t GetUniformTypeSize(UniformType uniformType)
		{
			switch (uniformType) {
			case UNIFORM_INT: return sizeof(int32_t);
			case UNIFORM_INT2: return sizeof(int32_t) * 2;
			case UNIFORM_INT3: return sizeof(int32_t) * 3;
			case UNIFORM_INT4: return sizeof(int32_t) * 4;
			case UNIFORM_UINT: return sizeof(uint32_t);
			case UNIFORM_UINT2: return sizeof(uint32_t) * 2;
			case UNIFORM_UINT3: return sizeof(uint32_t) * 3;
			case UNIFORM_UINT4: return sizeof(uint32_t) * 4;
			case UNIFORM_FLOAT: return sizeof(float_t);
			case UNIFORM_FLOAT2: return sizeof(float_t) * 2;
			case UNIFORM_FLOAT3: return sizeof(float_t) * 3;
			case UNIFORM_FLOAT4: return sizeof(float_t) * 4;
			case UNIFORM_DOUBLE: return sizeof(double_t);
			case UNIFORM_DOUBLE2: return sizeof(double_t) * 2;
			case UNIFORM_DOUBLE3: return sizeof(double_t) * 3;
			case UNIFORM_DOUBLE4: return sizeof(double_t) * 4;
			case UNIFORM_MAT2: return sizeof(float_t) * 4;
			case UNIFORM_MAT3: return sizeof(float_t) * 9;
			case UNIFORM_MAT4: return sizeof(float_t) * 16;
			case UNIFORM_D_MAT2: return sizeof(double_t) * 4;
			case UNIFORM_D_MAT3: return sizeof(double_t) * 9;
			case UNIFORM_D_MAT4: return sizeof(double_t) * 16;
			case UNIFORM_IMAGE: return sizeof(std::shared_ptr<Texture>);
			case UNIFORM_SSBO: return sizeof(std::shared_ptr<graphics::Buffer>);
			case UNIFORM_UBO: return sizeof(std::shared_ptr<graphics::Buffer>);
			case UNIFORM_TEXTURE_SAMPLER: return sizeof(TextureUniform);
			case UNIFORM_TYPE_END:
			default: return 0;
			}
		}
		static glm::u64vec2 UniformDataDim(UniformType uniformType) {
			switch (uniformType) {
			case UNIFORM_INT:
			case UNIFORM_UINT:
			case UNIFORM_FLOAT:
			case UNIFORM_DOUBLE:
				return { 1,1 };

			case UNIFORM_INT2:
			case UNIFORM_UINT2:
			case UNIFORM_FLOAT2:
			case UNIFORM_DOUBLE2:
				return { 2,1 };

			case UNIFORM_INT3:
			case UNIFORM_UINT3:
			case UNIFORM_FLOAT3:
			case UNIFORM_DOUBLE3:
				return { 3,1 };

			case UNIFORM_INT4:
			case UNIFORM_UINT4:
			case UNIFORM_FLOAT4:
			case UNIFORM_DOUBLE4:
				return { 4,1 };

			case UNIFORM_MAT2:
				return { 2,2 };

			case UNIFORM_MAT3:
				return { 3,3 };

			case UNIFORM_MAT4:
				return { 4,4 };

			case UNIFORM_D_MAT2:
				return { 2,2 };

			case UNIFORM_D_MAT3:
				return { 3,3 };

			case UNIFORM_D_MAT4:
				return { 4,4 };

				// Handle other types as necessary, or provide a default case
			default:
				// Returning {0,0} or another suitable default value
				return { 1,1 };
			}
		}

		static DataType UniformDataType(UniformType uniformType)
		{
			switch (uniformType) {
			case UNIFORM_INT: case UNIFORM_INT2:case UNIFORM_INT3: case UNIFORM_INT4: return DataType::int32_t;
			case UNIFORM_UINT:case UNIFORM_UINT2:case UNIFORM_UINT3: case UNIFORM_UINT4: return DataType::uint32_t;
			case UNIFORM_FLOAT:case UNIFORM_FLOAT2: case UNIFORM_FLOAT3:case UNIFORM_FLOAT4: return DataType::float32_t;
			case UNIFORM_DOUBLE: case UNIFORM_DOUBLE2: case UNIFORM_DOUBLE3: case UNIFORM_DOUBLE4: return DataType::float64_t;
			case UNIFORM_MAT2: case UNIFORM_MAT3: case UNIFORM_MAT4: return DataType::float32_t;
			case UNIFORM_D_MAT2:case UNIFORM_D_MAT3:case UNIFORM_D_MAT4:return DataType::float64_t;
			case UNIFORM_IMAGE:
			case UNIFORM_SSBO: 
			case UNIFORM_UBO:
			case UNIFORM_TEXTURE_SAMPLER:
			case UNIFORM_TYPE_END:
			default:return DataType::float16_t;
			}
		}
		static int vector_length(UniformType uniformType) {
			switch (uniformType) {
			case UNIFORM_INT:
			case UNIFORM_UINT:
			case UNIFORM_FLOAT:
			case UNIFORM_DOUBLE:
				return 1;

			case UNIFORM_INT2:
			case UNIFORM_UINT2:
			case UNIFORM_FLOAT2:
			case UNIFORM_DOUBLE2:
				return 2;

			case UNIFORM_INT3:
			case UNIFORM_UINT3:
			case UNIFORM_FLOAT3:
			case UNIFORM_DOUBLE3:
				return 3;

			case UNIFORM_INT4:
			case UNIFORM_UINT4:
			case UNIFORM_FLOAT4:
			case UNIFORM_DOUBLE4:
				return 4;

			case UNIFORM_MAT2:
			case UNIFORM_D_MAT2:
				return 4; // 2x2 matrix

			case UNIFORM_MAT3:
			case UNIFORM_D_MAT3:
				return 9; // 3x3 matrix

			case UNIFORM_MAT4:
			case UNIFORM_D_MAT4:
				return 16; // 4x4 matrix

			case UNIFORM_IMAGE:
			case UNIFORM_SSBO:
			case UNIFORM_UBO:
			case UNIFORM_TEXTURE_SAMPLER:
			case UNIFORM_TYPE_END:
			default:
				return 0; // For types where element count doesn't apply or is unknown
			}
		}

	protected:
		ProgramObject(uint32_t shaderID) : ID(shaderID), PassMask(0)
		{
		}
	};







	struct UniformBlockLayout
	{
		struct alignas(8) Member
		{
			int32_t relativeOffset = 0;
			int32_t elementSize = 0;
			int32_t elementCount = 1;
			ProgramObject::UniformType uniformType;
		};

		std::unordered_map<std::string, Member> blockMembers;
		size_t size;

		UniformBlockLayout(size_t _layoutSize = 0) : size(_layoutSize)
		{

		}
		void emplace(std::string name, int32_t offset, int32_t elementCount, ProgramObject::UniformType uniformType)
		{
			blockMembers.emplace(std::move(name), Member(offset, ProgramObject::GetUniformTypeSize(uniformType), elementCount, uniformType));
		}

		Member find(const std::string& uniformName)const
		{
			const auto it = blockMembers.find(uniformName);
			return it == blockMembers.end() ? Member() : it->second;
		}



	};

	template<typename T>
	auto std140_cast(const T& value) {
		return value;
	}

	template<>
	inline auto std140_cast<glm::vec3>(const glm::vec3& value) {
		return glm::vec4(value, 1.0f);
	}

	template<>
	inline auto std140_cast<glm::dvec3>(const glm::dvec3& value) {
		return glm::dvec4(value, 1.0);
	}

	template<>
	inline auto std140_cast<glm::mat3>(const glm::mat3& value) {
		return glm::mat3x4(
			glm::vec4(value[0], 0.0f),
			glm::vec4(value[1], 0.0f),
			glm::vec4(value[2], 0.0f)
		);
	}
	template<>
	inline auto std140_cast<glm::dmat3>(const glm::dmat3& value) {
		return glm::mat3x4(
			glm::vec4(value[0], 0.0f),
			glm::vec4(value[1], 0.0f),
			glm::vec4(value[2], 0.0f)
		);
	}
	template<typename T>
	constexpr ProgramObject::UniformType glmUniformType() {
		if constexpr (std::is_same<T, float_t>()) {
			return ProgramObject::UNIFORM_FLOAT;
		}
		if constexpr (std::is_same<T, glm::vec2>()) {
			return ProgramObject::UNIFORM_FLOAT2;
		}
		if constexpr (std::is_same<T, glm::vec3>()) {
			return ProgramObject::UNIFORM_FLOAT3;
		}
		if constexpr (std::is_same<T, glm::vec4>()) {
			return ProgramObject::UNIFORM_FLOAT4;
		}
		if constexpr (std::is_same<T, int32_t>()) {
			return ProgramObject::UNIFORM_INT;
		}
		if constexpr (std::is_same<T, glm::ivec2>()) {
			return ProgramObject::UNIFORM_INT2;
		}
		if constexpr (std::is_same<T, glm::ivec3>()) {
			return ProgramObject::UNIFORM_INT3;
		}
		if constexpr (std::is_same<T, glm::ivec4>()) {
			return ProgramObject::UNIFORM_INT4;
		}
		if constexpr (std::is_same<T, uint32_t>()) {
			return ProgramObject::UNIFORM_UINT;
		}
		if constexpr (std::is_same<T, glm::uvec2>()) {
			return ProgramObject::UNIFORM_UINT2;
		}
		if constexpr (std::is_same<T, glm::uvec3>()) {
			return ProgramObject::UNIFORM_UINT3;
		}
		if constexpr (std::is_same<T, glm::uvec4>()) {
			return ProgramObject::UNIFORM_UINT4;
		}
		if constexpr (std::is_same<T, double_t>()) {
			return ProgramObject::UNIFORM_DOUBLE;
		}
		if constexpr (std::is_same<T, glm::dvec2>()) {
			return ProgramObject::UNIFORM_DOUBLE2;
		}
		if constexpr (std::is_same<T, glm::dvec3>()) {
			return ProgramObject::UNIFORM_DOUBLE3;
		}
		if constexpr (std::is_same<T, glm::dvec4>()) {
			return ProgramObject::UNIFORM_DOUBLE4;
		}
		if constexpr (std::is_same<T, glm::mat2>()) {
			return ProgramObject::UNIFORM_MAT2;
		}
		if constexpr (std::is_same<T, glm::mat3>()) {
			return ProgramObject::UNIFORM_MAT3;
		}
		if constexpr (std::is_same<T, glm::mat4>()) {
			return ProgramObject::UNIFORM_MAT4;
		}
		if constexpr (std::is_same<T, glm::dmat2>()) {
			return ProgramObject::UNIFORM_D_MAT2;
		}
		if constexpr (std::is_same<T, glm::dmat3>()) {
			return ProgramObject::UNIFORM_D_MAT3;
		}
		if constexpr (std::is_same<T, glm::dmat4>()) {
			return ProgramObject::UNIFORM_D_MAT4;
		}
		// Add checks for any additional types like UNIFORM_IMAGE, UNIFORM_SSBO, UNIFORM_UBO, UNIFORM_TEXTURE_SAMPLER here
		// if they correspond to specific C++ types

		// Default case or error handling can be added here
		// For now, it just returns the end_index enum value
		return ProgramObject::UNIFORM_TYPE_END;
	}
	template<typename... Args>
	constexpr std::array<ProgramObject::UniformType, (sizeof...(Args) > 0) ? sizeof...(Args) : 1> glmUniformTypesArray() {
		return { glmUniformType<Args>()... };
	}
	


	template<typename... Types>
	struct UniformBlockRange {
		using target = std::tuple<decltype(std140_cast<Types>())...>;
		using pointer = std::tuple<decltype(std140_cast<Types>())*...>;
		size_t stride = 0;
		std::tuple<decltype(std140_cast<Types>())*...> start_ptrs;
		void* end_ptr = nullptr;

		class iterator {
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = target;
			using difference_type = std::ptrdiff_t;
			using pointer = std::tuple<decltype(std140_cast<Types>())*...>;
			using reference = std::tuple<decltype(std140_cast<Types>())&...>;

			iterator(pointer current_ptrs, size_t stride)
				: current_ptrs(current_ptrs), stride(stride) {}

			reference operator*() const {
				return dereference(std::make_index_sequence<sizeof...(Types)>());
			}

			iterator& operator++() {
				increment_ptrs(std::make_index_sequence<sizeof...(Types)>());
				return *this;
			}

			bool operator==(const iterator& other) const {
				return std::get<0>(current_ptrs) == std::get<0>(other.current_ptrs);
			}

			bool operator!=(const iterator& other) const {
				return !(*this == other);
			}

		private:
			pointer current_ptrs;
			size_t stride;

			template<size_t... Is>
			reference dereference(std::index_sequence<Is...>) const {
				return { *std::get<Is>(current_ptrs)... };
			}

			template<size_t... Is>
			void increment_ptrs(std::index_sequence<Is...>) {
				(..., (std::get<Is>(current_ptrs) = reinterpret_cast<decltype(std140_cast<Types>())*>(
					reinterpret_cast<char*>(std::get<Is>(current_ptrs)) + stride)));
			}
		};

		iterator begin() {
			return iterator(start_ptrs, stride);
		}

		iterator end() {
			// Assuming end_ptr marks the end_ptr for the first type
			auto end_ptr_casted = reinterpret_cast<typename std::tuple_element<0, std::tuple<decltype(std140_cast<Types>())*...>>::type>(end_ptr);

			// Construct a tuple for the end_ptr iterator, substituting the first element with end_ptr_casted
			auto end_ptrs = std::tuple_cat(std::make_tuple(end_ptr_casted), tuple_tail(start_ptrs));

			return iterator(end_ptrs, stride);
		}

		// Helper function for tuple_tail
		template <typename Tuple, size_t... Is>
		auto tuple_tail_impl(Tuple&& t, std::index_sequence<Is...>) {
			return std::make_tuple(std::get<Is + 1>(std::forward<Tuple>(t))...);
		}

		template <typename... Args>
		auto tuple_tail(const std::tuple<Args...>& t) {
			return tuple_tail_impl(t, std::make_index_sequence<sizeof...(Args) - 1>{});
		}
	};







	struct UniformBuffer : Buffer
	{

		UniformBuffer(const UniformBlockLayout& bufferLayout, int32_t count, BufferObject&& base = BufferObject::CreateStack()) :
			Buffer(BufferTarget::UniformBuffer, std::move(base)), layout(bufferLayout)
		{
			m_linkedData.resize(bufferLayout.size * count);
			Buffer::BufferData(m_linkedData, StorageHint::PRESETDynamic);
		}
		void BufferData(utility::const_span<void> read, StorageHint bufferFlags) = delete;

		void BufferSubData(utility::const_span<void> read, int32_t offset) = delete;
		void GetBufferSubData(utility::span<void> write, int32_t offset) = delete;
		void SetTarget(BufferTarget newTarget) = delete;
		void SetBufferRange(int32_t layoutOffset, int32_t numLayouts = 1)const
		{
			Buffer::SetBufferRange(layoutOffset * stride(), numLayouts * stride());
		}

		struct UniformAccessor {
			std::string uniformName;
			size_t element;
		};

		template<typename T>
		void SetUniformV(const std::string& str, const T* values, int32_t numElements = 1, int32_t layoutOffset = 0)
		{
			const UniformBlockLayout::Member uniform = layout.find(str);
			const int32_t offset = layoutOffset * stride() + uniform.relativeOffset;
			const int32_t size = std::min(numElements, uniform.elementCount) * uniform.elementSize;

			void* write_ptr = math::pointer_add(m_linkedData.data(), offset);
			std::memcpy(write_ptr, values, size);
			m_updates.emplace(utility::SubUpdate(offset, offset + size));
		}



		template<typename...Types>
		requires (sizeof...(Types)>0)
		UniformBlockRange<Types...> range(const std::array<std::string, sizeof...(Types)>& names, size_t start = 0, size_t numElements = std::numeric_limits<size_t>::max())
		{
			numElements = std::min(count() - start, numElements);
			m_updates.emplace(utility::SubUpdate(start * stride(), (start + numElements) * stride()));


			void* begin = math::pointer_add(m_linkedData.data(), stride() * start);
			void* end = math::pointer_add(m_linkedData.data(), stride() * (start + numElements));

			UniformBlockRange<Types...> range;

			constexpr size_t numArgs = sizeof...(Types);
			std::array<void*, numArgs> offsets;
			std::array<void*, numArgs> endIterators;
			constexpr std::array<ProgramObject::UniformType, numArgs> typesControl = glmUniformTypesArray<Types...>();
			for (size_t i = 0; i < numArgs; i++)
			{
				const std::string& name = names[i];
				UniformBlockLayout::Member member = layout.find(name);

				offsets[i] = math::pointer_add(begin, member.relativeOffset);
				endIterators[i] = math::pointer_add(end, member.relativeOffset);

				assert(typesControl[i] == member.uniformType);
			}

			range.end_ptr = endIterators[0];


			range.stride = stride();
			memcpy(reinterpret_cast<void*>(&range.start_ptrs), &offsets[0], sizeof(size_t) * numArgs);


			utility::assignTupleFromArray(range.start_ptrs, offsets, std::index_sequence_for<Types...>{});


			assert(std::get<0>(range.start_ptrs) == offsets[0]);

			return range;
		}


		void Link()
		{
			for (const auto update : m_updates)
			{
				Buffer::BufferSubData(
					utility::const_span(math::pointer_add(m_linkedData.data(), update.From),
						math::pointer_add(m_linkedData.data(), update.End)),
					static_cast<uint32_t>(update.From)
				);
			}
			m_updates.reset();
		}
		void Bind()
		{
			Link();
			Buffer::Bind();
		}


		int32_t stride()const
		{
			return static_cast<int32_t>(layout.size);
		}
		int32_t count()const
		{
			return static_cast<int32_t>(m_linkedData.size() / layout.size);
		}
		int32_t size()const
		{
			return static_cast<int32_t>(m_linkedData.size());
		}


		static std::shared_ptr<UniformBuffer> Create(const UniformBlockLayout& _layout, int32_t count)
		{
			return std::make_shared<UniformBuffer>(_layout, count);
		}

	private:


		mutable utility::SpanBatcher m_updates;
		UniformBlockLayout layout;
		std::vector<char> m_linkedData;


	};
	enum BlendMode {
		SRC_COLOR = 0x0300,
		ONE_MINUS_SRC_COLOR = 0x0301,
		SRC_ALPHA = 0x0302,
		ONE_MINUS_SRC_ALPHA = 0x0303,
		DST_ALPHA = 0x0304,
		ONE_MINUS_DST_ALPHA = 0x0305,
		DST_COLOR = 0x0306,
		ONE_MINUS_DST_COLOR = 0x0307,
		SRC_ALPHA_SATURATE = 0x0308
	};



	class Program :public ProgramObject
	{
	public:
		static std::shared_ptr<Program> Create(ProgramObject&& obj = ProgramObject::CreateStack())
		{
			return std::make_shared<Program>(std::move(obj));
		}



		struct UniformBlueprint
		{
			UniformBlueprint()
			{

			}
			UniformBlueprint(std::string name, UniformType _type, int32_t _location, int32_t _count) :
				uniformName(std::move(name)), type(_type), location(_location), count(_count)
			{

			}


			std::string uniformName;
			UniformType type = UniformType::UNIFORM_TYPE_END;
			int32_t location = 0;
			int32_t count = 0;
		};


		struct BufferUniformBlueprint
		{
			BufferUniformBlueprint() : fixedBindingPoint(-1), type(UniformType::UNIFORM_TYPE_END) {}
			BufferUniformBlueprint(std::string name, int32_t bindingPoint, UniformType uinformType)
				:uniformName(std::move(name)), fixedBindingPoint(bindingPoint), type(uinformType) {}


			std::string uniformName;
			int32_t fixedBindingPoint;
			UniformType type;
		};


		struct UniformBlockBlueprint
		{
			UniformBlockBlueprint()
			{

			}
			BufferUniformBlueprint uniform;
			UniformBlockLayout layout;
			UniformBlockBlueprint(BufferUniformBlueprint&& buffer, UniformBlockLayout&& layout)
				:uniform(std::move(buffer)), layout(std::move(layout))
			{
			}
		};


		Program(ProgramObject&& obj) : ProgramObject(std::move(obj)), vertexAttributeMask(0)
		{

		}


		std::vector<BufferUniformBlueprint> queryBufferUniforms()const
		{
			std::vector<BufferUniformBlueprint> blueprints;

			int32_t numUBO;
			glGetProgramiv(ID, GL_ACTIVE_UNIFORM_BLOCKS, &numUBO);
			int32_t numSSBO;
			glGetProgramInterfaceiv(ID, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &numSSBO);

			GLenum props[2] = { GL_NAME_LENGTH, GL_BUFFER_BINDING };

			int32_t bindingPoint;
			int32_t nameLength;
			auto params = new int32_t[2];


			constexpr int32_t maxNameLength = 256;
			char characters[maxNameLength];

			for (int32_t i = 0; i < numUBO; i++)
			{
				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
				glGetActiveUniformBlockName(ID, i, nameLength, nullptr, characters);
				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_BINDING, &bindingPoint);
				blueprints.emplace_back(std::string(characters), bindingPoint, UNIFORM_UBO);
			}
			for (GLint i = 0; i < numSSBO; i++) {

				glGetProgramResourceiv(ID, GL_SHADER_STORAGE_BLOCK, i, 2, props, 2, &nameLength, params);
				nameLength = params[0];
				bindingPoint = params[1];
				glGetProgramResourceName(ID, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &nameLength, characters);
				blueprints.emplace_back(std::string(characters), bindingPoint, UNIFORM_SSBO);
			}
			return blueprints;
		}

		std::vector<UniformBlueprint> queryUniforms()const {
			std::vector<UniformBlueprint> uniforms;

			GLint uniformCount;
			glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &uniformCount);

			for (GLint i = 0; i < uniformCount; ++i) {
				constexpr GLsizei bufSize = 256;
				GLchar name[bufSize];
				GLsizei length;
				GLint size;
				GLenum type;
				glGetActiveUniform(ID, i, bufSize, &length, &size, &type, name);

				UniformBlueprint blueprint;
				blueprint.uniformName = std::string(name, length);
				blueprint.location = glGetUniformLocation(ID, name);
				blueprint.type = GetUniformType(static_cast<int32_t>(type));
				blueprint.count = size;

				uniforms.push_back(blueprint);
			}

			return uniforms;
		}


		UniformBlockLayout GetUniformBlockLayout(const std::string& name)const
		{
			const auto match = std::ranges::find_if(blockLayouts.begin(), blockLayouts.end(), [&name](const UniformBlockBlueprint& blueprint)
				{
					return blueprint.uniform.uniformName == name;
				});
			return (match == blockLayouts.end() ? UniformBlockLayout(0) : match->layout);
		}
		utility::const_span<UniformBlockBlueprint> GetUniformBlocks()const
		{
			return blockLayouts;
		}


		void Link(const std::string& name = "") {
			if(!ProgramObject::Link())
			{
				std::cout << "ERROR::LINKING FAILED " << name<<std::endl;
			}
			blockLayouts = queryUniformBlockLayouts();
			vertexAttributeMask = queryVertexAttributeMask();
		}

		uint32_t GetVertexAttributeMask()const
		{
			return vertexAttributeMask;
		}

	private:
		std::vector<UniformBlockBlueprint> queryUniformBlockLayouts()const
		{
			std::vector<UniformBlockBlueprint> uniformBlocks;

			int32_t numUBO;
			glGetProgramiv(ID, GL_ACTIVE_UNIFORM_BLOCKS, &numUBO);
			uniformBlocks.resize(numUBO);

			std::vector<int32_t> member_indices;
			std::vector<int32_t> member_offset;
			std::vector<int32_t> member_type;
			std::vector<int32_t> member_element_count;

			for (int32_t i = 0; i < numUBO; i++)
			{
				int32_t bindingPoint, nameLength, layoutSize, numMembers;
				char characters[256];

				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);
				glGetActiveUniformBlockName(ID, i, nameLength, nullptr, characters);
				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_BINDING, &bindingPoint);
				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numMembers);
				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &layoutSize);


				UniformBlockLayout layout(layoutSize);
				BufferUniformBlueprint buffer(std::string(characters), bindingPoint, UNIFORM_UBO);


				member_indices.resize(numMembers);
				member_offset.resize(numMembers);
				member_type.resize(numMembers);
				member_element_count.resize(numMembers);

				glGetActiveUniformBlockiv(ID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, member_indices.data());
				glGetActiveUniformsiv(ID, numMembers, reinterpret_cast<uint32_t*>(member_indices.data()), GL_UNIFORM_OFFSET, member_offset.data());
				glGetActiveUniformsiv(ID, numMembers, reinterpret_cast<uint32_t*>(member_indices.data()), GL_UNIFORM_TYPE, member_type.data());
				glGetActiveUniformsiv(ID, numMembers, reinterpret_cast<uint32_t*>(member_indices.data()), GL_UNIFORM_SIZE, member_element_count.data());

				for (int32_t memberIndex = 0; memberIndex < numMembers; memberIndex++)
				{
					glGetActiveUniformName(ID, member_indices[memberIndex], sizeof(characters), &nameLength, characters);
					layout.emplace(
						characters,
						member_offset[memberIndex],
						member_element_count[memberIndex],
						GetUniformType(member_type[memberIndex])
					);
				}

				member_indices.clear();
				member_offset.clear();
				member_type.clear();
				member_element_count.clear();
				UniformBlockBlueprint bp(std::move(buffer), std::move(layout));
				uniformBlocks.emplace_back(std::move(bp));
			}
			return uniformBlocks;
		}

		uint32_t queryVertexAttributeMask()const {

			GLint numAttributes;
			glGetProgramiv(ID, GL_ACTIVE_ATTRIBUTES, &numAttributes);


			uint32_t layoutMask = 0;
			for (GLint i = 0; i < numAttributes; ++i) {
				char nameBuffer[128];
				GLsizei length;
				GLint size;
				GLenum type;
				glGetActiveAttrib(ID, i, sizeof(nameBuffer), &length, &size, &type, nameBuffer);

				const GLint location = glGetAttribLocation(ID, nameBuffer);
				layoutMask |= (1 << location);
			}
			return layoutMask;
		}

		std::vector<UniformBlockBlueprint> blockLayouts;
		uint32_t vertexAttributeMask;
	};






	template<typename A, typename B>
	class un_mutable_map {
	private:
		std::unordered_map<A, int32_t> map;
		std::vector<B> Values;

	public:
		void emplace(A&& key, B&& value) {
			auto it = map.find(key);
			if (it != map.end_index()) {
				Values[it->second] = std::move(value);
			}
			else {
				map.emplace(std::move(key), Values.size());
				Values.emplace_back(std::move(value));
			}
		}

		void emplace(A&& key, const B& value) {
			auto it = map.find(key);
			if (it != map.end()) {
				Values[it->second] = value;
			}
			else {
				map.emplace(std::move(key), Values.size());
				Values.emplace_back(value);
			}
		}

		void emplace(const A& key, B&& value) {
			auto it = map.find(key);
			if (it != map.end()) {
				Values[it->second] = std::move(value);
			}
			else {
				map.emplace(key, Values.size());
				Values.emplace_back(std::move(value));
			}
		}

		void emplace(const A& key, const B& value) {
			auto it = map.find(key);
			if (it != map.end()) {
				Values[it->second] = value;
			}
			else {
				map.emplace(key, Values.size());
				Values.emplace_back(value);
			}
		}

		B* find(const A& key) {
			auto it = map.find(key);
			return it == map.end() ? nullptr : &Values[it->second];
		}

		const B* find(const A& key) const {
			auto it = map.find(key);
			return it == map.end() ? nullptr : &Values[it->second];
		}

		typename std::vector<B>::iterator begin() {
			return Values.begin_index();
		}

		typename std::vector<B>::iterator end() {
			return Values.end();
		}

		typename std::vector<B>::const_iterator begin() const {
			return Values.cbegin();
		}

		typename std::vector<B>::const_iterator end() const {
			return Values.cend();
		}

		size_t size() const {
			return Values.size();
		}


		const A& GetKey(const B* item)
		{
			int32_t distance = static_cast<int32_t>(math::pointer_distance(Values.data(), item) / sizeof(B));
			for (const auto& [key, value] : map)
			{
				if (value == distance)
				{
					return key;
				}
			}
			throw std::exception("key not found");

		}


	};



	class ComputeShader
	{
		struct Uniform
		{
			Program::UniformType uniformType;
			int32_t location;
			int32_t count;
		};
		struct BufferUniform
		{
			int32_t FixedBindingLocation;
			Program::UniformType uniformType;
		};
		


		std::shared_ptr<Program> program;
		std::unordered_map<std::string, Uniform> uniforms;
		std::unordered_map<std::string, BufferUniform> bufferUniforms;


	public:
		ComputeShader() : program(nullptr)
		{

		}

		ComputeShader(std::shared_ptr<Program> _program) : program(std::move(_program))
		{
			initialize();
		}


		void BindBufferRange(const std::string& name, const Buffer& buffer)const
		{
			const auto it = bufferUniforms.find(name);
			if (const BufferUniform* bufferInfo = it == bufferUniforms.end() ? nullptr : &it->second) {
				buffer.BindBufferRange(bufferInfo->FixedBindingLocation);
			}
		}



		void Uniform1iv(const std::string& name, const int32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform1iv(getUniformLocation(name), count, value);
		}
		void Uniform2iv(const std::string& name, const int32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform2iv(getUniformLocation(name), count, value);
		}
		void Uniform3iv(const std::string& name, const int32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform3iv(getUniformLocation(name), count, value);
		}
		void Uniform4iv(const std::string& name, const int32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform4iv(getUniformLocation(name), count, value);
		}

		// Unsigned integer vector uniforms
		void Uniform1uiv(const std::string& name, const uint32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform1uiv(getUniformLocation(name), count, value);
		}
		void Uniform2uiv(const std::string& name, const uint32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform2uiv(getUniformLocation(name), count, value);
		}
		void Uniform3uiv(const std::string& name, const uint32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform3uiv(getUniformLocation(name), count, value);
		}
		void Uniform4uiv(const std::string& name, const uint32_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform4uiv(getUniformLocation(name), count, value);
		}

		// Float vector uniforms
		void Uniform1fv(const std::string& name, const float_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform1fv(getUniformLocation(name), count, value);
		}
		void Uniform2fv(const std::string& name, const float_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform2fv(getUniformLocation(name), count, value);
		}
		void Uniform3fv(const std::string& name, const float_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform3fv(getUniformLocation(name), count, value);
		}
		void Uniform4fv(const std::string& name, const float_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform4fv(getUniformLocation(name), count, value);
		}

		// Double vector uniforms
		void Uniform1dv(const std::string& name, const double_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform1dv(getUniformLocation(name), count, value);
		}
		void Uniform2dv(const std::string& name, const double_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform2dv(getUniformLocation(name), count, value);
		}
		void Uniform3dv(const std::string& name, const double_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform3dv(getUniformLocation(name), count, value);
		}
		void Uniform4dv(const std::string& name, const double_t* value, int32_t count = 1) const {
			program->Bind();
			glUniform4dv(getUniformLocation(name), count, value);
		}

		// Float matrix uniforms
		void UniformMatrix2fv(const std::string& name, int32_t count, bool transpose, const float_t* value) const {
			program->Bind();
			glUniformMatrix2fv(getUniformLocation(name), count, transpose, value);
		}
		void UniformMatrix3fv(const std::string& name, int32_t count, bool transpose, const float_t* value) const {
			program->Bind();
			glUniformMatrix3fv(getUniformLocation(name), count, transpose, value);
		}
		void UniformMatrix4fv(const std::string& name, int32_t count, bool transpose, const float_t* value) const {
			program->Bind();
			glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
		}

		// Double matrix uniforms
		void UniformMatrix2dv(const std::string& name, int32_t count, bool transpose, const double_t* value) const {
			program->Bind();
			glUniformMatrix2dv(getUniformLocation(name), count, transpose, value);
		}
		void UniformMatrix3dv(const std::string& name, int32_t count, bool transpose, const double_t* value) const {
			program->Bind();
			glUniformMatrix3dv(getUniformLocation(name), count, transpose, value);
		}
		void UniformMatrix4dv(const std::string& name, int32_t count, bool transpose, const double_t* value) const {
			program->Bind();
			glUniformMatrix4dv(getUniformLocation(name), count, transpose, value);
		}

		void BindSamplerTexture(const std::string& name, const Texture& texture, int32_t unit)const
		{
			texture.Bind(unit);
			Uniform1iv(name, &unit, 1);
		}



		int32_t getUniformLocation(const std::string& name)const
		{
			const auto it = uniforms.find(name);
			return it == uniforms.end() ? -1 : it->second.location;
		}

		void DispatchCompute(int32_t x, int32_t y, int32_t z)const
		{
			const glm::uvec3 groupSize = { x,y,z };
			program->Bind();
			Uniform3uiv("dispatchSize", &groupSize.x, 1);
			glDispatchCompute(x, y, z);
		}

		const std::shared_ptr<Program>& GetProgram()const
		{
			return program;
		}
		void Bind()const
		{
			program->Bind();
		}

	private:
		void initialize()
		{
			const auto uniformBlueprints = program->queryUniforms();
			const auto buffers = program->queryBufferUniforms();
			for (const auto& uniform : uniformBlueprints)
			{
				const auto type = uniform.type;
				uniforms.emplace(uniform.uniformName, Uniform(type, uniform.location, uniform.count));
			}
			for (const auto& uniformBuffer : buffers)
			{
				const auto type = uniformBuffer.type;
				bufferUniforms.emplace(uniformBuffer.uniformName, BufferUniform(uniformBuffer.fixedBindingPoint, type));
			}

			for (const auto& [name, info] : uniforms)
			{
				std::cout << "UniformName __" << name << "__" << std::endl;
				std::cout << "UniformType __" << info.uniformType << "__" << std::endl;
				std::cout << "Location __" << info.location << "__" << std::endl;
				std::cout << "UniformCount __" << info.count << "__" << std::endl;
				std::cout << std::endl;
			}
			std::cout << std::endl;

			for (const auto& [name, info] : bufferUniforms)
			{
				std::cout << "BufferName __" << name << "__" << std::endl;
				std::cout << "UniformType __" << info.uniformType << "__" << std::endl;
				std::cout << "BindingPoint __" << info.FixedBindingLocation << "__" << std::endl;
				std::cout << std::endl;
			}
		}
	};
	template<typename T>
	concept isInt = LayoutDescriptor::IsDataTypeOrUnderlying<T>(
		DataType::uint8_t, DataType::uint16_t, DataType::uint32_t, DataType::uint64_t,
		DataType::int8_t, DataType::int16_t, DataType::int32_t, DataType::int64_t);

	template<typename T>
	concept isFloat = LayoutDescriptor::IsDataTypeOrUnderlying<T>(
		DataType::float16_t, DataType::float32_t, DataType::float64_t
	);
	template<typename T>
	concept isMatrix = !std::is_same_v<graphics::matrix_underlying_prim_type<T>, std::false_type>;

	class Material
	{
		ComputeShader directAccess;

		struct Property
		{
			Property() : uniformType(Program::UNIFORM_TYPE_END), numElements(0), binding_location(0), data(nullptr)
			{

			}
			Program::UniformType uniformType;
			int32_t numElements;
			int32_t binding_location;
			bool IsActive = false;

			mutable void* data;


			int32_t binding()const
			{
				return binding_location;
			}
			int32_t getLocation()const
			{
				return binding_location;
			}
			~Property()
			{
				if (data != nullptr)
				{
					switch (uniformType)
					{
					case Program::UNIFORM_INT:
					case Program::UNIFORM_INT2:
					case Program::UNIFORM_INT3:
					case Program::UNIFORM_INT4:
					case Program::UNIFORM_UINT:
					case Program::UNIFORM_UINT2:
					case Program::UNIFORM_UINT3:
					case Program::UNIFORM_UINT4:
					case Program::UNIFORM_FLOAT:
					case Program::UNIFORM_FLOAT2:
					case Program::UNIFORM_FLOAT3:
					case Program::UNIFORM_FLOAT4:
					case Program::UNIFORM_DOUBLE:
					case Program::UNIFORM_DOUBLE2:
					case Program::UNIFORM_DOUBLE3:
					case Program::UNIFORM_DOUBLE4:
					case Program::UNIFORM_MAT2:
					case Program::UNIFORM_MAT3:
					case Program::UNIFORM_MAT4:
					case Program::UNIFORM_D_MAT2:
					case Program::UNIFORM_D_MAT3:
					case Program::UNIFORM_D_MAT4:
						break;
					case Program::UNIFORM_IMAGE:
					case Program::UNIFORM_TEXTURE_SAMPLER:
						static_cast<TextureUniform*>(data)->dispose();
						break;
					case Program::UNIFORM_SSBO:
					case Program::UNIFORM_UBO:
						static_cast<std::shared_ptr<Buffer>*>(data)->reset();
						break;
					case Program::UNIFORM_TYPE_END:
						break;
					}
				}

			}


			Property(Property&& other) noexcept
				: uniformType(other.uniformType), numElements(other.numElements),
				binding_location(other.binding_location), // Just choose one member of the union
				IsActive(other.IsActive),
				data(other.data)
			{

				other.data = nullptr;
			}

			Property& operator=(Property&& other) noexcept {
				if (this != &other) {
					// Clean up the current resources

					IsActive = other.IsActive;
					uniformType = other.uniformType;
					numElements = other.numElements;
					binding_location = other.binding_location; // Just choose one member of the union
					data = other.data;


					other.data = nullptr;
					other.uniformType = Program::UNIFORM_TYPE_END;
					other.numElements = 0;
				}
				return *this;
			}


			template<typename T>
			T& cast()const
			{
				return *static_cast<T*>(data);
			}

		};

		un_mutable_map<std::string, Property> properties;

		mutable std::vector<TextureBatcher::TextureBindParameters> textureBinderParams;
		std::vector<TextureUniform*> textureUniforms;
		std::vector<std::shared_ptr<Buffer>*>buffers;



		std::vector<char> propertyData;



	public:
		Material(std::shared_ptr<Program> _program) : directAccess(std::move(_program))
		{
			const auto uniforms = directAccess.GetProgram()->queryUniforms();
			const auto bufferUniforms = directAccess.GetProgram()->queryBufferUniforms();

			int32_t size = 0;
			for (const auto& uniform : uniforms) {
				size += Program::GetUniformTypeSize(uniform.type) * uniform.count;
			}
			for (const auto& buffer : bufferUniforms) {
				size += Program::GetUniformTypeSize(buffer.type);
			}

			propertyData.resize(size);


			void* offset = propertyData.data();
			for (const auto& uniform : uniforms)
			{
				const int32_t uniformSize = Program::GetUniformTypeSize(uniform.type) * uniform.count;
				Property property;

				property.data = offset;
				property.numElements = uniform.count;
				property.uniformType = uniform.type;
				property.binding_location = uniform.location;
				property.IsActive = uniform.location >= 0;

				if(uniform.type == ProgramObject::UNIFORM_TEXTURE_SAMPLER)
				{
					textureUniforms.push_back(static_cast<TextureUniform*>(property.data));
				}

				properties.emplace(uniform.uniformName, std::move(property));
				offset = math::pointer_add(offset, uniformSize);
			}
			for (const auto& buffer : bufferUniforms)
			{
				const int32_t uniformSize = Program::GetUniformTypeSize(buffer.type);
				Property property;

				property.data = offset;
				property.numElements = 1;
				property.uniformType = buffer.type;
				property.binding_location = buffer.fixedBindingPoint;
				property.IsActive = buffer.fixedBindingPoint >= 0;

				properties.emplace(buffer.uniformName, std::move(property));
				offset = math::pointer_add(offset, uniformSize);
			}
		}



		template<typename W, typename C>
		void assign(W* dst,const C* src, size_t count)
		{
			for(size_t i = 0;i<count;i++)
			{
				dst[i] = static_cast<W>(src[i]);
			}
		}

		template<typename Func>
		static void datatype_invoke(DataType type, Func&& lambda) {
			switch (type) {
			case DataType::uint8_t: lambda.template operator() < std::uint8_t > (); break;
			case DataType::int8_t: lambda.template operator() < std::int8_t > (); break;
			case DataType::uint16_t: lambda.template operator() < std::uint16_t > (); break;
			case DataType::int16_t: lambda.template operator() < std::int16_t > (); break;
			case DataType::uint32_t: lambda.template operator() < std::uint32_t > (); break;
			case DataType::int32_t: lambda.template operator() < std::int32_t > (); break;
			case DataType::uint64_t: lambda.template operator() < std::uint64_t > (); break;
			case DataType::int64_t: lambda.template operator() < std::int64_t > (); break;
			case DataType::float32_t: lambda.template operator() < float > (); break;
			case DataType::float64_t: lambda.template operator() < double > (); break;
			default: break;
			}
		}


		template<typename T>
			requires isInt<T>
		void SetInt(const std::string& name, const T& v, int32_t n = 1)
		{
			using underlying = vector_underlying_prim_type<int>;
			constexpr int32_t vectorLength = LayoutDescriptor::vector_length<T>();
			if(const Property * property = getProperty(name))
			{
				const int32_t property_vector_length = ProgramObject::vector_length(property->uniformType);
				void* dst = property->data;
				const underlying* src = reinterpret_cast<const underlying*>(&v);
				size_t count = static_cast<size_t>(std::min(vectorLength, property_vector_length) *
					std::min(n, property->numElements));

				datatype_invoke(ProgramObject::UniformDataType(property->uniformType), [&]<typename W>(){
					assign(static_cast<W*>(dst), src, count);
				});
			}
		}

		template<typename T>
			requires isFloat<T>
		void SetFloat(const std::string& name, const T& v, int32_t n = 1)
		{
			using underlying = vector_underlying_prim_type<T>;
			constexpr int32_t vectorLength = LayoutDescriptor::vector_length<T>();
			if (const Property* property = getProperty(name))
			{
				const int32_t property_vector_length = ProgramObject::vector_length(property->uniformType);
				void* dst = property->data;
				const underlying* src = reinterpret_cast<const underlying*>(&v);
				size_t count = static_cast<size_t>(std::min(vectorLength, property_vector_length) *
					std::min(n, property->numElements));

				datatype_invoke(ProgramObject::UniformDataType(property->uniformType), [&]<typename W>() {
					assign(static_cast<W*>(dst), src, count);
				});
			}
		}

		template<typename T>
			requires isMatrix<T>
		void SetMatrix(const std::string& name, const T& v, size_t n = 1)
		{
			constexpr glm::u64vec2 dim = LayoutDescriptor::matrix_dim<T>();
			using underlying = matrix_underlying_prim_type<T>;

			if (const Property* property = getProperty(name))
			{
				void* dst = property->data;

				const underlying* src = reinterpret_cast<const underlying*>(&v);

				auto dstDim = ProgramObject::UniformDataDim(property->uniformType);
				for(size_t i = 0;i<std::min(n, static_cast<size_t>(property->numElements));i++)
				{
					for (size_t row = 0; row < glm::min(dim.y, dstDim.y); row++)
					{
						datatype_invoke(ProgramObject::UniformDataType(property->uniformType), [&]<typename W>() {

							W* _dst = static_cast<W*>(dst);
							assign(_dst, src, dstDim.x);
							src += dim.x;
							dst = reinterpret_cast<void*>(_dst + dstDim.x);
						});
					}
				}
				
			}
		}
		
		void BindValue(const std::string& name, const void* values, int32_t count = 1)const
		{
			if (const Property* property = getProperty(name))
			{
				std::memcpy(
					property->data,
					values, static_cast<size_t>(
						std::min(count, property->numElements) *
						Program::GetUniformTypeSize(property->uniformType))
				);
			}
		}
		void BindTexture(const std::string& name, std::shared_ptr<Texture> texture)const
		{
			if (const Property* property = getProperty(name))
			{
				property->cast<TextureUniform>().texture = std::move(texture);
			}
			RefreshTextures();
		}
		void BindBuffer(const std::string& name, std::shared_ptr<Buffer> buffer)const
		{
			if (const Property* property = getProperty(name))
			{
				property->cast<std::shared_ptr<Buffer>>() = std::move(buffer);
			}
		}
		void Activate()const
		{
			for (const auto& property : properties)
			{
				if (property.IsActive)
				{
					ActivateProperty(property);
				}
			}
		}
		void UseTexturesManually()const
		{
			int32_t unit = 0;
			for(const auto texture: textureBinderParams)
			{
				glBindSamplers(unit, 1, reinterpret_cast<const uint32_t*>(&texture.SamplerID));
				glBindTextures(unit++, 1, reinterpret_cast<const uint32_t*>(&texture.TextureID));
			}
		}
		const ComputeShader& DirectShaderAccess()const
		{
			return directAccess;
		}


		const std::shared_ptr<UniformBuffer>& GetUniformBuffer(const std::string& name)
		{
			std::shared_ptr<UniformBuffer>* buffer = nullptr;
			if (const Property* property = getProperty(name))
			{
				buffer = &property->cast<std::shared_ptr<UniformBuffer>>();
				if (!*buffer) {
					resetUniformBuffer(property);
				}
			}
			return *buffer;
		}


		TextureBatch& UseTextures()const
		{
			TextureBatch& batch = TextureBatcher::BindTextures(textureBinderParams);
			for (size_t i = 0;i<textureBinderParams.size();i++)
			{
				const auto& texture = textureUniforms[i];
				texture->unit = textureBinderParams[i].outUnit;
			}
			return batch;
		}


	private:

		void RefreshTextures()const
		{
			textureBinderParams.clear();
			for (const auto textureUniform : textureUniforms)
			{
				if (const Texture* txt = textureUniform->texture.get())
				{
					const Sampler* sampler = txt->GetSampler();

					uint32_t textureID = txt ? txt->GetBase().GetID() : 0;
					uint32_t samplerID = txt && sampler ? sampler->ID : 0;
					textureBinderParams.emplace_back(textureID, samplerID, 0);
				}
			}
		}

		void resetUniformBuffer(const Property* property)
		{
			auto& buffer = property->cast<std::shared_ptr<UniformBuffer>>();
			const std::string& name = properties.GetKey(property);
			buffer = UniformBuffer::Create(directAccess.GetProgram()->GetUniformBlockLayout(name), 1);
		}

		const Property* getProperty(const std::string& name)const
		{
			return properties.find(name);
		}
		void ActivateProperty(const Property& property)const
		{
			switch (property.uniformType)
			{
			case Program::UNIFORM_INT:
				getProgram()->Uniform1iv(property.getLocation(), &property.cast<int32_t>(), property.numElements);
				break;
			case Program::UNIFORM_INT2:
				getProgram()->Uniform2iv(property.getLocation(), &property.cast<int32_t>(), property.numElements);
				break;
			case Program::UNIFORM_INT3:
				getProgram()->Uniform3iv(property.getLocation(), &property.cast<int32_t>(), property.numElements);
				break;
			case Program::UNIFORM_INT4:
				getProgram()->Uniform4iv(property.getLocation(), &property.cast<int32_t>(), property.numElements);
				break;
			case Program::UNIFORM_UINT:
				getProgram()->Uniform1uiv(property.getLocation(), &property.cast<uint32_t>(), property.numElements);
				break;
			case Program::UNIFORM_UINT2:
				getProgram()->Uniform2uiv(property.getLocation(), &property.cast<uint32_t>(), property.numElements);
				break;
			case Program::UNIFORM_UINT3:
				getProgram()->Uniform3uiv(property.getLocation(), &property.cast<uint32_t>(), property.numElements);
				break;
			case Program::UNIFORM_UINT4:
				getProgram()->Uniform4uiv(property.getLocation(), &property.cast<uint32_t>(), property.numElements);
				break;
			case Program::UNIFORM_FLOAT:
				getProgram()->Uniform1fv(property.getLocation(), &property.cast<float>(), property.numElements);
				break;
			case Program::UNIFORM_FLOAT2:
				getProgram()->Uniform2fv(property.getLocation(), &property.cast<float>(), property.numElements);
				break;
			case Program::UNIFORM_FLOAT3:
				getProgram()->Uniform3fv(property.getLocation(), &property.cast<float>(), property.numElements);
				break;
			case Program::UNIFORM_FLOAT4:
				getProgram()->Uniform4fv(property.getLocation(), &property.cast<float>(), property.numElements);
				break;
			case Program::UNIFORM_DOUBLE:
				getProgram()->Uniform1dv(property.getLocation(), &property.cast<double>(), property.numElements);
				break;
			case Program::UNIFORM_DOUBLE2:
				getProgram()->Uniform2dv(property.getLocation(), &property.cast<double>(), property.numElements);
				break;
			case Program::UNIFORM_DOUBLE3:
				getProgram()->Uniform3dv(property.getLocation(), &property.cast<double>(), property.numElements);
				break;
			case Program::UNIFORM_DOUBLE4:
				getProgram()->Uniform4dv(property.getLocation(), &property.cast<double>(), property.numElements);
				break;
			case Program::UNIFORM_MAT2:
				getProgram()->UniformMatrix2fv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<float>());
				break;
			case Program::UNIFORM_MAT3:
				getProgram()->UniformMatrix3fv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<float>());
				break;
			case Program::UNIFORM_MAT4:
				getProgram()->UniformMatrix4fv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<float>());
				break;
			case Program::UNIFORM_D_MAT2:
				getProgram()->UniformMatrix2dv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<double>());
				break;
			case Program::UNIFORM_D_MAT3:
				getProgram()->UniformMatrix3dv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<double>());
				break;
			case Program::UNIFORM_D_MAT4:
				getProgram()->UniformMatrix4dv(property.getLocation(), property.numElements, GL_FALSE, &property.cast<double>());
				break;

			case Program::UNIFORM_IMAGE:
				break;
			case Program::UNIFORM_TEXTURE_SAMPLER:
				/*if (const TextureUniform& view = property.cast<TextureUniform>()) {
					
					getProgram()->Uniform1iv(property.getLocation(), &view.unit, 1);
				}*/
				break;
			case Program::UNIFORM_SSBO:
				if (const auto& ptr = property.cast<std::shared_ptr<Buffer>>()) {
					ptr->BindBufferRange(property.getLocation());
				}
				break;
			case Program::UNIFORM_UBO:
				if (const auto& ptr = property.cast<std::shared_ptr<Buffer>>()) {
					ptr->BindBufferRange(property.getLocation());
				}
				break;
			case Program::UNIFORM_TYPE_END:
			default:
				break;
			}
		}


		const Program* getProgram()const
		{
			return directAccess.GetProgram().get();
		}

	};


	struct ProgramRequest
	{
		struct Task
		{
			ShaderType type;
			std::string ProjectPath;
		};
		ProgramRequest() = default;
		ProgramRequest(std::initializer_list<Task> tasks) : task(tasks)
		{
			sort();
		}
		std::vector<Task> task;

		void emplace(ShaderType shaderType, const std::string& str)
		{
			task.emplace_back(shaderType, str);
		}
		void sort()
		{
			std::ranges::sort(task.begin(), task.end(), [](const Task& a, const Task& b) {return a.type < b.type; });
		}

		std::string ProgramName()const
		{
			std::stringstream ss;
			for (auto& tsk : task)
			{
				ss << tsk.ProjectPath;
			}
			return ss.str();
		}
	};
	
	template <typename... Args>
	struct ArePairs;

	template <typename T1, typename T2, typename... Rest>
	struct ArePairs<T1, T2, Rest...> {
		static constexpr bool value =
			(std::is_constructible_v<ShaderType, T1> && std::is_constructible_v<std::string, T2>) &&
			ArePairs<Rest...>::value;
	};

	template <>
	struct ArePairs<> {
		static constexpr bool value = true;
	};

	template <typename... Args>
	concept IsPairConcept = ArePairs<Args...>::value;


	class ShaderFactory
	{
		std::unordered_map<std::string, std::weak_ptr<Program>> shaderPrograms;
		std::unordered_map<std::string, std::weak_ptr<ShaderPass>> shaderPasses;



	public:

		static void emplace(ProgramRequest& request)
		{

		}
		template<typename T1, typename T2, typename... Args>
		static void emplace(ProgramRequest& request, T1& a, T2& b, Args... args)
		{
			request.emplace(a, b);
			emplace(request, args...);
		}


		template<typename...Args>
			requires IsPairConcept<Args...>
		static std::shared_ptr<Program> MakeProgram(Args...args)
		{
			ProgramRequest request;
			emplace(request, args...);
			request.sort();
			return instance().GetOrCreateProgram(request);
		}




		std::unordered_map<std::string, std::weak_ptr<Program>> shaders;


		static std::shared_ptr<Program> newCreateShader(const std::string& identifier, std::array<std::string_view, 6>& sources)
		{
			auto it = instance().shaders.find(identifier);
			if (it == instance().shaders.end() || it->second.expired())
			{
				std::shared_ptr<Program> program = Program::Create();
				it = instance().shaders.emplace(identifier, program).first;
				for (size_t i = 0; i < 6; i++)
				{
					if (!sources[i].empty())
					{
						ShaderType type = ShaderPass::GetShaderType(ShaderBit(1 << i));
						program->AttachShader(
							ShaderPass::Create(type, sources[i])
						);
					}
				}
				program->Link(identifier);
			}
			return it->second.lock();
		}



		template<typename...Args>
			requires (...&& std::is_base_of_v<shaderSourceEffect, Args>)
		static std::shared_ptr<Program> NewLoadShader(std::string path, Args...args)
		{
			path::Shortcuts::Apply(path);
			auto content = asset::GetFileContent(path);
			std::string source(content.begin(), content.end());
			std::array<std::string_view, 6> passes;
			std::string suffix;

			ShaderPreprocessor preprocessor = ShaderPreprocessor::Create(args...);
			preprocessor.PreprocessSource(source, passes, suffix);
			std::cout << "LOad\n" << std::string(content.begin(), content.end()) << "\n->\n" << source << std::endl;
			return newCreateShader(suffix, passes);
		}







		
		template<typename... Args>
			requires IsPairConcept<Args...>
		static std::shared_ptr<ComputeShader> MakeShader(Args&&... args)
		{
			return std::make_shared<ComputeShader>(MakeProgram(std::forward<Args>(args)...)); // Use std::forward here
		}

		static std::shared_ptr<ComputeShader> MakeComputeShader(const std::string relativePath)
		{
			return std::make_shared<ComputeShader>(MakeProgram(ShaderType::Compute, relativePath));
		}

		template<typename... Args>
			requires IsPairConcept<Args...>
		static std::shared_ptr<Material> MakeMaterial(Args&&... args)
		{
			std::shared_ptr<Program> program = MakeProgram(std::forward<Args>(args)...); // Use std::forward here
			return std::make_shared<Material>(program);
		}
		void test()
		{
			auto material = MakeMaterial(ShaderType::Vertex, "$Shader\\Skybox.vert", ShaderType::Fragment, "$Shader\\Skybox.frag");
		}


	private:
		std::shared_ptr<Program> GetOrCreateProgram(const ProgramRequest& request)
		{
			const std::string name = request.ProgramName();
			auto it = shaderPrograms.find(name);
			std::shared_ptr<Program> program = (it == shaderPrograms.end() || it->second.expired()) ?
				nullptr : it->second.lock();

			if (!program)
			{
				program = Program::Create();
				for (const auto& task : request.task)
				{
					program->AttachShader(GetOrCreateShaderPass(task));
				}
				program->Link(name);
				shaderPrograms[name] = program;
			}

			return program;
		}
		std::shared_ptr<ShaderPass> GetOrCreateShaderPass(const ProgramRequest::Task& task)
		{
			std::string name = task.ProjectPath;
			auto it = shaderPasses.find(name);
			std::shared_ptr<ShaderPass> shader = (it == shaderPasses.end() || it->second.expired()) ?
				nullptr : it->second.lock();
			if (!shader)
			{
				path::Shortcuts::Apply(name);
				auto shaderContent = asset::GetFileContent(name);
				assert(!shaderContent.empty());
				shader = ShaderPass::Create(task.type,
					{ shaderContent.data(),shaderContent.data() + shaderContent.size() });

				shaderPasses[name] = shader;
			}
			return shader;

		}

		static ShaderFactory& instance()
		{
			static ShaderFactory instance;
			return instance;
		}
	};
};