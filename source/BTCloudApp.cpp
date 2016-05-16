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
	, qQueueBT4FileNames()
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

bool BTCloudApp::Update(float dt)
{
	Dbg(TAG "%d", dt);

	if(!qQueueBT4FileNames.empty())
	{
		// Get file at top position
		string filePath = qQueueBT4FileNames.front();
		int fileLength = filePath.length();
		Dbg(TAG "%s %d", filePath.c_str(), fileLength);

		// Process
		cProtocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);
		qQueueBT4FileNames.pop();
		return true;
	}
	else return false;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	string filePath = ev->GetDirName() + ev->GetFileName();
	Dbg(TAG "OnFileSystemNotifyChange %s", filePath.c_str());

	// Push file to queue
	qQueueBT4FileNames.push(filePath);
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
