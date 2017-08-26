/* (c) Juan McKernel & Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#include <LibZpg.hpp>
#include <iostream>
#include <cstring>
#include <zlib.h>

const char LibZpg::FILE_SIGN[] = {'Z','P','G','\0'};
const char LibZpg::FILE_VERSION[] = {'1','.','0','\0'};

bool LibZpg::open(const char *pFile)
{
	if (m_PackageFile.is_open())
		return false;

	m_mFileHeaders.clear();
	memset(&m_PackageHeader, 0, sizeof(m_PackageHeader));
	m_PackageFile.open(pFile, std::ios::in | std::ios::out | std::ios::binary);

	if (!checkFile())
	{
		m_PackageFile.close();
		return false;
	}

	getFileHeaders();

	return true;
}

void LibZpg::close()
{
	m_PackageFile.close();
}

bool LibZpg::create(const char *pFile)
{
	if (m_PackageFile.is_open())
		return false;

	m_mFileHeaders.clear();
	memset(&m_PackageHeader, 0, sizeof(m_PackageHeader));
	m_PackageFile.open(pFile, std::ios::out | std::ios::binary);
	if (m_PackageFile.is_open())
	{
		m_PackageHeader.m_NumFiles = 0;
		strncpy(m_PackageHeader.m_Version, FILE_VERSION, sizeof(m_PackageHeader.m_Version));
		m_PackageFile << FILE_SIGN;
		m_PackageFile.write(reinterpret_cast<const char*>(&m_PackageHeader), sizeof(m_PackageHeader));
		return true;
	}
	return false;
}

void LibZpg::getFileHeaders()
{
	m_PackageFile.seekg(sizeof(FILE_SIGN)-1, std::ios::beg);
	m_PackageFile.read(reinterpret_cast<char*>(&m_PackageHeader), sizeof(m_PackageHeader));

	for (unsigned int i=0; i<m_PackageHeader.m_NumFiles; i++)
	{
		ZpgFileHeader fileHeader;
		m_PackageFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
		const unsigned int nameLength = fileHeader.m_FileStart - m_PackageFile.tellg();
		std::cout << "LL: " << nameLength << std::endl;
		char aFileName[nameLength+1] = {0};
		m_PackageFile.read(aFileName, nameLength);
		aFileName[nameLength] = 0;
		m_mFileHeaders.insert(std::make_pair(std::string(aFileName), fileHeader));
		m_PackageFile.seekg(fileHeader.m_FileSizeComp, std::ios::cur);
	}
}

bool LibZpg::checkFile()
{
	if (!m_PackageFile.is_open())
		return false;

	char aSign[sizeof(FILE_SIGN)] = {0};
	m_PackageFile.seekg(0, std::ios::beg);
	m_PackageFile.read(aSign, sizeof(aSign)-1);
	return (strncmp(aSign, FILE_SIGN, sizeof(aSign)) == 0);
}

bool LibZpg::exists(const char *pFullPath)
{
	return m_mFileHeaders.find(pFullPath) != m_mFileHeaders.end();
}

bool LibZpg::addFromFile(const char *pFromFullPath, const char *pToFullPath)
{
	if (exists(pToFullPath))
	{
		std::cerr << "Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	std::ifstream file(pFromFullPath, std::ios::binary);
	if(!file.is_open())
	{
		std::cerr << "File '" << pFromFullPath << "' not found" << std::endl;
		return false;
	}

	file.seekg(0, std::ios::end);
	const unsigned long length = file.tellg();
	file.seekg(0, std::ios::beg);

	unsigned char fileData[length] = {0};
	file.read(reinterpret_cast<char*>(&fileData), length);

	file.close();

	return addFromMemory(fileData, length, pToFullPath);
}

bool LibZpg::addFromMemory(const unsigned char *pData, unsigned long size, const char *pToFullPath)
{
	if (exists(pToFullPath))
	{
		std::cerr << "Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	unsigned long compSize = compressBound(size);
	unsigned char compData[compSize] = {0};
	if (compress((Bytef*)compData, &compSize, (Bytef*)pData, size) != Z_OK)
	{
		std::cerr << "Unexpected ZLib Error using compress! '" << std::endl;
		return false;
	}

	m_PackageFile.seekg(0, std::ios::end);

	ZpgFileHeader fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
	const unsigned int nameLength = strlen(pToFullPath);
	fileHeader.m_FileSize = size;
	fileHeader.m_FileSizeComp = compSize;
	fileHeader.m_FileStart = (unsigned long)m_PackageFile.tellg() + sizeof(fileHeader) + nameLength;
	m_mFileHeaders.insert(std::make_pair(std::string(pToFullPath), fileHeader));

	m_PackageFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader)); // File Header
	m_PackageFile.write(pToFullPath, nameLength); // File Name
	m_PackageFile.write(reinterpret_cast<char*>(compData), compSize);	// File Data
	++m_PackageHeader.m_NumFiles;
	m_PackageFile.seekg(sizeof(FILE_SIGN)-1, std::ios::beg);
	m_PackageFile.write(reinterpret_cast<const char*>(&m_PackageHeader), sizeof(m_PackageHeader)); // Update Package Header

	return true;
}

unsigned char* LibZpg::getFileData(const char *pFullPath, unsigned long *pfileSize)
{
	std::map<std::string, ZpgFileHeader>::const_iterator cit = m_mFileHeaders.find(pFullPath);
	if (cit == m_mFileHeaders.end())
		return 0x0;

	const ZpgFileHeader &fileHeader = (*cit).second;
	unsigned char fileCompData[fileHeader.m_FileSizeComp];
	m_PackageFile.seekg(fileHeader.m_FileStart, std::ios::beg);
	m_PackageFile.read(reinterpret_cast<char*>(&fileCompData), fileHeader.m_FileSizeComp);

	unsigned long fileSize = fileHeader.m_FileSize;
	unsigned char *pfileData = new unsigned char[fileSize];
	if (uncompress((Bytef*)pfileData, &fileSize, (Bytef*)fileCompData, fileHeader.m_FileSizeComp) != Z_OK)
	{
		delete[] pfileData;
		std::cerr << "Unexpected ZLib Error using uncompress! '" << std::endl;
		return 0x0;
	}

	*pfileSize = fileHeader.m_FileSize;
	return pfileData;
}
