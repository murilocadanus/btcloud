/*
 * BlueTecFileManager.cpp
 *
 *  Created on: 22/10/2013
 *      Author: david.campos
 */

#include <sstream>
#include <string.h>
#include <chrono>
#include <util/Log.hpp>
#include "managers/BlueTecFileManager.h"
#include "io/SwapFile.h"
#include "mongo/client/dbclient.h" // for the driver

#define TAG "[BlueTecFileManager] "

namespace Bluetec {

BlueTecFileManager::BlueTecFileManager()
{
	this->files.setFile(&swap);
}

BlueTecFileManager::BlueTecFileManager(std::string path)
{
	this->files.setFile(&swap);
	this->SetPath(path);
}

BlueTecFileManager::~BlueTecFileManager()
{

}

void BlueTecFileManager::SetPath(std::string path)
{
	this->files.SetPath(path);
}

bool BlueTecFileManager::getBufferFile(uint32_t veioid, uint32_t pointer, uint16_t file, char *bufferFile, uint32_t& sizeBufferFile, struct HeaderDataFile &bluetecHeaderFile, uint32_t timestamp)
{
	bool retorno = false;

	std::stringstream nameFile;

	// Search a point and file in a index of specified veioid
	if(GetsBluetecHeaderFile(veioid, pointer, file, bluetecHeaderFile, timestamp))
	{
		nameFile << bluetecHeaderFile.headerFile.id;

		// Search the file in the directory and return the buffer
		retorno = this->files.GetBufferFileVeioId(veioid, nameFile.str(), bufferFile, sizeBufferFile);
	}

	return retorno;
}

// Save buffer in a new file
void BlueTecFileManager::SaveBufferFile(uint32_t veioid, const char *bufferFile, uint32_t sizeBufferFile, struct HeaderDataFile& bluetecHeaderFile)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	uint64_t nano = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

	Dbg(TAG "%d", nano);

	std::vector<struct HeaderDataFile*> *vec = this->GetListsBluetecHeaderFile(veioid);
	struct HeaderDataFile *header = new HeaderDataFile();

	*header = bluetecHeaderFile;
	header->headerFile.id = nano; // file name
	vec->push_back(header);

	Dbg(TAG "Vec size: %d", vec->size());

	std::stringstream nameFile;
	nameFile << header->headerFile.id;

	this->files.SaveBufferFileVeioid(veioid, bufferFile, sizeBufferFile, nameFile.str());
	this->SaveIndexFile(veioid, vec);
	this->DeleteIndexFileObject(vec);
}

void BlueTecFileManager::RenameFile(std::string pathFileOld, std::string pathFileNew)
{
	this->files.RenameFile(pathFileOld, pathFileNew);
}

void BlueTecFileManager::DelFile(uint32_t veioid, DataFile file)
{
	std::stringstream nameFile;
	std::vector<struct HeaderDataFile*>::iterator it;
	std::vector<struct HeaderDataFile*> *vec = this->GetListsBluetecHeaderFile(veioid);

	nameFile << file.id;

	for(it = vec->begin(); it != vec->end(); it++)
	{
		if((*it)->headerFile.id == file.id)
		{
			delete *it;
			vec->erase(it);
			this->files.DelFileVeioId(veioid, nameFile.str());
			this->SaveIndexFile(veioid, vec);
			break;
		}
	}

	this->DeleteIndexFileObject(vec);
}

bool BlueTecFileManager::GetsBluetecHeaderFile(uint32_t veioid, uint32_t pointer, uint16_t file, struct HeaderDataFile& bluetecHeaderFile, uint32_t timestamp)
{
	bool retorno = false;

	std::vector<struct HeaderDataFile*>::iterator it;

	std::vector<struct HeaderDataFile*> *vec = this->GetListsBluetecHeaderFile(veioid);


	for(it = vec->begin(); it != vec->end(); it++)
	{
		Error("Pointers: (%d == %d|%d) header type: %d timestamp: %d", pointer, (*it)->beginPointer, (*it)->endPointer, (*it)->dataType, (*it)->timestamp);
		if((((*it)->beginPointer == pointer || (*it)->endPointer == pointer) || timestamp == (*it)->timestamp) && (*it)->file == file)
		{
			Error("Found: (%d == %d|%d) header type: %d timestamp: %d == %d", pointer, (*it)->beginPointer, (*it)->endPointer, (*it)->dataType, timestamp, (*it)->timestamp);
			retorno = true;
			bluetecHeaderFile = *(*it);
			break;
		}
	}

	this->DeleteIndexFileObject(vec);

	return retorno;
}

