/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef ZPG_HPP
#define ZPG_HPP

#include <map>
#include <string>
#include <fstream>

#define MAX_FILENAME_LENGTH 1024

struct ZpgFileHeader
{
	unsigned long m_FileSize;
	unsigned long m_FileSizeComp;
};

struct ZpgFile
{
	ZpgFileHeader m_Header;
	char m_aFileName[MAX_FILENAME_LENGTH];
	unsigned long m_Offset;
	unsigned char *m_pData;
};

class Zpg
{
	static const char FILE_SIGN[];

public:
    Zpg();
    ~Zpg();

    bool open(std::string File);
    void close();
    bool saveToFile(std::string File);

    bool addFromFile(std::string FromFullPath, std::string ToFullPath, bool Overwrite = false);
    bool addFromMemory(const unsigned char *pData, const unsigned long Size, std::string FullPath, bool Overwrite = false); // Can't be >2GiB
    bool removeFile(std::string FullPath);
    bool moveFile(std::string OldFullPath, std::string NewFullPath);

    void unloadData(std::string FullPath);
    const unsigned char* getFileData(std::string FullPath, unsigned long *pFileSize);
    const std::map<std::string, ZpgFile*>& getFiles() const { return m_mFiles; }

    static inline std::string toString(const unsigned char *pData, unsigned long Size)
    {
    	return std::string(reinterpret_cast<const char*>(pData), Size);
    }

    void unloadAll();

protected:
    std::map<std::string, ZpgFile*> m_mFiles;
    std::ifstream m_PackageFile;

private:
    bool exists(std::string FullPath) const;
    void swap(unsigned char *pData, unsigned long Size) const;
    bool decompressFileData(ZpgFile *pZpgFile);
};

#endif // LIBZPG_HPP
