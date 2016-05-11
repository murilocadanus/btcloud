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


#define TAG "[BlueTec400] "

using namespace std;

namespace BTCloud {

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
	Info(TAG "Initializing...");

	// Add the listener to notify this object
	pFileSystem->AddFileSystemListener(this);

	// Init mongo client
	mongo::client::initialize();

	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mongo client
	BTCloudApp::cDBConnection.connect(pConfiguration->GetMongoDBHost());
	Info(TAG "Connected to mongodb");

	// TODO Refactory to remove this
	cFileManager.SetPath(pConfiguration->GetAppListeningPath());

	return true;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	string filePath = ev->GetDirName() + ev->GetFileName();
	Dbg(TAG "OnFileSystemNotifyChange %s", filePath.c_str());
	//Error(TAG "File to process: %s", ev->GetFileName().c_str());

	// Process
	cProtocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);
}

bool BTCloudApp::Shutdown()
{
	Info(TAG "Shutting down...");

	// Disconnect from mongodb
	BSONObj info;
	BTCloudApp::cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(TAG "Disconnected from mongodb");

	return true;
}

}
