#ifndef BLUETEC400APP_HPP
#define BLUETEC400APP_HPP

#include "Defines.hpp"
#include <cstdlib>
#include <iostream>

using namespace Sascar;

class Bluetec400App : public IApp
{
	public:
		Bluetec400App();
		virtual ~Bluetec400App();

		virtual bool Initialize();
		virtual bool Process() override;
		virtual bool Shutdown() override;
};

#endif // BLUETEC400APP_HPP
