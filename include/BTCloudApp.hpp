#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>
#include "BlueTecFileManager.h"
#include "NotifyDirectory.h"

using namespace Sascar;
using namespace bluetec;
using namespace std;

class BTCloudApp : public IApp
{
	public:
		BTCloudApp();
		virtual ~BTCloudApp();

		virtual bool Initialize();
		bool Process() { while(1); return true; }
		virtual bool Shutdown() override;

	private:
		BlueTecFileManager cFileManager;
		NotifyDirectory cNotifyDirectory;
};

#endif // BTCLOUD_APP_HPP
