/*
 * SwapFile.h
 *
 *  Created on: 22/11/2013
 *      Author: david.campos
 */

#ifndef SWAPFILE_H_
#define SWAPFILE_H_
#include "File.h"

namespace bluetec {

/*
 * Extendendo a funcionalidade de File, esta classe é especializada em
 * salvar os arquivos em disco garantindo uma cópia em swap.
 */
class SwapFile : public File
{
	public:
		SwapFile();
		virtual ~SwapFile();
		/*
		 * Salva o buffer em um arquivo de swap e posteriormente move para o arquivo original.
		 */
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);
		/*
		 * Retorna o arquivo em um buffer.
		 */
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);
};

} /* namespace bluetec */

#endif /* SWAPFILE_H_ */
