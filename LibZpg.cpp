#include "LibZpg.hpp"
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

LibZpg::LibZpg()
{

}

bool LibZpg::add(std::string filename) // TODO: forbid to add multiple files with same name
{
	std::ifstream file(filename, std::ios::binary);
	if(file.is_open())
	{
		ZFile zfile;
		zfile.fileName = filename;

		size_t pos = filename.find_last_of(".");
		std::string ext;
		if(pos != std::string::npos)
			ext = filename.substr(filename.find_last_of("."), filename.length()-pos);
		if(ext == ".png")
			zfile.tag = "IMAGE";
		else if(ext == ".ogg")
			zfile.tag = "SOUND";
		else if(ext == ".txt")
			zfile.tag = "TEXT";

		file.seekg(0, std::ios::end);
		zfile.fileSize = file.tellg();
		file.seekg(0, std::ios::beg);
		char input[zfile.fileSize];
		file.read(input, zfile.fileSize);
		file.close();
		zfile.compSize = compressBound(zfile.fileSize);
		zfile.binary = new unsigned char[zfile.compSize];
		compress((Bytef*)zfile.binary, &zfile.compSize, (Bytef*)input, zfile.fileSize);
		m_vZFiles.push_back(zfile);

		return true;
	}
	else return false;
}

bool LibZpg::toFile(std::string output)
{
	std::ofstream file(output, std::ios::binary);
	if(file.is_open())
	{
		for(unsigned int i=0; i<m_vZFiles.size(); i++) //TODO: order by tag
		{
			ZFile zfile = m_vZFiles.at(i);

			file << zfile.tag << " " << zfile.fileName << " " << zfile.fileSize << " " << zfile.compSize << endl;
			file.write((const char*)zfile.binary, m_vZFiles.at(i).compSize);
			file << endl;
		}
		file.close();
		return true;
	}
	else
		return false;
}

bool LibZpg::read(std::string input)
{
	std::ifstream file(input, std::ios::binary);
	if(!file.is_open())
	{
		cerr << "File " << input << " not found" << endl;
		return false;
	}

	file.seekg (0, file.end);
	unsigned long length = file.tellg();
	file.seekg (0, file.beg);

	while(file.tellg() < length) // (!file.eof()) causes a last unwanted iteration
	{
		ZFile zfile;

		file >> zfile.tag >> zfile.fileName >> zfile.fileSize >> zfile.compSize;
		file.ignore(1);
		zfile.binary = new unsigned char[zfile.compSize];
		file.read((char *)zfile.binary, zfile.compSize);
		file.ignore(1);
		unsigned char output[zfile.fileSize];
		uncompress((Bytef*)output, &zfile.fileSize, (Bytef*)zfile.binary, zfile.compSize);

        delete zfile.binary;


		cout << zfile.tag << ": " << zfile.fileName << " (" << zfile.fileSize << " bytes)" << endl;

		//// extracts packed files in 'out' folder (it must exists)
		std::ofstream out("out/"+zfile.fileName, std::ios::binary);
		out << output;
	}
	file.close();
	return true;
}

bool LibZpg::load(std::string zipname, std::string filename, unsigned char ** buffer)
{
	std::ifstream file(zipname, std::ios::binary);
	if(!file.is_open())
	{
		cerr << "File " << zipname << " not found" << endl;
		return false;
	}

	file.seekg (0, file.end);
	unsigned long length = file.tellg();
	file.seekg (0, file.beg);
	while(file.tellg() < length) // (!file.eof()) causes a last unwanted iteration
	{
		ZFile zfile;

		file >> zfile.tag >> zfile.fileName >> zfile.fileSize >> zfile.compSize;
		file.ignore(1);
		if(filename == zfile.fileName)
		{
			zfile.binary = new unsigned char[zfile.compSize];
			file.read((char *)zfile.binary, zfile.compSize);
			file.ignore(1);
			*buffer = new unsigned char[zfile.fileSize];
			uncompress((Bytef*)*buffer, &zfile.fileSize, (Bytef*)zfile.binary, zfile.compSize);

            delete zfile.binary;
			file.close();
			return true;
		}
		else
			file.ignore(zfile.compSize);
	}
	return false;
}
