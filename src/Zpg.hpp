/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef ZPG_HPP
#define ZPG_HPP

#include <map>
#include <vector>
#include <string>


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

    bool load(std::string File);
    bool saveToFile(std::string File, int numIterations = 15);

    bool addFromFile(std::string FromFullPath, std::string ToFullPath);
    bool addFromMemory(const unsigned char *pData, unsigned long size, std::string FullPath); // Can't be >2GiB

    const unsigned char* getFileData(std::string FullPath, unsigned long *pFileSize) const;
    const std::map<std::string, ZpgFile*>& getFiles() const { return m_mFiles; }

    static inline std::string toString(const unsigned char *pData, unsigned long size)
    {
    	return std::string(reinterpret_cast<const char*>(pData), size);
    }

    void unloadAll();

protected:
    unsigned short m_PackageVersion;
    std::map<std::string, ZpgFile*> m_mFiles;

private:
    bool exists(std::string FullPath) const;
};

#endif // LIBZPG_HPP
