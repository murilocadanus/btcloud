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

bool BTCloudApp::Initialize()
{
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	cEventFileSystem.setListenerParam(EventFileSystem::RECURSIVE_PATH);
	cEventFileSystem.setPath(pConfiguration->GetAppListeningPath());
	cFileManager.setPath(PATH);

	cEventFileSystem.setListnerEventFileSystem(ProcessPackage);
	cEventFileSystem.start();

	return true;
}

bool BTCloudApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());
	return true;
}
