#pragma once
#include "File.h"
namespace asset
{

	//struct IAssetAction
	//{
	//	virtual void Undo() = 0;
	//	virtual void Complete() = 0;
	//	virtual ~IAssetAction() {}
	//};

	//struct FileBackup
	//{
	//	std::vector<char> dataBackup;
	//	std::string filePath;
	//	bool isFile = false;
	//
	//	FileBackup() = default;

	//	FileBackup& Create(const std::string& _filePath, bool deleteFile = false)
	//	{
	//		filePath = _filePath;
	//		isFile = FileExists(filePath);
	//		if (isFile)
	//		{
	//			dataBackup = GetFileContent(filePath);
	//			if (deleteFile) {
	//				DeleteFile(filePath);
	//			}
	//		}
	//		return *this;
	//	}

	//	void Restore()const
	//	{
	//		if (isFile)
	//		{
	//			WriteFile(dataBackup, filePath);
	//		}
	//		if (!isFile && FileExists(filePath))
	//		{
	//			DeleteFile(filePath);
	//		}
	//	}
	//};

	//struct AssetBackup
	//{
	//	std::vector<char> m_dataBackup;
	//	std::string m_relativePath;
	//	bool m_isFile = false;

	//	AssetBackup() = default;
	//	AssetBackup(const std::string& _relativePath, bool deleteFile = false)
	//	{
	//		Create(_relativePath, deleteFile);
	//	}


	//	AssetBackup& Create(const std::string& _relativePath, bool deleteFile = false)
	//	{
	//		m_relativePath = _relativePath;
	//		const std::string filePath = AssetPath(_relativePath);
	//		m_isFile = FileExists(filePath);
	//		if (m_isFile)
	//		{
	//			m_dataBackup = GetFileContent(filePath);
	//			if (deleteFile) {
	//				DeleteFile(filePath);
	//			}
	//		}
	//		return *this;
	//	}

	//	void Restore()const
	//	{
	//		const std::string filePath = AssetPath(m_relativePath);

	//		if (m_isFile)
	//		{
	//			WriteFile(m_dataBackup, filePath);
	//		}
	//		if (!m_isFile && FileExists(filePath))
	//		{
	//			DeleteFile(filePath);
	//		}
	//	}
	//};

	//struct AssetDeleteAction :public IAssetAction
	//{
	//	FileBackup fileBackup;

	//	std::string filePath;

	//	AssetDeleteAction(std::string _filePath) : filePath(std::move(_filePath))
	//	{
	//		AssetDeleteAction::Complete();

	//	}

	//	void Undo() override
	//	{
	//		fileBackup.Restore();
	//	}
	//	void Complete()override
	//	{
	//		fileBackup.Create(filePath, true);
	//	}
	//	~AssetDeleteAction()override = default;
	//};

	//struct AssetWriteAction :public IAssetAction
	//{
	//	FileBackup fileBackup;
	//	std::vector<char> data;
	//	std::string filePath;

	//	AssetWriteAction(std::vector<char>&& vec, const std::string _filePath)
	//		: data(std::move(vec)), filePath(_filePath)
	//	{
	//		AssetWriteAction::Complete();
	//	}

	//	void Undo() override
	//	{
	//		fileBackup.Restore();
	//	}
	//	void Complete()override
	//	{
	//		fileBackup.Create(filePath, true);
	//		WriteFile(data, filePath);
	//	}
	//	~AssetWriteAction()override = default;
	//};
	//struct AssetMoveAction :public IAssetAction
	//{
	//	FileBackup previousFile;
	//	FileBackup overwrittenFile;
	//	std::string prevPath;
	//	std::string newPath;

	//	AssetMoveAction(std::string previousPath, std::string _newPath)
	//		: prevPath(std::move(previousPath)), newPath(std::move(_newPath))
	//	{
	//		AssetMoveAction::Complete();

	//	}

	//	void Undo() override
	//	{
	//		previousFile.Create(prevPath, true);
	//		overwrittenFile.Create(newPath, true);

	//		if (previousFile.isFile)
	//		{
	//			previousFile.Restore();
	//			overwrittenFile.Restore();
	//		}
	//	}
	//	void Complete()override
	//	{
	//		previousFile.Create(prevPath, true);
	//		overwrittenFile.Create(newPath, true);
	//		if (previousFile.isFile)
	//		{
	//			WriteFile(previousFile.dataBackup, newPath);
	//		}
	//	}
	//	~AssetMoveAction()override = default;
	//};

	//class AssetActionRegistry
	//{
	//	std::vector<IAssetAction*> history;
	//	std::vector<IAssetAction*> redoHistory;

	//	static AssetActionRegistry& instance()
	//	{
	//		static AssetActionRegistry m_instance;
	//		return m_instance;
	//	}

	//public:
	//	static void Undo()
	//	{
	//		AssetActionRegistry& inst = instance();
	//		if (!inst.history.empty())
	//		{
	//			inst.history.back()->Undo();
	//			inst.redoHistory.emplace_back(inst.history.back());
	//			inst.history.pop_back();
	//		}
	//	}
	//	static void Redo()
	//	{
	//		AssetActionRegistry& inst = instance();
	//		if (!inst.redoHistory.empty())
	//		{
	//			inst.redoHistory.back()->Complete();
	//			inst.history.emplace_back(inst.redoHistory.back());
	//			inst.redoHistory.pop_back();
	//		}
	//	}
	//	static void Memorize(IAssetAction* action)
	//	{
	//		AssetActionRegistry& inst = instance();
	//		inst.history.emplace_back(action);
	//	}

	//};

}