#include "io/FileMod.h"
#include <sstream>
#include <iomanip>
#include "io/IFile.h"

namespace Bluetec {

FileMod::FileMod()
{
	this->file = NULL;
	this->mod = 1000; // default
}

FileMod::~FileMod()
{
	//this->file o responsavel por criar file que o destrua...
}

void FileMod::setFile(IFile *file)
{
	this->file = file;
}

void FileMod::SetMod(uint32_t mod)
{
	this->mod = mod;
}

std::string FileMod::getIndice(uint32_t veioid)
{
	std::stringstream indice;
	indice << std::setfill('0') << std::setw(3) << veioid % this->mod;
	return indice.str();
}

void FileMod::SetPath(std::string path)
{
	this->file->SetPath(path);
}

std::string FileMod::getDirectory(uint32_t veioid)
{
	std::stringstream directory;
	directory << this->getIndice(veioid) << "/" << veioid;
	return directory.str();
}

void FileMod::SaveBufferFileVeioid(uint32_t veioid, const char *bufferFile, int sizeBufferFile, std::string nameFile)
{
	std::string pathFile = this->getDirectory(veioid) + "/" + nameFile;

	try
	{
		this->file->SaveBufferFile(pathFile, bufferFile, sizeBufferFile);
	}
	catch(IFile::PathNotDefinedException &)
	{
		this->file->CreateDir(this->getIndice(veioid));
		this->file->CreateDir(this->getDirectory(veioid));
		this->SaveBufferFileVeioid(veioid, bufferFile, sizeBufferFile, nameFile);
	}
}

bool FileMod::GetBufferFileVeioId(uint32_t veioid, std::string nameFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	std::string pathFile = this->getDirectory( veioid ) + "/" + nameFile;
	return this->file->GetBufferFile(pathFile, bufferFile, sizeBufferFile);
}

void FileMod::SaveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile)
{
	this->file->SaveBufferFile(pathFile, bufferFile, sizeBufferFile);
}

bool FileMod::GetBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	return this->file->GetBufferFile(pathFile, bufferFile, sizeBufferFile);
}

void FileMod::DelFileVeioId(uint32_t veioid, std::string nameFile)
{
	std::string pathFile = this->getDirectory(veioid) + "/" + nameFile;
	this->file->DelFile(pathFile);
}

void FileMod::RenameFile(std::string pathFileOld, std::string pathFileNew)
{
	this->file->RenameFile(pathFileOld, pathFileNew);
}

uint32_t FileMod::GetSizeFile(uint32_t veioid, std::string nameFile)
{
	std::string pathFile = this->getDirectory(veioid) + "/" + nameFile;
	return this->file->GetSizeFile(veioid, pathFile);
}
}
