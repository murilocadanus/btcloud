/*
 * File.h
 *
 *  Created on: 21/10/2013
 *      Author: david.campos
 */

#ifndef FILE_H_
#define FILE_H_
#include <iostream>
#include <stdint.h>
#include <dirent.h>
#include "IFile.h"

namespace bluetec {
/*
 * Classe utilizada para manipular arquivos.
 */
class File : public IFile
{
	public:

		File();
		virtual ~File();
		/*
		 * Define o Path default.
		 */
		virtual void setPath(std::string path);
		/*
		 * Salva o buffer em um arquivo.
		 */
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);
		/*
		 * Retorna o arquivo em um buffer.
		 */
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);
		/*
		 * Remove fisicamente o arquivo.
		 */
		virtual void delFile(std::string pathFile);
		/*
		 * Retorna o tamanho do arquivo.
		 */
		virtual void renameFile(std::string pathFileOld, std::string pathFileNew);

		virtual uint32_t getSizeFile(uint32_t veioid, std::string nameFile);

		virtual void createDir(std::string directory);

	private:
		std::string path;
		DIR *dirPath;
		int dirFd;


};

} /* namespace bluetec */

#endif /* FILE_H_ */
