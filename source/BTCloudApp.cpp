#include "BTCloudApp.hpp"
#include "Configuration.hpp"

#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include "FileSystem.hpp"

#define BTCLOUD_TAG "[BlueTec400] "

using namespace std;

BTCloudApp::BTCloudApp()
	: cDBConnection()
{
}

BTCloudApp::~BTCloudApp()
{
	// Release mongodb connection
	mongo::client::shutdown(0);

	// Release mysql connection
	//if (cMysqlConnection)
	//	delete(cMysqlConnection);

	//cMysqlConnection = nullptr;
}

bool BTCloudApp::Initialize()
{
	Info(BTCLOUD_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	// Add the listener to notify this object
	pFileSystem->AddFileSystemListener(this);

	// Init mongo client
	mongo::client::initialize();

	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mongo client
	BTCloudApp::cDBConnection.connect(pConfiguration->GetMongoDBHost());
	Info(BTCLOUD_TAG "%s - Connected to mongodb", pConfiguration->GetTitle().c_str());

	// TODO Refactory to remove this
	cFileManager.setPath(pConfiguration->GetAppListeningPath());

	return true;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	string filePath = ev->GetDirName() + ev->GetFileName();
	Dbg(BTCLOUD_TAG "OnFileSystemNotifyChange %s", filePath.c_str());

	// Process
	cProtocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);
}

bool BTCloudApp::Shutdown()
{
	Info(BTCLOUD_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());

	// Disconnect from mongodb
	BSONObj info;
	BTCloudApp::cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(BTCLOUD_TAG "%s - Disconnected from mongodb", pConfiguration->GetTitle().c_str());

	return true;
}
