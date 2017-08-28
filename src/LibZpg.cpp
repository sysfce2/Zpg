/* (c) Juan McKernel & Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#include "LibZpg.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <zlib.h>

#define ZPG_VERSION	1

const char LibZpg::FILE_SIGN[] = {'Z','P','G','\0'};
LibZpg::LibZpg()
{
	unloadAll();
}
LibZpg::~LibZpg()
{
	unloadAll();
}

bool LibZpg::load(const char *pFile)
{
	unloadAll();

	std::ifstream packageFile(pFile, std::ios::binary);
	if (!packageFile.is_open())
	{
		std::cerr << "[LibZpg] Can't open '" << pFile << "'!" << std::endl;
		return false;
	}

	// Is ZPG file?
	char aSign[sizeof(FILE_SIGN)];
	memset(aSign, 0, sizeof(aSign));
	packageFile.seekg(0, std::ios::beg);
	packageFile.read(aSign, sizeof(aSign));
	if (strncmp(aSign, FILE_SIGN, sizeof(aSign)) != 0)
	{
		std::cerr << "[LibZpg] The file '" << pFile << "' doesn't appear to be ZPG type..." << std::endl;
		packageFile.close();
		return false;
	}

	// Get File Header
	packageFile.read(reinterpret_cast<char*>(&m_PackageHeader), sizeof(m_PackageHeader));
	if (m_PackageHeader.m_Version < 0 || m_PackageHeader.m_Version > ZPG_VERSION)
	{
		std::cerr << "[LibZpg] The file '" << pFile << "' uses a version incompatible with the program! Try to upgrade..." << std::endl;
		packageFile.close();
		unloadAll();
		return false;
	}

	// Get Files
	for (unsigned int i=0; i<m_PackageHeader.m_NumFiles; i++)
	{
		// Get Header
		ZpgFileHeader fileHeader;
		memset(&fileHeader, 0, sizeof(fileHeader));
		packageFile.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
		const unsigned int nameLength = fileHeader.m_FileStart - packageFile.tellg();
		char aFileName[nameLength+1];
		memset(aFileName, 0, sizeof(aFileName));
		packageFile.read(aFileName, nameLength);
		aFileName[nameLength] = 0;
		m_vFileHeaders.push_back(fileHeader);

		// Get Data
		unsigned char fileCompData[fileHeader.m_FileSizeComp];
		packageFile.read(reinterpret_cast<char*>(&fileCompData), fileHeader.m_FileSizeComp);

		unsigned long fileSize = fileHeader.m_FileSize;
		unsigned char *pFileData = new unsigned char[fileSize+1];
		if (uncompress((Bytef*)pFileData, &fileSize, (Bytef*)fileCompData, fileHeader.m_FileSizeComp) != Z_OK)
		{
			delete[] pFileData;
			pFileData = 0x0;
			std::cerr << "[LibZpg] Unexpected ZLib Error using uncompress with the file '" << aFileName << "'! '" << std::endl;
		}

		pFileData[fileSize] = 0; // It is assumed that all can be a string
		m_vpFileDatas.push_back(pFileData);
		m_mFiles.insert(std::make_pair(std::string(aFileName), i));
	}

	packageFile.close();

	return true;
}

bool LibZpg::saveToFile(const char *pFile)
{
	std::ofstream packageFile(pFile, std::ios::binary);
	if (!packageFile.is_open())
		return false;

	packageFile.write(FILE_SIGN, sizeof(FILE_SIGN)); // Sign
	m_PackageHeader.m_Version = ZPG_VERSION; // ZPG Version
	packageFile.write(reinterpret_cast<const char*>(&m_PackageHeader), sizeof(m_PackageHeader)); // File Header

	std::map<std::string, unsigned int>::const_iterator cit = m_mFiles.begin();
	while (cit != m_mFiles.end())
	{
		ZpgFileHeader *pFileHeader = &m_vFileHeaders[(*cit).second];
		const unsigned char *pFileData = m_vpFileDatas[(*cit).second];

		const unsigned long fileSize = pFileHeader->m_FileSize;
		unsigned long compSize = compressBound(fileSize);
		unsigned char compData[compSize];
		memset(compData, 0, sizeof(compData));
		if (compress((Bytef*)compData, &compSize, (Bytef*)pFileData, fileSize) == Z_OK)
		{
			pFileHeader->m_FileSizeComp = compSize;
			pFileHeader->m_FileStart = (unsigned long)packageFile.tellp() + sizeof(ZpgFileHeader) + (*cit).first.length();

			packageFile.write(reinterpret_cast<char*>(pFileHeader), sizeof((*pFileHeader))); // File Header
			packageFile.write((*cit).first.c_str(), (*cit).first.length()); // File Name
			packageFile.write(reinterpret_cast<char*>(compData), compSize);	// File Data
		}
		else
			std::cerr << "[LibZpg] Unexpected ZLib Error using compress with the file '" << (*cit).first << "'! '" << std::endl;

		++cit;
	}

	packageFile.close();
	return true;
}

void LibZpg::unloadAll()
{
	m_vFileHeaders.clear();

	std::vector<unsigned char*>::iterator cit = m_vpFileDatas.begin();
	while (cit != m_vpFileDatas.end())
		delete[] (*cit++);
	m_vpFileDatas.clear();

	memset(&m_PackageHeader, 0, sizeof(m_PackageHeader));
}

int LibZpg::exists(const char *pFullPath)
{
	std::map<std::string, unsigned int>::iterator it = m_mFiles.find(pFullPath);
	if (it != m_mFiles.end())
		return (*it).second;

	return -1;
}

bool LibZpg::addFromFile(const char *pFromFullPath, const char *pToFullPath)
{
	if (exists(pToFullPath) != -1)
	{
		std::cerr << "[LibZpg] Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	std::ifstream file(pFromFullPath, std::ios::binary);
	if(!file.is_open())
	{
		std::cerr << "[LibZpg] File '" << pFromFullPath << "' not found" << std::endl;
		return false;
	}

	file.seekg(0, std::ios::end);
	const unsigned long length = file.tellg();
	file.seekg(0, std::ios::beg);

	unsigned char *pFileData = new unsigned char[length];
	memset(pFileData, 0, length);
	file.read(reinterpret_cast<char*>(pFileData), length);

	file.close();

	return addFromMemory(pFileData, length, pToFullPath);
}

bool LibZpg::addFromMemory(const unsigned char *pData, unsigned long size, const char *pToFullPath)
{
	if (exists(pToFullPath) != -1)
	{
		delete[] pData;
		std::cerr << "[LibZpg] Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	ZpgFileHeader fileHeader;
	memset(&fileHeader, 0, sizeof(fileHeader));
	fileHeader.m_FileSize = size;

	m_vFileHeaders.push_back(fileHeader);
	m_vpFileDatas.push_back((unsigned char*)pData);
	m_mFiles.insert(std::make_pair(pToFullPath, m_PackageHeader.m_NumFiles));

	m_PackageHeader.m_NumFiles += 1;

	return true;
}

const unsigned char* LibZpg::getFileData(const char *pFullPath, unsigned long *pfileSize)
{
	std::map<std::string, unsigned int>::const_iterator cit = m_mFiles.find(pFullPath);
	if (cit == m_mFiles.end())
		return 0x0;

	*pfileSize = m_vFileHeaders[(*cit).second].m_FileSize;
	return m_vpFileDatas[(*cit).second];
}

const ZpgFileHeader& LibZpg::getFileHeader(const char *pFullPath)
{
	std::map<std::string, unsigned int>::const_iterator cit = m_mFiles.find(pFullPath);
	return m_vFileHeaders[(*cit).second];
}
