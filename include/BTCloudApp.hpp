#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include <cstdlib>
#include <iostream>
#include <api/mysql/MySQLConnector.hpp>
#include <interface/IEventFileSystemListener.hpp>
#include <mongo/client/dbclient.h> // for the driver
#include "Defines.hpp"
#include "BlueTecFileManager.h"
#include "Configuration.hpp"


using namespace Sascar;
using namespace bluetec;
using namespace std;
using namespace mongo;

class BTCloudApp : public IApp, public IEventFileSystemListener
{
	public:
		BTCloudApp();
		virtual ~BTCloudApp();

		// IApp - IManager
		virtual bool Initialize() override;
		virtual bool Shutdown() override;

	private:
		BlueTecFileManager cFileManager;
		DBClientConnection cDBConnection;
		MysqlConnector cMysqlConnection;

		void OnFileSystemNotifyChange(const EventFileSystem *ev);
};

#endif // BTCLOUD_APP_HPP
