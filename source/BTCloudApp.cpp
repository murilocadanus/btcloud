#include "BTCloudApp.hpp"
#include "Configuration.hpp"

#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include <thread>
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
	cDBConnection.connect(pConfiguration->GetMongoDBHost());
	Info(TAG "Connected to mongodb");

	// TODO Refactory to remove this
	cFileManager.SetPath(pConfiguration->GetAppListeningPath());

	activemq::library::ActiveMQCPP::initializeLibrary();
	{
		BT4Consumer consumer(pConfiguration->GetActiveMQTarget());
		// Start the consumer thread.
		Thread consumerThread(&consumer);
		consumerThread.start();

		// Wait for the consumer to indicate that its ready to go.
		consumer.waitUntilReady();

		// Wait for the threads to complete.
		consumerThread.join();
	}

	activemq::library::ActiveMQCPP::shutdownLibrary();

	return true;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	string filePath = ev->GetDirName() + ev->GetFileName();
	Dbg(TAG "OnFileSystemNotifyChange %s", filePath.c_str());

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

	// Finish thread consumer
	//consumerThread.close();

	return true;
}

}
