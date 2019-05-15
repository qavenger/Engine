#pragma once
#include <string>
#include <functional>
#include <algorithm>
#include <comdef.h>
#include <fstream>
class FileUtility
{
public:
	bool IsEqual(const std::wstring& str0, const std::wstring& str1);
	void TrimDelimiter(std::wstring& path);
	
	void SearchFileInFolder(const wchar_t* path, const wchar_t* ext, std::function<void(const wchar_t* filename)> fileOpFunc, bool searchSubFolder = true);
	char* ReadBinaryFile(const wchar_t* path, unsigned long long* fileSize);
};
