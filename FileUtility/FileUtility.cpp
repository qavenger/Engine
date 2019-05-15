#include "FileUtility.h"
bool FileUtility::IsEqual(const std::wstring & str0, const std::wstring & str1)
{
	if (str0.length() != str1.length())return false;

	for (int i = 0; i < (int)str0.length(); ++i)
	{
		if (towlower(str0[i]) != towlower(str1[i])) return false;
	}

	return true;
}

void FileUtility::TrimDelimiter(std::wstring & path)
{
	std::replace(path.begin(), path.end(), L'\\', L'/');
	size_t pos = path.find(L"//");
	while (pos != std::wstring::npos)
	{
		path.replace(pos, 2, L"/");
		pos = path.find(L"//");
	}
}

void FileUtility::SearchFileInFolder(const wchar_t * path, const wchar_t * ext, std::function<void(const wchar_t* filename)> fileOpFunc, bool searchSubFolder/* = true */)
{
	WIN32_FIND_DATAW foundFile;
	HANDLE hFind;

	std::wstring resFindingPath = path;
	TrimDelimiter(resFindingPath);
	hFind = FindFirstFileW(resFindingPath.data(), &foundFile);

	std::wstring relativePath(path);
	relativePath = relativePath.substr(0, relativePath.find_last_of(L'/') + 1);
	relativePath += L'/';
	do
	{
		if ((foundFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			std::wstring resName = relativePath + foundFile.cFileName;
			std::wstring fileExt = resName.substr(resName.find_last_of(L'.'));

			if (IsEqual(ext, fileExt))
			{
				fileOpFunc(resName.data());
			}
		}
		else if(searchSubFolder)
		{
			if (foundFile.cFileName[0] != '.')
			{
				std::wstring subDirName(relativePath);
				subDirName += foundFile.cFileName;
				subDirName += L"/*.*";

				SearchFileInFolder(subDirName.data(), ext, fileOpFunc);
			}
		}
	} while (FindNextFileW(hFind, &foundFile) != 0);
}

char * FileUtility::ReadBinaryFile(const wchar_t * path, unsigned long long * fileSize)
{
	std::ifstream fin(path, std::ios::binary);
	if (!fin.is_open())
	{
		OutputDebugStringW(L"Read File: ");
		OutputDebugStringW(path);
		OutputDebugStringW(L" Failed");
		return nullptr;
	}

	fin.seekg(0, std::ios::end);
	*fileSize = (unsigned long long)fin.tellg();

	char* pBuffer = new char[*fileSize];
	fin.seekg(0);
	fin.read(pBuffer, *fileSize);
	fin.close();
	return pBuffer;
}