std::vector<struct HeaderDataFile*> *BlueTecFileManager::GetListsBluetecHeaderFile(uint32_t veioid)
{
	std::vector<struct HeaderDataFile*> *listBluetecHeaderFile = new std::vector<struct HeaderDataFile*>();
	struct HeaderDataFile *pHeader, *he;
	uint32_t sizeBufferFileHeader = 0, size;
	char *bufferFileHeader = NULL;

	try
	{
		size = this->files.GetSizeFile(veioid, NAME_INDEX_FILE);
		bufferFileHeader = new char[size];auto duration = std::chrono::system_clock::now().time_since_epoch();

		if(this->files.GetBufferFileVeioId(veioid, NAME_INDEX_FILE, bufferFileHeader, sizeBufferFileHeader))
		{
			pHeader = (struct HeaderDataFile*)bufferFileHeader;

			while((pHeader - (struct HeaderDataFile*)bufferFileHeader) * sizeof(HeaderDataFile) < sizeBufferFileHeader)
			{
				he = new HeaderDataFile();
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
		//Error(TAG "Warning: Index of file does not exist");
	}
	catch(IFile::FileReadException &)
	{
		if( bufferFileHeader != NULL )
		{
			delete[] bufferFileHeader;
		}
		Error(TAG "Can not read file");
	}

	return listBluetecHeaderFile;
}

void BlueTecFileManager::SaveIndexFile(uint32_t veioid, std::vector<struct HeaderDataFile*> *listBluetecHeaderFile)
{
	struct HeaderDataFile *ph;

	// Clean files
	this->Vacuum(veioid, listBluetecHeaderFile);

	int size = listBluetecHeaderFile->size() * sizeof(struct HeaderDataFile);
	char *buffer = new char[size];
	ph = (struct HeaderDataFile*) buffer;

	for(std::vector<struct HeaderDataFile*>::iterator it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		memcpy(ph,(*it),sizeof(struct HeaderDataFile));
		ph++;
	}

	this->files.SaveBufferFileVeioid(veioid, buffer, size, NAME_INDEX_FILE);

	delete[] buffer;
}

void BlueTecFileManager::DeleteIndexFileObject(std::vector<struct HeaderDataFile*> *listBluetecHeaderFile)
{
	std::vector<struct HeaderDataFile*>::iterator it;
	for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		delete *it;
	}

	delete listBluetecHeaderFile;
}

uint32_t BlueTecFileManager::GetNextIdRoute()
{
	char buffer[5] = {0};
	uint32_t sizeBufferFile = 0, sequence = 0;

	// Interface
	if(this->files.GetBufferFile(SEQUENCE_TRECHO, buffer, sizeBufferFile))
	{
		memcpy(&sequence, buffer, 4);
		uint32_t tmp = sequence;

		Dbg(TAG "Sequence: %d", sequence);

		tmp++;
		memcpy(buffer, &tmp, 4);

		// Save changes
		this->files.SaveBufferFile(SEQUENCE_TRECHO, buffer, 4);
	}
	else
	{
		// Create file if it not exist
		this->files.SaveBufferFile(SEQUENCE_TRECHO, buffer, 4);
	}

	return sequence;
}

bool BlueTecFileManager::GetHfull(uint32_t veioid, HFull& hfull)
{
	bool retorno = false;
	char buffer[255] = {0};
	uint32_t sizeBufferFile = 0;

	if(this->files.GetBufferFileVeioId(veioid,NAME_HFULL_FILE, buffer, sizeBufferFile))
	{
		memcpy(&hfull, buffer, sizeBufferFile);
		retorno = true;
	}

	return retorno;

}

void BlueTecFileManager::SaveHfull(uint32_t veioid, HFull& hfull)
{
	this->files.SaveBufferFileVeioid(veioid,(char*)&hfull,sizeof(HFull), NAME_HFULL_FILE);
}

void BlueTecFileManager::Vacuum(uint32_t veioid, std::vector<struct HeaderDataFile*> *listBluetecHeaderFile)
{
	std::vector<struct HeaderDataFile*>::iterator it;
	std::vector<struct HeaderDataFile*> listTemp;

	for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
	{
		auto duration = std::chrono::system_clock::now().time_since_epoch();

		std::chrono::nanoseconds s ((*it)->headerFile.id);

		// If the file is there more than 7 days, add it to the list to remove
		if(((std::chrono::duration_cast<std::chrono::seconds>(duration).count() - std::chrono::duration_cast<std::chrono::seconds>(s).count()) / 86400) >= 7)
		{
			listTemp.push_back(*it);
		}
	}

	for(HeaderDataFile *i : listTemp)
	{
		for(it = listBluetecHeaderFile->begin(); it != listBluetecHeaderFile->end(); it++)
		{
			if(i == *it)
			{
				std::stringstream nameFile;
				nameFile << (*it)->headerFile.id;
				delete *it;
				listBluetecHeaderFile->erase(it);
				this->files.DelFileVeioId( veioid, nameFile.str() );
				break;
			}
		}
	}
}

} // namespace

