/*
 * File.cpp
 *
 *  Created on: 21/10/2013
 *      Author: david.campos
 */

#include "File.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace bluetec {

File::File()
{
	this->dirFd = 0;
	this->dirPath = NULL;
}

File::~File()
{
	if( this->dirPath )
	{
		closedir( this->dirPath );
	}
}

void File::setPath(std::string path)
{
	this->path = path;

	if( this->dirFd )
	{
		close( this->dirFd );
		this->dirFd = 0;
	}

	if( this->dirPath )
	{
		closedir( this->dirPath );
		this->dirPath = NULL;
	}

	if(access(this->path.c_str(), R_OK | W_OK) != 0)
	{
		std::cout << strerror( errno ) << " - " << this->path << std::endl;
		abort();
	}

	this->dirPath = opendir( this->path.c_str() );

	if( !this->dirPath )
	{
		throw PathNotFoundException();
	}

	this->dirFd = dirfd( this->dirPath );

	if( !this->dirFd )
	{
		throw PathNotFoundException();
	}
}

void File::createDir(std::string directory)
{
	int fdTemp = mkdirat( this->dirFd, directory.c_str(),  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

	std::cout << "Criando diretorio: " << directory << std::endl;

	if( fdTemp == -1 )
	{
		if(errno != EEXIST)
		{
			std::cout <<" opa " << strerror( errno ) << std::endl;
			throw PathCreateException();
		}
		std::cout << "aaui " << strerror( errno ) << std::endl;
	}
}

void File::saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile)
{
	if( this->dirPath == NULL ) throw PathNotDefinedException();

	int fd = openat( this->dirFd, pathFile.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0666 );

	if( fd != -1 )
	{
		std::cout << "gravando: " << pathFile << std::endl;

		int escrito = write( fd, bufferFile, sizeBufferFile );

		close( fd );

		if( escrito < sizeBufferFile )
		{
			throw FileWriteException();
		}
	}
	else
	{
		std::cout << strerror( errno ) << std::endl;
		if( errno == ENOSPC)
		{
			throw NotSpaceAvaiableException();
		}
		throw PathNotDefinedException();
	}
}

bool File::getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile)
{
	bool retorno = false;

	struct stat st;

	if( this->dirPath == NULL ) throw PathNotDefinedException();

	int fd = openat( this->dirFd, pathFile.c_str(), O_RDWR );

	if( fd != -1 )
	{
		std::cout << "lendo: " << pathFile << std::endl;

		fstat( fd, &st );

		sizeBufferFile = st.st_size;

		int lido = read( fd, bufferFile, st.st_size );

		close( fd );

		if( lido < st.st_size )
		{
			throw FileReadException();
		}

		retorno = true;
	}

	return retorno;
}

uint32_t File::getSizeFile(uint32_t veioid, std::string pathFile)
{
	struct stat st;

	if( this->dirPath == NULL ) throw PathNotDefinedException();

	int fd = openat( this->dirFd, pathFile.c_str(), O_RDWR );

	if( fd != -1 )
	{
		std::cout << "getSizeFile: " << pathFile << std::endl;

		fstat( fd, &st );

		close( fd );

		return st.st_size;
	}
	else
	{
		throw FileNotFoundException();
	}
}

void File::renameFile(std::string pathFileOld, std::string pathFileNew)
{
	int fd = renameat( this->dirFd, pathFileOld.c_str(), this->dirFd , pathFileNew.c_str());

	std::cout << "renameFile: " << pathFileOld << " para " << pathFileNew << std::endl;

	if( fd == -1 )
	{
		std::cout << "renameFile: " << strerror( errno ) << std::endl;
	}
}

void File::delFile(std::string pathFile)
{
	std::stringstream del;

	//del << this->path << "/" << this->getDirectory( veioid ) << "/" << nameFile;
	del << this->path << "/" << pathFile;

	std::cout << "deletando: " << del.str() << std::endl;

	remove(del.str().c_str());

}

} /* namespace bluetec */
