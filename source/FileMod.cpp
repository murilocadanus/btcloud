/*
 * FileMod.cpp
 *
 *  Created on: 21/11/2013
 *      Author: david.campos
 */

#include "FileMod.h"
#include <sstream>
#include <iomanip>
#include "IFile.h"

namespace bluetec {

FileMod::FileMod()
{
	this->file = NULL;
	this->mod = 1000; // default
}

FileMod::~FileMod()
{
	//this->file o responsável por criar file que o destrua...
}

void FileMod::setFile(IFile *file)
{
	this->file = file;
}

void FileMod::setMod(uint32_t mod)
{
	this->mod = mod;
}

std::string FileMod::getIndice(uint32_t veioid)
{
	std::stringstream indice;
	indice << std::setfill('0') << std::setw(3) << veioid % this->mod;
	return indice.str();
}

void FileMod::setPath(std::string path)
{
	this->file->setPath( path );
}

std::string FileMod::getDirectory(uint32_t veioid)
{
	std::stringstream directory;
	directory << this->getIndice( veioid ) << "/" << veioid;
	return directory.str();
}

void FileMod::saveBufferFileVeioid(uint32_t veioid, const char *bufferFile, int sizeBufferFile, std::string nameFile)
{
	std::string pathFile = this->getDirectory( veioid ) + "/" + nameFile;

	try
	{
		this->file->saveBufferFile( pathFile, bufferFile, sizeBufferFile );
	}
	catch(IFile::PathNotDefinedException &)
	{
		this->file->createDir( this->getIndice( veioid ) );
		this->file->createDir( this->getDirectory( veioid ) );
		this->saveBufferFileVeioid( veioid, bufferFile, sizeBufferFile, nameFile );
	}
}

bool FileMod::getBufferFileVeioid(uint32_t veioid, std::string nameFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	std::string pathFile = this->getDirectory( veioid ) + "/" + nameFile;
	return this->file->getBufferFile( pathFile, bufferFile, sizeBufferFile );
}

void FileMod::saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile)
{
	this->file->saveBufferFile( pathFile, bufferFile, sizeBufferFile );
}

bool FileMod::getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	return this->file->getBufferFile( pathFile, bufferFile, sizeBufferFile );
}

void FileMod::delFileVeioid(uint32_t veioid, std::string nameFile)
{
	std::string pathFile = this->getDirectory( veioid ) + "/" + nameFile;
	this->file->delFile( pathFile );
}

uint32_t FileMod::getSizeFile(uint32_t veioid, std::string nameFile)
{
	std::string pathFile = this->getDirectory( veioid ) + "/" + nameFile;
	return this->file->getSizeFile( veioid, pathFile );
}

} /* namespace bluetec */


