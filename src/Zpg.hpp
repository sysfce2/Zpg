/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
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
	unsigned long m_FileSize;
	unsigned long m_FileSizeComp;
};

struct ZpgFile
{
	ZpgFileHeader m_Header;
	unsigned char *m_pData;
};

class Zpg
{
	static const char FILE_SIGN[];

public:
    Zpg();
    ~Zpg();

    bool load(const char *pFile);
    bool saveToFile(const char *pFile, int numIterations = 15);

    bool addFromFile(const char *pFromFullPath, const char *pToFullPath);
    bool addFromMemory(const unsigned char *pData, unsigned long size, const char *pFullPath); // Can't be >2GiB

    const unsigned char* getFileData(const char *pFullPath, unsigned long *pfileSize) const;
    const std::map<std::string, ZpgFile*>& getFiles() const { return m_mFiles; }

    static inline std::string toString(const unsigned char *pData, unsigned long size)
    {
    	return std::string(reinterpret_cast<const char*>(pData), size);
    }

    void unloadAll();

protected:
    ZpgHeader m_PackageHeader;
    std::map<std::string, ZpgFile*> m_mFiles;

private:
    bool exists(const char *pFullPath) const;
};

#endif // LIBZPG_HPP
