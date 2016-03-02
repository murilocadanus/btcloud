#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include <cstdlib>
#include <iostream>
#include <api/mysql/MySQLConnector.hpp>
#include <interface/IEventFileSystemListener.hpp>
#include <mongo/client/dbclient.h> // for the driver
#include "managers/BlueTecFileManager.h"
#include "Defines.hpp"
#include "Configuration.hpp"
#include "Protocol.hpp"

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
		void OnFileSystemNotifyChange(const EventFileSystem *ev);

	private:
		DBClientConnection cDBConnection;
		BlueTecFileManager cFileManager;
		Protocol cProtocol;

};

#endif // BTCLOUD_APP_HPP
