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
 * 		console_packer mypack.zpg -L						> This list files inside 'mypack.zpg'
 * 		console_packer mypack.zpg -E photo.png				> This extracts 'photo.png' from 'mypack.zpg'
 */
#include <LibZpg.hpp>
#include <cstring>
#include <iostream>

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
			std::cout << "Invalid ZPG File!" << std::endl;
			return -1;
		}

		if (aAddContentPath[0] != 0)
		{
			std::string path(aAddContentPath);
			std::size_t delPos = path.find_last_of("/\\");

			if (!myZ.addFromFile(aAddContentPath, (delPos == std::string::npos)?path.c_str():path.substr(path.find_last_of("/\\")+1).c_str()))
				std::cout << "Can't add '" << aAddContentPath << "' to package!" << std::endl;
		}

		if (aExtractContentPath[0] != 0)
		{
			unsigned long fileSize = 0;
			const unsigned char* pData = myZ.getFileData(aExtractContentPath, &fileSize);
			if (!pData)
			{
				std::cout << "File '" << aExtractContentPath << "' not found!" << std::endl;
			}
			else
			{
				std::ofstream file(aExtractContentPath, std::ios::binary);
				if (file.is_open())
				{
					file.write(reinterpret_cast<const char*>(pData), fileSize);
					file.close();
					std::cout << "File '" << aExtractContentPath << "' successfully extracted" << std::endl;
				}
				else
				{
					std::cout << "Can't extract '" << aExtractContentPath << "'" << std::endl;
				}
			}
			delete [] pData;
		}

		if (listMode)
		{
			const std::vector<ZpgFileHeader> &Files = myZ.getFilesInfo();
			std::cout << "Num Files: " << Files.size() << std::endl;
			std::vector<ZpgFileHeader>::const_iterator cit = Files.begin();
			while (cit != Files.end())
			{
				std::cout << std::dec << (*cit).m_aFullPath << " [CSize: " << (*cit).m_FileSizeComp << "][Size: " << (*cit).m_FileSize << "]" << "[StartAt: 0x" << std::hex << std::uppercase << (*cit).m_FileStart << "]" << std::nouppercase << std::endl;
				++cit;
			}
		}

		myZ.close();
    }
    else
    {
    	std::cout << "Invalid Parameters!" << std::endl;
    	return -1;
    }

    return 0;
}
