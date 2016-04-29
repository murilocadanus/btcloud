#include "io/SwapFile.h"

namespace Bluetec {

SwapFile::SwapFile()
{

}

SwapFile::~SwapFile()
{

}

void SwapFile::SaveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile)
{
	std::string pathFileSwap = pathFile + "~";
	File::SaveBufferFile(pathFileSwap, bufferFile, sizeBufferFile);
	this->RenameFile(pathFileSwap, pathFile);
}

bool SwapFile::GetBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	bool retorno = false;
	std::string pathFileSwap = pathFile + "~";
	retorno = File::GetBufferFile(pathFileSwap, bufferFile, sizeBufferFile);

	// Case file is found
	if(retorno && sizeBufferFile > 0)
	{
		// If file contains data, it can be renamed
		this->RenameFile(pathFileSwap, pathFile);
	}
	else
	{
		// Case it is not found, open original file
		retorno = File::GetBufferFile(pathFile, bufferFile, sizeBufferFile);
	}


	return retorno;
}

} // namespace
