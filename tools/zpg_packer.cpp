/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
/*****************************************************
 * Syntaxis:
 * 		console_packer <ZPGPackageFile> <options>
 *
 * Options:
 * 		- C				> Create the ZPG Package
 * 		- A <path>		> Adds the indicate file into ZPG package
 * 		- L				> List all files inside ZPG package
 * 		- E <file>		> Extract file
 *
 * Example Usage:
 * 		console_packer mypack.zpg -C -A photo.png			> This will create a new ZPG package 'mypack.zpg' and adds the "photo.png" file
 * 		console_packer mypack.zpg -A notes.txt				> This adds into 'mypack.zpg' the "notes.txt" file
 * 		console_packer mypack.zpg -A one/folder/data/		> This adds into 'mypack.zpg' the folder "data/"
 * 		console_packer mypack.zpg -L						> This list files inside 'mypack.zpg'
 * 		console_packer mypack.zpg -E photo.png				> This extracts 'photo.png' from 'mypack.zpg'
 * 		console_packer mypack.zpg -E data/					> This extracts 'data/' folder from 'mypack.zpg'
 */
#include <LibZpg.hpp>
#include <cstring>
#include <iostream>
#if defined(__linux__)
	#include <dirent.h>
	#include <sys/stat.h>
#endif


void makeDir(const char *pPath)
{
	std::string path(pPath);
	std::size_t delPos = 0;
	while ((delPos = path.find_first_of("/\\", delPos+1)) != std::string::npos)
	{
	#if defined(__linux__)
		mkdir(path.substr(0, delPos+1).c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	#endif
	}
}

bool extractFile(LibZpg &zpg, const char *pPathFile)
{
	unsigned long fileSize = 0;
	const unsigned char* pData = zpg.getFileData(pPathFile, &fileSize);
	if (!pData)
		return false;
	else
	{
		std::string dir(pPathFile);
		dir = dir.substr(0, dir.find_last_of("/\\")+1);
		makeDir(dir.c_str());
		std::ofstream file(pPathFile, std::ios::binary);
		if (file.is_open())
		{
			file.write(reinterpret_cast<const char*>(pData), fileSize);
			file.close();
		}
		else
			return false;
	}
	delete [] pData;
	return true;
}

void extractDirectory(LibZpg &zpg, const char *pPath)
{
	const std::map<std::string, ZpgFileHeader> &Files = zpg.getFilesInfo();
	std::map<std::string, ZpgFileHeader>::const_iterator cit = Files.begin();
	while (cit != Files.end())
	{
		std::size_t fpos = (*cit).first.find(pPath, 0);
		if (fpos == 0)
		{
			bool res = extractFile(zpg, (*cit).first.c_str());
			std::cout << "Extracting '" << (*cit).first << "'... " << (res?"OK":"FAILURE!") << std::endl;
		}
		++cit;
	}
}

bool addDirectory(LibZpg &zpg, const char *pFromFullPath, const char *pToFullPath)
{
	bool hasNoErrors = true;
#if defined(__linux__)
	struct dirent *pDirent;
	DIR *pDir = opendir(pFromFullPath);
	if (!pDir)
	{
		std::cerr << "Failed to open input directory" << std::endl;
		return false;
	}

	while ((pDirent = readdir(pDir)))
	{
		if (strncmp(pDirent->d_name, ".", sizeof(pDirent->d_name)) == 0 || strncmp(pDirent->d_name, "..", sizeof(pDirent->d_name)) == 0)
			continue;

		if (pDirent->d_type == DT_DIR)
		{
			char aNewFromPath[1024], aNewToPath[1024];
			snprintf(aNewFromPath, sizeof(aNewFromPath), "%s%s/", pFromFullPath, pDirent->d_name);
			snprintf(aNewToPath, sizeof(aNewToPath), "%s%s/", pToFullPath, pDirent->d_name);
			closedir(pDir);
			return addDirectory(zpg, aNewFromPath, aNewToPath);
		}
		else
		{
			char aFileFromPath[1024], aFileToPath[1024];
			snprintf(aFileFromPath, sizeof(aFileFromPath), "%s%s", pFromFullPath, pDirent->d_name);
			snprintf(aFileToPath, sizeof(aFileToPath), "%s%s", pToFullPath, pDirent->d_name);
			bool res = zpg.addFromFile(aFileFromPath, aFileToPath);
			std::cout << "Adding '" << aFileFromPath << "'... " << (res?"OK":"FAILURE!") << std::endl;
		}
	}

	closedir(pDir);
#else
	#error Not Implemented!
#endif

	return hasNoErrors;
}

int main(int argc, char *argv[])
{
    LibZpg myZ;
    char aToFile[512] = {0};
    char aAddContentPath[1024] = {0};
    char aExtractContentPath[1024] = {0};
    bool createMode = false;
    bool listMode = false;

    if(argc>1)
    {
    	strncpy(aToFile, argv[1], sizeof(aToFile));
        for (int i=2; i<argc; i++)
        {
        	if (argv[i][0] == '-' && argv[i][1] == 'A')
        	{
        		strncpy(aAddContentPath, argv[++i], sizeof(aAddContentPath));
        	}
        	else if (argv[i][0] == '-' && argv[i][1] == 'C')
        	{
        		createMode = true;
        	}
        	else if (argv[i][0] == '-' && argv[i][1] == 'L')
        	{
        		listMode = true;
        	}
        	else if (argv[i][0] == '-' && argv[i][1] == 'E')
			{
        		strncpy(aExtractContentPath, argv[++i], sizeof(aAddContentPath));
			}
        }


		if (createMode)
		{
			bool res = myZ.create(aToFile);
			std::cout << "Creating File..." << (res?"OK":"FAILURE!") << std::endl;
		} else if (!myZ.open(aToFile))
		{
			std::cerr << "Invalid ZPG File!" << std::endl;
			return -1;
		}

		if (aAddContentPath[0] != 0)
		{
			std::string path(aAddContentPath);
			std::size_t delPos = path.find_last_of("/\\");

			if (delPos == path.size()-1) // Directory
			{
				addDirectory(myZ, aAddContentPath, "test/");
			}
			else if (!myZ.addFromFile(aAddContentPath, (delPos == std::string::npos)?path.c_str():path.substr(delPos+1).c_str())) // File
				std::cerr << "Can't add '" << aAddContentPath << "' to package!" << std::endl;
		}

		if (aExtractContentPath[0] != 0)
		{
			std::string path(aAddContentPath);
			std::size_t delPos = path.find_last_of("/\\");

			if (delPos == path.size()-1) // Directory
			{
				extractDirectory(myZ, aExtractContentPath);
			}
			else
			{
				bool res = extractFile(myZ, aExtractContentPath);
				std::cout << "Extracting '" << aExtractContentPath << "'... " << (res?"OK":"FAILURE!") << std::endl;
			}
		}

		if (listMode)
		{
			const std::map<std::string, ZpgFileHeader> &Files = myZ.getFilesInfo();
			std::cout << "Num. Files: " << Files.size() << std::endl;
			std::map<std::string, ZpgFileHeader>::const_iterator cit = Files.begin();
			while (cit != Files.end())
			{
				std::cout << std::dec << (*cit).first << " [CSize: " << (*cit).second.m_FileSizeComp << "][Size: " << (*cit).second.m_FileSize << "]" << "[StartAt: 0x" << std::hex << std::uppercase << (*cit).second.m_FileStart << "]" << std::nouppercase << std::endl;
				++cit;
			}
		}

		myZ.close();
    }
    else
    {
    	std::cerr << "Invalid Parameters!" << std::endl;
    	return -1;
    }

    return 0;
}
