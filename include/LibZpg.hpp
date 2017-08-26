/* (c) Juan McKernel & Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef LIBZPG_HPP
#define LIBZPG_HPP

#include <fstream>
#include <map>

struct ZpgHeader
{
	char m_Version[4];
	unsigned int m_NumFiles;
};

struct ZpgFileHeader
{
	unsigned int m_NameLength;
	unsigned long m_FileSize;
	unsigned long m_FileSizeComp;
	unsigned long m_FileStart;
};

class LibZpg
{
    static const char FILE_SIGN[];
    static const char FILE_VERSION[];

public:
    bool open(const char *pFile);
    bool create(const char *pFile);
    void close();

    bool exists(const char *pFullPath);

    bool addFromFile(const char *pFromFullPath, const char *pToFullPath);
    bool addFromMemory(const unsigned char *pData, unsigned long size, const char *pFullPath);

    unsigned char* getFileData(const char *pFullPath, unsigned long *pfileSize);
    const std::map<std::string, ZpgFileHeader>& getFilesInfo() const { return m_mFileHeaders; }

private:
    bool checkFile();
    void getFileHeaders();

protected:
    std::fstream m_PackageFile;
    ZpgHeader m_PackageHeader;
    std::map<std::string, ZpgFileHeader> m_mFileHeaders;
};

#endif // LIBZPG_HPP
