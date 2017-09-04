/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#include "Zpg.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include <zlib.h>
#include <zopfli/zopfli.h>


const char Zpg::FILE_SIGN[] = {'Z','P','G','1','a','\0'};
Zpg::Zpg()
{
	unloadAll();
}
Zpg::~Zpg()
{
	unloadAll();
}

bool Zpg::load(std::string File)
{
	unloadAll();

	std::ifstream PackageFile(File.c_str(), std::ios::binary);
	if (!PackageFile.is_open())
	{
		std::cerr << "[LibZpg] Can't open '" << File << "'!" << std::endl;
		return false;
	}

	// File Size
	unsigned long PackageSize = 0ul;
	PackageFile.seekg(0, std::ios::end);
	PackageSize = PackageFile.tellg();
	PackageFile.seekg(0, std::ios::beg);

	// Is ZPG file?
	char aSign[sizeof(FILE_SIGN)];
	memset(aSign, 0, sizeof(aSign));
	PackageFile.read(aSign, sizeof(aSign)-1);
	if (strncmp(aSign, FILE_SIGN, sizeof(aSign)) != 0)
	{
		std::cerr << "[LibZpg] The file '" << File << "' doesn't appear to be ZPG type..." << std::endl;
		PackageFile.close();
		return false;
	}

	// Get Files
	while (static_cast<unsigned long>(PackageFile.tellg()) < PackageSize)
	{
		ZpgFile *pZpgFile = new ZpgFile();
		memset(pZpgFile, 0, sizeof(ZpgFile));
		// Get Header
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		PackageFile.read(reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(pZpgFile->m_Header));
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		unsigned char Temp[sizeof(ZpgFileHeader)];
		PackageFile.read(reinterpret_cast<char*>(Temp), sizeof(Temp));
		swap(Temp, sizeof(Temp));
		pZpgFile->m_Header = *(reinterpret_cast<ZpgFileHeader*>(Temp));
	#else
		#error Not Implemented!
	#endif

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

bool Zpg::saveToFile(std::string File, int NumIterations)
{
	std::ofstream PackageFile(File.c_str(), std::ios::binary);
	if (!PackageFile.is_open())
		return false;

	ZopfliOptions Options;
	ZopfliInitOptions(&Options);
	Options.numiterations = NumIterations; // Compression level

	PackageFile.write(FILE_SIGN, sizeof(FILE_SIGN)-1); // Sign

	// Files
	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.begin();
	while (It != m_mFiles.end())
	{
		ZpgFile *pZpgFile = (*It).second;
		size_t CompSize = 0ul;
		unsigned char *pCompData = NULL;

		ZopfliCompress(&Options, ZOPFLI_FORMAT_ZLIB, pZpgFile->m_pData, pZpgFile->m_Header.m_FileSize, &pCompData, &CompSize);

		if (pCompData)
		{
			pZpgFile->m_Header.m_FileSizeComp = CompSize;

		#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			PackageFile.write(reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(pZpgFile->m_Header)); // File Header
		#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			unsigned char Temp[sizeof(pZpgFile->m_Header)];
			memcpy(Temp, reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(Temp));
			swap(Temp, sizeof(Temp));
			PackageFile.write(reinterpret_cast<char*>(Temp), sizeof(Temp)); // File Header
		#else
			#error Not Implemented!
		#endif
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
}

bool Zpg::exists(std::string FullPath) const
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.find(FullPath);
	return (It != m_mFiles.end());
}

void Zpg::swap(unsigned char *pData, unsigned long Size) const
{
	const unsigned long m = Size/2ul;
	for (unsigned long i=Size-1,e=0; e<m; --i,++e)
	{
		const unsigned char Temp = pData[e];
		pData[e] = pData[i];
		pData[i] = Temp;
	}
}

bool Zpg::removeFile(std::string FullPath)
{
	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.find(FullPath);
	if (It == m_mFiles.end())
		return false;

	if ((*It).second)
	{
		if ((*It).second->m_pData)
			delete[] (*It).second->m_pData;
		delete (*It).second;
	}

	m_mFiles.erase(It);
	return true;
}

bool Zpg::moveFile(std::string OldFullPath, std::string NewFullPath)
{
	if (exists(NewFullPath))
	{
		std::cerr << "[LibZpg] Can't move '" << OldFullPath << "' to '" << NewFullPath << "'... already exists!" << std::endl;
		return false;
	}

	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.find(OldFullPath);
	if (It != m_mFiles.end())
	{
	  std::swap(m_mFiles[NewFullPath], (*It).second);
	  m_mFiles.erase(It);
	  return true;
	}

	return false;
}

bool Zpg::addFromFile(std::string FromFullPath, std::string ToFullPath, bool Overwrite)
{
	std::ifstream File(FromFullPath.c_str(), std::ios::binary);
	if(!File.is_open())
	{
		std::cerr << "[LibZpg] File '" << FromFullPath << "' not found" << std::endl;
		return false;
	}

	File.seekg(0, std::ios::end);
	const unsigned int Length = File.tellg();
	File.seekg(0, std::ios::beg);

	unsigned char FileData[Length];
	memset(FileData, 0, Length);
	File.read(reinterpret_cast<char*>(FileData), Length);

	File.close();

	return addFromMemory(FileData, Length, ToFullPath, Overwrite);
}

bool Zpg::addFromMemory(const unsigned char *pData, unsigned long Size, std::string ToFullPath, bool Overwrite)
{
	if (exists(ToFullPath))
	{
		if (Overwrite)
		{
			if (!removeFile(ToFullPath))
			{
				std::cerr << "[LibZpg] Unexpected error overwriting '" << ToFullPath << "'..." << std::endl;
				return false;
			}
		}
		else
		{
			std::cerr << "[LibZpg] Destination Path '" << ToFullPath << "' already in use" << std::endl;
			return false;
		}
	}

	ZpgFile *pZpgFile = new ZpgFile;
	if (!pZpgFile)
		return false;

	memset(&pZpgFile->m_Header, 0, sizeof(ZpgFile));

	pZpgFile->m_Header.m_FileSize = Size;
	pZpgFile->m_pData = new unsigned char[Size];
	if (!pZpgFile->m_pData)
		return false;
	memcpy(pZpgFile->m_pData, pData, Size);

	m_mFiles.insert(std::make_pair(ToFullPath, pZpgFile));

	return true;
}

const unsigned char* Zpg::getFileData(std::string FullPath, unsigned long *pFileSize) const
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.find(FullPath);
	if (It == m_mFiles.end())
		return 0x0;

	*pFileSize = (*It).second->m_Header.m_FileSize;
	return (*It).second->m_pData;
}
