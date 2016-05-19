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
	, cBT4Consumer(pConfiguration->GetActiveMQTarget())
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
	//pFileSystem->AddFileSystemListener(this);

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
		// Start the producer thread.
		//Thread producerThread(&producer);
		//producerThread.start();

		// Start the consumer thread.
		Thread consumerThread(&cBT4Consumer);
		consumerThread.start();

		// Wait for the consumer to indicate that its ready to go.
		cBT4Consumer.waitUntilReady();

		// Wait for the threads to complete.
		//producerThread.join();
		consumerThread.join();

		//consumer.close();
		//producer.close();
	}

	activemq::library::ActiveMQCPP::shutdownLibrary();

	return true;
}

bool BTCloudApp::Update(float dt)
{
	if(!cBT4Consumer.GetQueue().empty())
	{
		// Get file at top position
		string filePath = cBT4Consumer.GetQueue().front();
		int fileLength = filePath.length();
		Dbg(TAG "%s %d -> %d", filePath.c_str(), fileLength, dt);

		// Process
		cProtocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);
		cBT4Consumer.GetQueue().pop();
		return true;
	}
	else return false;
}

/*void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	string filePath = ev->GetDirName() + ev->GetFileName();
	Dbg(TAG "OnFileSystemNotifyChange %s", filePath.c_str());

	// Push file to queue case it is a new BT4
	if(ev->GetFileName().substr(0, 3) == "BT4")
		qQueueBT4FileNames.push(filePath);
}*/

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
