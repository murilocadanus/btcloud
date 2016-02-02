#include "Bluetec400App.hpp"
#include "Configuration.hpp"
#include <algorithm>

#define REVGEO_TAG "[BlueTec400] "

#define PATH "/home/murilo/Documents/bluetec/data"

Bluetec400App::Bluetec400App()
{
}

Bluetec400App::~Bluetec400App()
{
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
	Info("File being processed: %s", file.c_str());

	// Verify the file name with BT4 string
	if(file.length() < 3 || (file.substr( 0, 3 ) != "BT4"))
	{
		Error("Arquivo não BT4 Ignorando...");
		return;
	}

	// Open file as with read permission
	in = fopen(buffer, "rb");

	if(in)
	{
		Info("Loading file");

		size_t lSize;
		size_t result;

		fseek(in, 0, SEEK_END);

		lSize = ftell(in);

		rewind(in);

		result = fread(bt4, 1, lSize, in);

		if((result != lSize) || (lSize == 0))
		{
			Error("Failed to read file");
			return;
		}

		fclose(in);
	}
}

bool Bluetec400App::Initialize()
{
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	cNotifyDirectory.setListenerParam(NotifyDirectory::RECURSIVE_PATH);
	cNotifyDirectory.setPath(PATH);

	cNotifyDirectory.setListnerNotifyDirectory(ProcessCall);
	cNotifyDirectory.start();

	cFileManager.setPath(PATH);

	return true;
}

bool Bluetec400App::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());
	return true;
}
