#include "queue/BT4Producer.hpp"
#include "Configuration.hpp"
#include "util/Log.hpp"
#include "entities/Message.hpp"

#define TAG "[BT4Producer] "

namespace Sascar {

BT4Producer::BT4Producer()
{
}

BT4Producer::~BT4Producer()
{
	cleanup();
}

void BT4Producer::close()
{
	this->cleanup();
}

void BT4Producer::run()
{
	Info(TAG "Starting run");
}

void BT4Producer::cleanup()
{
}

void BT4Producer::createMessage()
{

}
}
