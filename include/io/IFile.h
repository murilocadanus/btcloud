/*
 * IFile.h
 *
 *  Created on: 22/11/2013
 *      Author: david.campos
 */

#ifndef IFILE_H_
#define IFILE_H_
#include <iostream>

namespace bluetec {

class IFile
{
	public:
		class FileNotFoundException{};
		class PathNotFoundException{};
		class FileReadException{};
		class FileWriteException{};
		class PathCreateException{};
		class PathNotDefinedException{};
		class NotSpaceAvaiableException{};
		virtual ~IFile(){};
		virtual void setPath(std::string path) = 0;
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile) =0;
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile) = 0;
		virtual void delFile(std::string pathFile) = 0;
		virtual void renameFile(std::string pathFileOld, std::string pathFileNew) = 0;
		virtual uint32_t getSizeFile(uint32_t veioid, std::string nameFile) = 0;
		virtual void createDir(std::string directory) = 0;
};

} /* namespace bluetec */

#endif /* IFILE_H_ */
