/*
 * FileMod.h
 *
 *  Created on: 21/11/2013
 *      Author: david.campos
 */

#ifndef FILEMOD_H_
#define FILEMOD_H_
#include "IFile.h"

namespace bluetec {

/*
 * Classe Resposável por gerenciar os arquivos que estão sendo processados
 * de forma que seja distribuida em pastas por % e veioid.
 */
class FileMod
{
	public:
		FileMod();
		~FileMod();
		void setPath(std::string path);

		/*
		* Salva o buffer em um arquivo vinculando a um determinado veioid.
		*/
		void saveBufferFileVeioid(uint32_t veioid, const char *bufferFile, int sizeBufferFile, std::string nameFile);

		/*
		* Retorna o arquivo vinculado a um determinado veioid em um buffer.
		*/
		bool getBufferFileVeioid(uint32_t veioid, std::string nameFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		* Deleta o arquivo vinculado a um determinado veioid.
		*/
		void delFileVeioid(uint32_t veioid, std::string nameFile);

		/*
		* Métodos para delegação...
		*/
		void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		/*
		* Métodos para delegação...
		*/
		bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		* Define o valor para calcular o mod do veioid que será utlizado para criar/nomear a pasta de armazenamento.
		*/
		void setMod(uint32_t mod);

		/*
		* Define o ponteiro de IFile...
		*/
		uint32_t getSizeFile(uint32_t veioid, std::string nameFile);

		void setFile(IFile *file);

	private:
		IFile *file;
		uint32_t mod;
		std::string getIndice(uint32_t veioid);
		std::string getDirectory(uint32_t veioid);
};

} /* namespace bluetec */

#endif /* FILEMOD_H_ */
