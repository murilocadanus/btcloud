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

class File : public IFile
{
	public:

		File();
		virtual ~File();

		/*
		 * Set default path
		 */
		virtual void setPath(std::string path);

		/*
		 * Save buffer at a file
		 */
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		/*
		 * Return file in a buffer
		 */
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		 * Remove file from disk
		 */
		virtual void delFile(std::string pathFile);

		/*
		 * Rename file at disk
		 */
		virtual void renameFile(std::string pathFileOld, std::string pathFileNew);

		/*
		 * Return file size
		 */
		virtual uint32_t getSizeFile(uint32_t veioid, std::string nameFile);

		virtual void createDir(std::string directory);

	private:
		std::string path;
		DIR *dirPath;
		int dirFd;


};

} /* namespace bluetec */

#endif /* FILE_H_ */
