#include "BTCloudApp.hpp"
#include "Configuration.hpp"
#include "Protocolo.h"

#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>

#define REVGEO_TAG "[BlueTec400] "

#define PATH "/home/murilo/Documents/bluetec/data/"

using namespace std;

BTCloudApp::BTCloudApp()
{
}

BTCloudApp::~BTCloudApp()
{
}

void ProcessPackage(string path)
{
	protocolo::processa_pacote((char*)path.c_str(), path.length());
}

void ProcessCall(string path)
{
	Info(REVGEO_TAG "%s - Processing...", pConfiguration->GetTitle().c_str());
	Info(REVGEO_TAG "%s - Path %s ", pConfiguration->GetTitle().c_str(), path.c_str());

	const char *buffer = path.c_str();
	int length = path.length();

	FILE *in = 0;
	unsigned char bt4[6500];
	std::string sbt4;

	// Get plate from path
	string plate(buffer);
	plate = plate.substr(0, plate.find_last_of( "/" ));
	plate = plate.substr(plate.find_last_of( "/" ) + 1, plate.length());
	transform(plate.begin(), plate.end(), plate.begin(), ::toupper);

	// Get file name
	string file(buffer);
	file = file.substr(file.find_last_of( "/" ) + 1, file.length());
	transform(file.begin(), file.end(), file.begin(), ::toupper);
	Info(REVGEO_TAG "%s - File being processed: %s", pConfiguration->GetTitle().c_str(), file.c_str());

	// Verify the file name with BT4 string
	if(file.length() < 3 || (file.substr( 0, 3 ) != "BT4"))
	{
		Error(REVGEO_TAG "%s - Ignoring file without BT4 format...", pConfiguration->GetTitle().c_str());
		return;
	}

	// Open file as with read permission
	in = fopen(buffer, "rb");

	if(in)
	{
		Info(REVGEO_TAG "%s - Loading file", pConfiguration->GetTitle().c_str());

		size_t lSize;
		size_t result;

		fseek(in, 0, SEEK_END);

		lSize = ftell(in);

		rewind(in);

		result = fread(bt4, 1, lSize, in);

		if((result != lSize) || (lSize == 0))
		{
			Error(REVGEO_TAG "%s - Failed to read file", pConfiguration->GetTitle().c_str());
			return;
		}

		std::string sbt4(bt4, bt4 + lSize);

		size_t pos = 0;
		int file = (int ) sbt4[1];

		cout << "DEBUG -- ARQUIVO " << dec << file << endl;

		long ponteiroIni = ( (long ) bt4[2] * 65536 ) + ( (long ) bt4[3] * 256 ) + (long ) bt4[4];
		cout << "DEBUG -- PONTEIRO INI " << (long ) bt4[2] << "*65536+" << (long ) bt4[3] << "*256+" << (long ) bt4[4] << "=" << dec << ponteiroIni << endl;

		long ponteiroFim = ( (long ) bt4[lSize - 8] * 65536 ) + ( (long ) bt4[lSize - 7] * 256 ) + (long ) bt4[lSize - 6];
		cout << "DEBUG -- PONTEIRO FIM " << dec << ponteiroFim << endl;

		sbt4.erase( 0, 5 );
		sbt4.erase( sbt4.length() - 8, 8 );
		lSize -= 13;

		for(size_t i = 1200; i < sbt4.length(); i += 1200 )
		{

			sbt4.erase( i, 13 ); // apagaa os 13 bytes (5 de início e 8 de fim) de uma transição de um subpacote para outro
			lSize -= 13;
		}

		puts( "DEBUG -- INICIANDO LAPSOS" );
		//int tipo_dado = bluetec::enumDataType::DADOS;
		size_t inicio = 0;

		// HEADER OFFSETS:
		size_t lFimTrecho = 12;
		size_t lHsync = 21;
		size_t lHsyns = 21;
		size_t lHfull = 250;

		string fimTrecho;
		fimTrecho.push_back( (unsigned char ) 0x82 );
		fimTrecho.push_back( (unsigned char ) 0xA3 );
		fimTrecho.push_back( (unsigned char ) 0xA5 );
		fimTrecho.push_back( (unsigned char ) 0xA7 );

		cout << "DEBUG -- BUSCANSO CABECALHOS... " << endl;

		fclose(in);
	}
}

bool BTCloudApp::Initialize()
{
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	cEventFileSystem.setListenerParam(EventFileSystem::RECURSIVE_PATH);
	cEventFileSystem.setPath(pConfiguration->GetAppListeningPath());

	cEventFileSystem.setListnerEventFileSystem(ProcessPackage);
	cEventFileSystem.start();

	cFileManager.setPath(PATH);

	return true;
}

bool BTCloudApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());
	return true;
}
