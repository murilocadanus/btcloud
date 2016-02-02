#ifndef BLUETEC400APP_HPP
#define BLUETEC400APP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>
#include "BlueTecFileManager.h"
#include "NotifyDirectory.h"

using namespace Sascar;
using namespace bluetec;
using namespace std;

class Bluetec400App : public IApp
{
	public:
		Bluetec400App();
		virtual ~Bluetec400App();

		virtual bool Initialize();
		bool Process() { while(1); return true; }
		virtual bool Shutdown() override;

	private:
		BlueTecFileManager cFileManager;
		NotifyDirectory cNotifyDirectory;
};

#endif // BLUETEC400APP_HPP
