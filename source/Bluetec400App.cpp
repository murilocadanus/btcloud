#include "Bluetec400App.hpp"
#include "Configuration.hpp"

#define REVGEO_TAG "[BlueTec400] "

Bluetec400App::Bluetec400App()
{
}

Bluetec400App::~Bluetec400App()
{
}

bool Bluetec400App::Initialize()
{
	Info(REVGEO_TAG "%s - Initializing...", pConfiguration->GetTitle().c_str());
	return true;
}

bool Bluetec400App::Process()
{
	Info(REVGEO_TAG "%s - Start processing...", pConfiguration->GetTitle().c_str());
	return EXIT_SUCCESS;
}

bool Bluetec400App::Shutdown()
{
	Info(REVGEO_TAG "%s - Shutting down...", pConfiguration->GetTitle().c_str());
	return true;
}
