/*
 * BlueTecFileManager.cpp
 *
 *  Created on: 22/10/2013
 *      Author: david.campos
 */

#include "BlueTecFileManager.h"
#include <sstream>
#include <string.h>
#include <chrono>
#include "SwapFile.h"

namespace bluetec {

BlueTecFileManager::BlueTecFileManager()
{
	this->files.setFile(&swap);
}

BlueTecFileManager::BlueTecFileManager(std::string path)
{
	this->files.setFile(&swap);
	this->setPath(path);
}

BlueTecFileManager::~BlueTecFileManager()
{

}

void BlueTecFileManager::setPath(std::string path)
{
	this->files.setPath(path);
}

bool BlueTecFileManager::getBufferFile(uint32_t veioid, uint32_t pointer, uint16_t file, char *bufferFile, uint32_t& sizeBufferFile, struct sBluetecHeaderFile &bluetecHeaderFile)
{
	bool retorno = false;

	std::stringstream nameFile;

	/* Pesquisa o pointer e o file no indice de um determinado veioid */
	if(getsBluetecHeaderFile(veioid, pointer, file, bluetecHeaderFile))
	{
		nameFile << bluetecHeaderFile.headerFile.id;
		/* Busca o arquivo no diretorio e retorna o buffer... */
		retorno = this->files.getBufferFileVeioid(veioid, nameFile.str(), bufferFile, sizeBufferFile);
	}

	return retorno;
}

/*
 * Salva o buffer em um arquivo sempre novo.
 */
void BlueTecFileManager::saveBufferFile(uint32_t veioid, const char *bufferFile, uint32_t sizeBufferFile, struct sBluetecHeaderFile& bluetecHeaderFile)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();

	uint64_t nano = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

	std::cout << nano << std::endl;

	std::vector<struct sBluetecHeaderFile*> *vec = this->getListsBluetecHeaderFile(veioid);

	struct sBluetecHeaderFile *header = new sBluetecHeaderFile();

	*header = bluetecHeaderFile;

	header->headerFile.id = nano; // nome do arquivo.

	vec->push_back(header);

	std::cout << "Vec size: " << vec->size() << std::endl;

	std::stringstream nameFile;

	nameFile << header->headerFile.id;

	this->files.saveBufferFileVeioid(veioid, bufferFile, sizeBufferFile, nameFile.str());

	this->saveIndexFile(veioid, vec);

	this->deleteIndexFileObject(vec);
}

/*
 * Remove o arquivo fisicamente.
 */
void BlueTecFileManager::delFile(uint32_t veioid, sHeaderFile file)
{
	std::stringstream nameFile;

	std::vector<struct sBluetecHeaderFile*>::iterator it;

	std::vector<struct sBluetecHeaderFile*> *vec = this->getListsBluetecHeaderFile(veioid);

	nameFile << file.id;

	for(it = vec->begin(); it != vec->end(); it++)
	{
		if((*it)->headerFile.id == file.id)
		{
			delete *it;
			vec->erase(it);
			this->files.delFileVeioid( veioid, nameFile.str() );
			this->saveIndexFile(veioid, vec);
			break;
		}
	}

	this->deleteIndexFileObject(vec);
}

/*
 * Retorna o header para um determinado veioid pesquisando pelo pointer e file.
 */
bool BlueTecFileManager::getsBluetecHeaderFile(uint32_t veioid, uint32_t pointer, uint16_t file, struct sBluetecHeaderFile& bluetecHeaderFile)
{
	bool retorno = false;

	std::vector<struct sBluetecHeaderFile*>::iterator it;

	std::vector<struct sBluetecHeaderFile*> *vec = this->getListsBluetecHeaderFile(veioid);

	for(it = vec->begin(); it != vec->end(); it++)
	{
		if( ((*it)->beginPointer == pointer || (*it)->endPointer == pointer) && (*it)->file == file)
		{
			retorno = true;
			bluetecHeaderFile = *(*it);
			break;
		}
	}

	this->deleteIndexFileObject(vec);

	return retorno;
}

/*
 * Retorna a lista de header para um determinado veioid.
 */
