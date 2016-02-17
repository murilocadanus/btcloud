#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>
#include <EventFileSystem.hpp>
#include "BlueTecFileManager.h"
#include "mongo/client/dbclient.h" // for the driver
#include <api/mysql/MySQLConnector.hpp>
#include "Configuration.hpp"

using namespace Sascar;
using namespace bluetec;
using namespace std;
using namespace mongo;

class BTCloudApp : public IApp
{
	public:
		BTCloudApp();
		virtual ~BTCloudApp();

		virtual bool Initialize();
		virtual bool Process();
		virtual bool Shutdown() override;
		static DBClientConnection cDBConnection;

	private:
		BlueTecFileManager cFileManager;
		EventFileSystem cEventFileSystem;
		MysqlConnector cMysqlConnection;
};

#endif // BTCLOUD_APP_HPP
