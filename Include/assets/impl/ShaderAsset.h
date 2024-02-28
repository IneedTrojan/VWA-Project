#pragma once
#include <memory>
#include <string>

#include "assets/Asset.h"

#if 0
namespace asset
{

	class ShaderAsset: public asset::Asset
	{
	public:
		ShaderAsset(std::string _name, std::string filePath = "")
			:Asset(std::move(filePath)), m_name(std::move(_name))
		{
			
		}
		void SetProgram(shader::ShaderType shaderType,const std::string& _relativePath)
		{
			auto it = std::ranges::find_if(m_shaders.begin_index(), m_shaders.end_index(),
				[shaderType](const ShaderFile& file){
					return file.tokenType == shaderType;
				});
			if(it != m_shaders.end_index())
			{
				m_shaders.erase(it);
			}
			AssetBackup file;
			file.Create(_relativePath);
			m_shaders.emplace_back(shaderType, std::move(file));
		}

		[[nodiscard]]std::weak_ptr<shader::ShaderClass> SingletonCompile()const
		{
			return shader::ShaderRegistry::SingletonGetShader(m_name ,m_shaders);
		}
		
		std::vector<char> Serialize() const override
		{
			nlohmann::json obj;
			for(const auto& shader: m_shaders)
			{
				switch(shader.tokenType)
				{
					case shader::ShaderType::Vertex:
						obj["vertexPath"] = shader.file.m_relativePath;
						break;
					case shader::ShaderType::Fragment:
						obj["fragmentPath"] = shader.file.m_relativePath;
						break;
					case shader::ShaderType::Geometry:
						obj["geometryPath"] = shader.file.m_relativePath;
						break;
					case shader::ShaderType::TessControl:
						obj["tessControlPath"] = shader.file.m_relativePath;
						break;
					case shader::ShaderType::TessEvaluation:
						obj["tessEvaluationPath"] = shader.file.m_relativePath;
						break;
					case shader::ShaderType::Compute:
						obj["computePath"] = shader.file.m_relativePath;
						break;
				}
			}
			auto temp = obj.dump();
			return{ temp.data(), temp.data() + temp.size() };
		}

		void Deserialize(const std::vector<char>& data) override
		{
			nlohmann::json obj(data);
			for (const auto& json : obj.items())
			{
				if (json.key() == "vertexPath") {
					m_shaders.emplace_back(shader::ShaderType::Vertex, json.value().get<std::string>());
				}
				if (json.key() == "fragmentPath") {
					m_shaders.emplace_back(shader::ShaderType::Fragment, json.value().get<std::string>());
				}
				if (json.key() == "geometryPath") {
					m_shaders.emplace_back(shader::ShaderType::Geometry, json.value().get<std::string>());
				}
				if (json.key() == "tessControlPath") {
					m_shaders.emplace_back(shader::ShaderType::TessControl, json.value().get<std::string>());
				}
				if (json.key() == "tessEvaluationPath") {
					m_shaders.emplace_back(shader::ShaderType::TessEvaluation, json.value().get<std::string>());
				}
				if (json.key() == "computePath") {
					m_shaders.emplace_back(shader::ShaderType::Compute, json.value().get<std::string>());
				}
			}
		}


	private:
		std::string m_name;
		std::vector<ShaderFile> m_shaders;
	};
}
#endif