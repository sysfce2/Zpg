/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#include "Zpg.hpp"

#include <iostream>
#include <cstring>
#include <zlib.h>
#include <ios>


const char Zpg::FILE_SIGN[] = {'Z','P','G','1','a','\0'};
Zpg::Zpg()
{
	unloadAll();
}
Zpg::~Zpg()
{
	close();
}

void Zpg::close()
{
	if (m_PackageFile.is_open())
		m_PackageFile.close();
	unloadAll();
}

bool Zpg::open(std::string File)
{
	m_PackageFile.open(File.c_str(), std::ios::binary);
	if (!m_PackageFile.is_open())
	{
		std::cerr << "[LibZpg] Can't open '" << File << "'!" << std::endl;
		return false;
	}

	// File Size
	std::streamoff PackageSize = 0ul;
	m_PackageFile.seekg(0, std::ios::end);
	PackageSize = m_PackageFile.tellg();
	m_PackageFile.seekg(0, std::ios::beg);

	// Is ZPG file?
	char aSign[sizeof(FILE_SIGN)];
	memset(aSign, 0, sizeof(aSign));
	m_PackageFile.read(aSign, sizeof(aSign)-1);
	if (strncmp(aSign, FILE_SIGN, sizeof(aSign)) != 0)
	{
		std::cerr << "[LibZpg] The file '" << File << "' doesn't appear to be ZPG type..." << std::endl;
		m_PackageFile.close();
		return false;
	}

	// Get Files
	while (m_PackageFile.tellg() < PackageSize)
	{
		ZpgFile *pZpgFile = new ZpgFile();
		memset(pZpgFile, 0, sizeof(ZpgFile));
		// Get Header
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		m_PackageFile.read(reinterpret_cast<char*>(&pZpgFile->m_Header), sizeof(pZpgFile->m_Header));
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		unsigned char Temp[sizeof(ZpgFileHeader)];
		PackageFile.read(reinterpret_cast<char*>(Temp), sizeof(Temp));
		swap(Temp, sizeof(Temp));
		pZpgFile->m_Header = *(reinterpret_cast<ZpgFileHeader*>(Temp));
	#else
		#error Not Implemented!
	#endif

		// Get Name
		char c = 0;
		while (m_PackageFile.read(&c, 1) && c != 0)
			pZpgFile->m_FileName += c;

		pZpgFile->m_Offset = static_cast<unsigned long>(m_PackageFile.tellg());
		m_PackageFile.seekg(pZpgFile->m_Header.m_FileSizeComp, std::ios_base::cur);

		m_mFiles.insert(std::make_pair(pZpgFile->m_FileName, pZpgFile));
	}

	return true;
}

bool Zpg::decompressFileData(ZpgFile *pZpgFile)
{
	const unsigned long fileSize = pZpgFile->m_Header.m_FileSizeComp;
	unsigned char *pFileCompData = new unsigned char[fileSize];
	m_PackageFile.seekg(pZpgFile->m_Offset, std::ios_base::beg);
	m_PackageFile.read(reinterpret_cast<char*>(pFileCompData), pZpgFile->m_Header.m_FileSizeComp);

	unsigned long FileSize = pZpgFile->m_Header.m_FileSize;
	pZpgFile->m_pData = new unsigned char[FileSize];
	if (uncompress((Bytef *)pZpgFile->m_pData, &FileSize, (Bytef *)pFileCompData, pZpgFile->m_Header.m_FileSizeComp) != Z_OK)
	{
		delete[] pZpgFile->m_pData;
		pZpgFile->m_pData = 0x0;
		std::cerr << "[LibZpg] Unexpected ZLib Error using uncompress with the file '" << pZpgFile->m_FileName << "'! '" << std::endl;
		return false;
	}

	delete [] pFileCompData;
	return true;
}

bool Zpg::saveToFile(std::string File)
{
	std::ofstream PackageFile(File.c_str(), std::ios::binary);
	if (!PackageFile.is_open())
		return false;

	PackageFile.write(FILE_SIGN, sizeof(FILE_SIGN)-1); // Sign

	// Files
	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.begin();
	while (It != m_mFiles.end())
	{
		ZpgFile *pZpgFile = (*It).second;
		unsigned long CompSize = 0ul;
		unsigned char *pCompData = NULL;

		CompSize = compressBound(pZpgFile->m_Header.m_FileSize);
		pCompData = new unsigned char[CompSize];
		if (compress(static_cast<Bytef*>(pCompData), static_cast<uLong*>(&CompSize), static_cast<Bytef*>(pZpgFile->m_pData), static_cast<uLong>(pZpgFile->m_Header.m_FileSize)) != Z_OK)
		{
			delete [] pCompData;
			pCompData = 0x0;
			std::cerr << "[LibZpg] Unexpected ZLib Error using compress with the file '" << (*It).first << "'! '" << std::endl;
		}
		else
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

			delete [] pCompData;
		}

		++It;
	}

	PackageFile.close();
	return true;
}

void Zpg::unloadData(std::string FullPath)
{
	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.find(FullPath);
	if (It != m_mFiles.end() && (*It).second->m_pData)
	{
		delete[] (*It).second->m_pData;
		(*It).second->m_pData = 0x0;
	}
}

void Zpg::unloadAll()
{
	std::map<std::string, ZpgFile*>::iterator It = m_mFiles.begin();
	while (It != m_mFiles.end())
	{
		if ((*It).second->m_pData)
		{
			delete[] (*It).second->m_pData;
			(*It).second->m_pData = 0x0;
		}
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
	const unsigned long Length = static_cast<const unsigned long>(File.tellg());
	File.seekg(0, std::ios::beg);

	unsigned char *pFileData = new unsigned char[Length];
	memset(pFileData, 0, Length);
	File.read(reinterpret_cast<char*>(pFileData), Length);
	File.close();

	const bool result = addFromMemory(pFileData, Length, ToFullPath, Overwrite);
	delete [] pFileData;

	return result;
}

bool Zpg::addFromMemory(const unsigned char *pData, const unsigned long Size, std::string ToFullPath, bool Overwrite)
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

const unsigned char* Zpg::getFileData(std::string FullPath, unsigned long *pFileSize)
{
	std::map<std::string, ZpgFile*>::const_iterator It = m_mFiles.find(FullPath);
	if (It == m_mFiles.end())
		return 0x0;

	*pFileSize = (*It).second->m_Header.m_FileSize;
	if (!(*It).second->m_pData)
		decompressFileData((*It).second);
	return (*It).second->m_pData;
}
