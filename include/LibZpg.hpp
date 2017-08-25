/* (c) Juan McKernel & Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef LIBZPG_HPP
#define LIBZPG_HPP

#include <fstream>
#include <vector>

#define LZPG_PATH_MAX_LENGTH			512
#define PATH_DELIMITER					'/'

struct ZpgHeader
{
	char m_Version[4];
	unsigned int m_NumFiles;
};

struct ZpgFileHeader
{
	char m_aFullPath[LZPG_PATH_MAX_LENGTH];
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
    void close();
    bool create(const char *pFile);

    bool exists(const char *pFullPath);

    bool addFromFile(const char *pFromFullPath, const char *pToFullPath);
    bool addFromMemory(const unsigned char *pData, unsigned long size, const char *pFullPath);
    unsigned char* getFileData(const char *pFullPath, unsigned long *pfileSize);
    bool remove(const char *pFullPath);
    bool move(const char *pCurFullPath, const char *pNewFullPath);

    const std::vector<ZpgFileHeader>& getFilesInfo() const { return m_vFileHeaders; }

private:
    bool checkFile();
    void getFileHeaders();

protected:
    std::fstream m_PackageFile;
    ZpgHeader m_PackageHeader;
    std::vector<ZpgFileHeader> m_vFileHeaders;
};

#endif // LIBZPG_HPP
