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
#include "FileSystem.hpp"

#define REVGEO_TAG "[BlueTec400] "

#define PATH "/home/murilo/Documents/bluetec/data/"

using namespace std;

DBClientConnection BTCloudApp::cDBConnection;

BTCloudApp::BTCloudApp()
{
	// Init mongo client
	mongo::client::initialize();

	// Init mysql client
	/*cMysqlConnection = MysqlConnector(pConfiguration->GetMySQLHost(),
										pConfiguration->GetMySQLUser(),
										pConfiguration->GetMySQLPassword());*/
}

BTCloudApp::~BTCloudApp()
{
	// Release mongodb connection
	mongo::client::shutdown(0);

	// Release mysql connection
	//cMysqlConnection.Disconnect();
}

void ProcessPackage(string path)
{
	// Connect to mongo client
	BTCloudApp::cDBConnection.connect(pConfiguration->GetMongoDBHost());
	Info(REVGEO_TAG "%s - Connected to mongodb", pConfiguration->GetTitle().c_str());

	// Process
	protocolo::processa_pacote((char*)path.c_str(), path.length(), &BTCloudApp::cDBConnection);

	// Disconnect from mongodb
	BSONObj info;
	BTCloudApp::cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(REVGEO_TAG "%s - Disconnected from mongodb", pConfiguration->GetTitle().c_str());
}

bool BTCloudApp::Initialize()
{
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	pFileSystem->AddFileSystemListener(this);
/*
	cEventFileSystem.setListenerParam(EventFileSystem::RECURSIVE_PATH);
	cEventFileSystem.setPath(pConfiguration->GetAppListeningPath());
	cFileManager.setPath(PATH);

	cEventFileSystem.setListnerEventFileSystem(ProcessPackage);
	cEventFileSystem.start();
*/
	return true;
}

bool BTCloudApp::Update(float dt)
{
	//Log("Update: %d", dt);
	(void) dt;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	(void) ev;
	Log(REVGEO_TAG "%s - File system change %s", pConfiguration->GetTitle().c_str());
}

bool BTCloudApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());
	return true;
}
