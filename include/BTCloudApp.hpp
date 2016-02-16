#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>
#include <EventFileSystem.hpp>
#include "BlueTecFileManager.h"
#include "mongo/client/dbclient.h" // for the driver

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
		bool Process() { while(1); return true; }
		virtual bool Shutdown() override;
		static DBClientConnection cDBConnection;

	private:
		BlueTecFileManager cFileManager;
		EventFileSystem cEventFileSystem;
};

#endif // BTCLOUD_APP_HPP
