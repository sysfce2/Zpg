#ifndef LIBZPG_HPP
#define LIBZPG_HPP

#include <zlib.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

class LibZpg
{
    struct ZFile
    {
        std::string tag;
        std::string fileName;
        unsigned long fileSize;
        uLong compSize;
        unsigned char *binary;
    };

    std::vector<ZFile> m_vZFiles;
public:
    LibZpg();
    bool add(std::string);
    bool toFile(std::string);
    bool read(std::string);
    bool load(std::string, std::string, unsigned char **);
};

#endif // LIBZPG_HPP
