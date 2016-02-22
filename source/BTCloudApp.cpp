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

using namespace std;

BTCloudApp::BTCloudApp()
	: cDBConnection()
	, cMysqlConnection()
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
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());

	// Add the listener to notify this object
	pFileSystem->AddFileSystemListener(this);

	// Init mongo client
	mongo::client::initialize();

	// Init mysql client
	cMysqlConnection.Initialize();

	// Connect to mongo client
	BTCloudApp::cDBConnection.connect(pConfiguration->GetMongoDBHost());
	Info(REVGEO_TAG "%s - Connected to mongodb", pConfiguration->GetTitle().c_str());

	// Connect to mysql
	cMysqlConnection.Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());
	Info(REVGEO_TAG "%s - Connected to mysql", pConfiguration->GetTitle().c_str());

	// TODO Refactory to remove this
	cFileManager.setPath(pConfiguration->GetAppListeningPath());

	return true;
}

void BTCloudApp::OnFileSystemNotifyChange(const EventFileSystem *ev)
{
	Log(REVGEO_TAG "OnFileSystemNotifyChange %s%s", ev->GetDirName().c_str(), ev->GetFileName().c_str());

	// Process
	//protocolo::processa_pacote(ev->GetDirName().c_str(), ev->GetDirName().length(), &cDBConnection);
}

bool BTCloudApp::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());

	// Disconnect from mongodb
	BSONObj info;
	BTCloudApp::cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(REVGEO_TAG "%s - Disconnected from mongodb", pConfiguration->GetTitle().c_str());

	// Disconnect mysql connection
	cMysqlConnection.Disconnect();
	Info(REVGEO_TAG "%s - Disconnected from mysql", pConfiguration->GetTitle().c_str());

	return true;
}