std::vector<struct sBluetecHeaderFile*> *BlueTecFileManager::getListsBluetecHeaderFile(uint32_t veioid)
{
	std::vector<struct sBluetecHeaderFile*> *listBluetecHeaderFile = new std::vector<struct sBluetecHeaderFile*>();

	struct sBluetecHeaderFile *pHeader, *he;

	uint32_t sizeBufferFileHeader = 0, size;

	char *bufferFileHeader = NULL;

	try
	{
		size = this->files.getSizeFile( veioid, NAME_INDEX_FILE );

		bufferFileHeader = new char[size];auto duration = std::chrono::system_clock::now().time_since_epoch();

		if(this->files.getBufferFileVeioid( veioid, NAME_INDEX_FILE, bufferFileHeader, sizeBufferFileHeader ))
		{
			pHeader = (struct sBluetecHeaderFile*)bufferFileHeader;

			while((pHeader - (struct sBluetecHeaderFile*)bufferFileHeader) * sizeof(sBluetecHeaderFile) < sizeBufferFileHeader)
			{
				he = new sBluetecHeaderFile();
				//std::cout << "Id: " << pHeader->id << " File: " << pHeader->file << " BeginPointer: " << pHeader->beginPointer << " EndPointer: " << pHeader->endPointer << " Timestamp: " << pHeader->timestamp << std::endl;
				*he = *pHeader;
				listBluetecHeaderFile->push_back(he);
				pHeader++;
			}
		}

		delete[] bufferFileHeader;
	}
	catch(IFile::FileNotFoundException &)
	{
		std::cout << "Arquivo de index nao existe... Tudo bem ele sera criado..." << std::endl;
	}
	catch(IFile::FileReadException &)
	{
		if( bufferFileHeader != NULL )
		{
			delete[] bufferFileHeader;
		}
		std::cout << "Nao foi possivel ler o arquivo por algum motivo... " << std::endl;
	}

	return listBluetecHeaderFile;
}

/*
 * Salva alteracao no indice.
 */
void BlueTecFileManager::saveIndexFile(uint32_t veioid, std::vector<struct sBluetecHeaderFile*> *listBluetecHeaderFile)
{
	struct sBluetecHeaderFile *ph;
	/* Faz uma limpeza nos arquivos... */
	this->vacuum( veioid, listBluetecHeaderFile );

	int size = listBluetecHeaderFile->size() * sizeof(struct sBluetecHeaderFile);

	char *buffer = new char[size];

	ph = (struct sBluetecHeaderFile*) buffer;

	for(std::vector<struct sBluetecHeaderFile*>::iterator it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		memcpy(ph,(*it),sizeof(struct sBluetecHeaderFile));
		ph++;
	}

	this->files.saveBufferFileVeioid(veioid, buffer, size, NAME_INDEX_FILE);

	delete[] buffer;
}

void BlueTecFileManager::deleteIndexFileObject(std::vector<struct sBluetecHeaderFile*> *listBluetecHeaderFile)
{
	std::vector<struct sBluetecHeaderFile*>::iterator it;
	for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		delete *it;
	}

	delete listBluetecHeaderFile;
}

uint32_t BlueTecFileManager::getNextIdTrecho()
{
	char buffer[5] = {0};
	uint32_t sizeBufferFile = 0, sequence = 0;

	//Interface...
	if(this->files.getBufferFile(SEQUENCE_TRECHO, buffer, sizeBufferFile))
	{
		memcpy(&sequence, buffer, 4);

		uint32_t tmp = sequence;

		std::cout << "sequence: " << sequence << std::endl;

		tmp++;

		memcpy(buffer, &tmp, 4);

		// Salva a alteração...
		this->files.saveBufferFile(SEQUENCE_TRECHO, buffer, 4);
	}
	else
	{
		// se o arquivo ainda nao existe então cria...
		this->files.saveBufferFile(SEQUENCE_TRECHO, buffer, 4);
	}

	return sequence;
}

bool BlueTecFileManager::getHfull(uint32_t veioid, sHfull& hfull)
{
	bool retorno = false;
	char buffer[255] = {0};
	uint32_t sizeBufferFile = 0;

	if(this->files.getBufferFileVeioid(veioid,NAME_HFULL_FILE, buffer, sizeBufferFile))
	{
		memcpy(&hfull, buffer, sizeBufferFile);
		retorno = true;
	}

	return retorno;

}

void BlueTecFileManager::saveHfull(uint32_t veioid, sHfull& hfull)
{
	this->files.saveBufferFileVeioid(veioid,(char*)&hfull,sizeof(sHfull), NAME_HFULL_FILE);
}

void BlueTecFileManager::vacuum(uint32_t veioid, std::vector<struct sBluetecHeaderFile*> *listBluetecHeaderFile)
{
	std::vector<struct sBluetecHeaderFile*>::iterator it;
	std::vector<struct sBluetecHeaderFile*> listTemp;

	for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		auto duration = std::chrono::system_clock::now().time_since_epoch();

		std::chrono::nanoseconds s ((*it)->headerFile.id);

		//Se o arquivo estiver la mais de 7 dias, adiciona na lista para remover.
		if(((std::chrono::duration_cast<std::chrono::seconds>(duration).count() - std::chrono::duration_cast<std::chrono::seconds>(s).count()) / 86400) >= 7)
		{
			listTemp.push_back(*it);
		}
	}

	for(sBluetecHeaderFile *i : listTemp)
	{
		for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
		{
			if(i == *it)
			{
				std::stringstream nameFile;
				nameFile << (*it)->headerFile.id;
				delete *it;
				listBluetecHeaderFile->erase(it);
				this->files.delFileVeioid( veioid, nameFile.str() );
				break;
			}
		}
	}
}

} /* namespace bluetec */

