/*
 * SwapFile.cpp
 *
 *  Created on: 22/11/2013
 *      Author: david.campos
 */

#include "io/SwapFile.h"

namespace bluetec {

SwapFile::SwapFile()
{

}

SwapFile::~SwapFile()
{

}

void SwapFile::saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile)
{
	std::string pathFileSwap = pathFile + "~";
	File::saveBufferFile(pathFileSwap, bufferFile, sizeBufferFile);
	this->renameFile(pathFileSwap, pathFile);
}

/*
 * Teoricamente se o arquivo de swap existe mas o seu conteudo esta zerado,
 * o arquivo de swap foi truncado por algum motivo, logo o arquivo original
 * esta desatualizado.
 */
bool SwapFile::getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	bool retorno = false;
	std::string pathFileSwap = pathFile + "~";
	retorno = File::getBufferFile(pathFileSwap, bufferFile, sizeBufferFile);

	/* se existir o arquivo...*/
	if(retorno && sizeBufferFile > 0)
	{
		/* Se tiver conteudo entao pode renomear... */
		this->renameFile(pathFileSwap, pathFile);
	}
	else
	{
		/* se nao achou o swap entao abre o original... */
		retorno = File::getBufferFile(pathFile, bufferFile, sizeBufferFile);
	}

	return retorno;
}

} /* namespace bluetec */
