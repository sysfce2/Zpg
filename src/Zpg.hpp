/* (c) Juan McKernel & Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef ZPG_HPP
#define ZPG_HPP

#include <map>
#include <vector>
#include <string>

struct ZpgHeader
{
	unsigned int m_Version;
	unsigned int m_NumFiles;
};

struct ZpgFileHeader
{
	unsigned int m_FileSize;
	unsigned int m_FileSizeComp;
	unsigned long m_FileStart;
};

class Zpg
{
    static const char FILE_SIGN[];

public:
    Zpg();
    ~Zpg();

    bool load(const char *pFile);
    bool saveToFile(const char *pFile);

    int exists(const char *pFullPath);

    bool addFromFile(const char *pFromFullPath, const char *pToFullPath);
    bool addFromMemory(const unsigned char *pData, unsigned long size, const char *pFullPath);

    const unsigned char* getFileData(const char *pFullPath, unsigned long *pfileSize);
    const ZpgFileHeader& getFileHeader(const char *pFullPath);
    const std::map<std::string, unsigned int>& getFiles() const { return m_mFiles; }

    static inline std::string toString(const unsigned char *pData, unsigned long size)
    {
    	return std::string(reinterpret_cast<const char*>(pData), size);
    }

    void unloadAll();

protected:
    ZpgHeader m_PackageHeader;

    std::map<std::string, unsigned int> m_mFiles;
    std::vector<ZpgFileHeader> m_vFileHeaders;
    std::vector<unsigned char*> m_vpFileDatas;
};

#endif // LIBZPG_HPP
