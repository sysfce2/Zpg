/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#include "Zpg.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <zlib.h>
#include <zopfli/zopfli.h>

#define ZPG_VERSION	1

const char Zpg::FILE_SIGN[] = {'Z','P','G','\0'};
Zpg::Zpg()
{
	unloadAll();
}
Zpg::~Zpg()
{
	unloadAll();
}

bool Zpg::load(const char *pFile)
{
	unloadAll();

	std::ifstream PackageFile(pFile, std::ios::binary);
	if (!PackageFile.is_open())
	{
		std::cerr << "[LibZpg] Can't open '" << pFile << "'!" << std::endl;
		return false;
	}

	// Is ZPG file?
	char aSign[sizeof(FILE_SIGN)];
	memset(aSign, 0, sizeof(aSign));
	PackageFile.seekg(0, std::ios::beg);
	PackageFile.read(aSign, sizeof(aSign));
	if (strncmp(aSign, FILE_SIGN, sizeof(aSign)) != 0)
	{
		std::cerr << "[LibZpg] The file '" << pFile << "' doesn't appear to be ZPG type..." << std::endl;
		PackageFile.close();
		return false;
	}

	// Get File Header
	PackageFile.read(reinterpret_cast<char*>(&m_PackageHeader), sizeof(m_PackageHeader));
	if (m_PackageHeader.m_Version < 0 || m_PackageHeader.m_Version > ZPG_VERSION)
	{
		std::cerr << "[LibZpg] The file '" << pFile << "' uses a version incompatible with the program! Try to upgrade..." << std::endl;
		PackageFile.close();
		unloadAll();
		return false;
	}

	// Get Files
	for (unsigned int i=0; i<m_PackageHeader.m_NumFiles; i++)
	{
		ZpgFile *pZpgFile = new ZpgFile();
		memset(pZpgFile, 0, sizeof(ZpgFile));
		// Get Header
		PackageFile.read(reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(pZpgFile->m_Header));

		// Get Name
		std::string FileName;
		char c = 0;
		while (PackageFile.read(&c, 1) && c != 0)
			FileName += c;

		// Get Data
		unsigned char FileCompData[pZpgFile->m_Header.m_FileSizeComp];
		PackageFile.read(reinterpret_cast<char*>(&FileCompData), pZpgFile->m_Header.m_FileSizeComp);

		unsigned long FileSize = pZpgFile->m_Header.m_FileSize;
		pZpgFile->m_pData = new unsigned char[FileSize];
		if (uncompress((Bytef *)pZpgFile->m_pData, &FileSize, (Bytef *)FileCompData, pZpgFile->m_Header.m_FileSizeComp) != Z_OK)
		{
			delete[] pZpgFile->m_pData;
			pZpgFile->m_pData = 0x0;
			std::cerr << "[LibZpg] Unexpected ZLib Error using uncompress with the file '" << FileName << "'! '" << std::endl;
		}

		m_mFiles.insert(std::make_pair(FileName, pZpgFile));
	}

	PackageFile.close();

	return true;
}

bool Zpg::saveToFile(const char *pFile, int numIterations)
{
	std::ofstream PackageFile(pFile, std::ios::binary);
	if (!PackageFile.is_open())
		return false;

	ZopfliOptions Options;
	ZopfliInitOptions(&Options);
	Options.numiterations = numIterations; // Compression level

	PackageFile.write(FILE_SIGN, sizeof(FILE_SIGN)); // Sign
	m_PackageHeader.m_Version = ZPG_VERSION; // ZPG Version
	PackageFile.write(reinterpret_cast<const char*>(&m_PackageHeader), sizeof(m_PackageHeader)); // File Header

	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.begin();
	while (It != m_mFiles.end())
	{
		ZpgFile *pZpgFile = (*It).second;
		unsigned long CompSize = 0;
		unsigned char *pCompData = NULL;

		ZopfliCompress(&Options, ZOPFLI_FORMAT_ZLIB, pZpgFile->m_pData, pZpgFile->m_Header.m_FileSize, &pCompData, &CompSize);

		if (pCompData)
		{
			pZpgFile->m_Header.m_FileSizeComp = CompSize;

			PackageFile.write(reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(pZpgFile->m_Header)); // File Header
			PackageFile << (*It).first << '\0'; // File Name
			PackageFile.write(reinterpret_cast<char*>(pCompData), CompSize);	// File Data

			free(pCompData);
		}
		else
			std::cerr << "[LibZpg] Unexpected ZLib Error using compress with the file '" << (*It).first << "'! '" << std::endl;

		++It;
	}

	PackageFile.close();
	return true;
}

void Zpg::unloadAll()
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.begin();
	while (It != m_mFiles.end())
	{
		if ((*It).second->m_pData)
			delete[] (*It).second->m_pData;
		delete (*It).second;
		++It;
	}
	m_mFiles.clear();

	memset(&m_PackageHeader, 0, sizeof(m_PackageHeader));
}

bool Zpg::exists(const char *pFullPath) const
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.find(pFullPath);
	return (It != m_mFiles.end());
}

bool Zpg::addFromFile(const char *pFromFullPath, const char *pToFullPath)
{
	if (exists(pToFullPath))
	{
		std::cerr << "[LibZpg] Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	std::ifstream File(pFromFullPath, std::ios::binary);
	if(!File.is_open())
	{
		std::cerr << "[LibZpg] File '" << pFromFullPath << "' not found" << std::endl;
		return false;
	}

	File.seekg(0, std::ios::end);
	const unsigned int Length = File.tellg();
	File.seekg(0, std::ios::beg);

	unsigned char FileData[Length];
	memset(FileData, 0, Length);
	File.read(reinterpret_cast<char*>(FileData), Length);

	File.close();

	return addFromMemory(FileData, Length, pToFullPath);
}

bool Zpg::addFromMemory(const unsigned char *pData, unsigned long size, const char *pToFullPath)
{
	if (exists(pToFullPath))
	{
		delete[] pData;
		std::cerr << "[LibZpg] Destination Path '" << pToFullPath << "' already in use" << std::endl;
		return false;
	}

	ZpgFile *pZpgFile = new ZpgFile;
	if (!pZpgFile)
		return false;

	memset(&pZpgFile->m_Header, 0, sizeof(ZpgFile));

	pZpgFile->m_Header.m_FileSize = size;
	pZpgFile->m_pData = new unsigned char[size];
	if (!pZpgFile->m_pData)
		return false;
	memcpy(pZpgFile->m_pData, pData, size);

	m_mFiles.insert(std::make_pair(std::string(pToFullPath), pZpgFile));

	++m_PackageHeader.m_NumFiles;

	return true;
}

const unsigned char* Zpg::getFileData(const char *pFullPath, unsigned long *pfileSize) const
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.find(pFullPath);
	if (It == m_mFiles.end())
		return 0x0;

	*pfileSize = (*It).second->m_Header.m_FileSize;
	return (*It).second->m_pData;
}
