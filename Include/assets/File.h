#pragma once
#include <iostream>
#include <ostream>
#include <vector>
#include <fstream>
#include <functional>

#include "utility/Span.h"

namespace path
{
	class Shortcuts
	{
	public:
		std::vector<char> str;
		std::vector<std::pair<int, int>> indices;
		std::vector<std::function<void(std::string&)>> macros;

		static Shortcuts& instance()
		{
			static Shortcuts shortcutsInstance; // Create a static instance
			return shortcutsInstance;
		}
		static void replace(std::string& str, std::string_view sequence, std::string_view replacement)
		{
			size_t startPos = 0;

			size_t foundPos = str.find(sequence, startPos);

			while (foundPos != std::string::npos)
			{
				str.replace(foundPos, sequence.length(), replacement);

				startPos = foundPos + replacement.length();

				foundPos = str.find(sequence, startPos);
			}
		}

		static void emplace(const std::string& shortcut, std::function<void(std::string&)>&& callback)
		{
			Shortcuts& shortcutsInstance = instance(); // Get the static instance
			size_t begin = shortcutsInstance.str.size();
			shortcutsInstance.indices.emplace_back(begin, begin + shortcut.length());
			shortcutsInstance.str.resize(shortcutsInstance.str.size() + shortcut.length());
			std::memcpy(shortcutsInstance.str.data() + begin, shortcut.data(), shortcut.length());
			shortcutsInstance.macros.emplace_back(std::move(callback));
		}

		static void Apply(std::string& out_str)
		{
			Shortcuts& shortcutsInstance = instance(); // Get the static instance
			for (size_t i = 0; i < shortcutsInstance.indices.size(); i++)
			{
				std::string_view view(shortcutsInstance.str.data() + shortcutsInstance.indices[i].first, shortcutsInstance.str.data()+shortcutsInstance.indices[i].second);
				if (out_str.find(view) != std::string::npos)
				{
					shortcutsInstance.macros[i](out_str);
					i = -1;
				}
			}
		}
	};

}


namespace asset
{

	inline bool FileExists(const std::string& filePath) {
		std::ifstream file(filePath);
		return file.good();
	}

	inline std::vector<char> GetFileContent(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file) {
			// Handle error, e.g., file not found
			return {};
		}

		file.seekg(0, std::ios::end);
		std::streampos fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);
		return buffer;
	}

	inline void WriteFile(const utility::const_span<char> data, const std::string& filePath) {
		std::ofstream file(filePath, std::ios::binary);
		if (!file) {
			// Handle error, e.g., unable to create or write to the file
			std::cerr << "Error: Unable to write to the file " << filePath << std::endl;
			return;
		}

		file.write(data.begin(), data.size());
		file.close();
	}

	inline void DeleteFile(const std::string& filePath) {
		if (remove(filePath.c_str()) != 0) {
			// Handle error, e.g., file not found or unable to delete
			std::cerr << "Error: Unable to delete the file " << filePath << std::endl;
		}
	}
}
