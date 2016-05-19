#ifndef BT4CONSUMER_HPP
#define BT4CONSUMER_HPP

#include <activemq/library/ActiveMQCPP.h>
#include <decaf/lang/Thread.h>
#include <decaf/lang/Runnable.h>
#include <decaf/util/concurrent/CountDownLatch.h>
#include <decaf/lang/Integer.h>
#include <decaf/lang/Long.h>
#include <decaf/lang/System.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <activemq/util/Config.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>
#include <cms/BytesMessage.h>
#include <cms/MapMessage.h>
#include <cms/ExceptionListener.h>
#include <cms/MessageListener.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include "managers/BlueTecFileManager.h"
#include "queue"

using namespace activemq::core;
using namespace decaf::util::concurrent;
using namespace decaf::util;
using namespace decaf::lang;
using namespace cms;
using namespace std;

namespace Sascar {

class BT4Consumer : public ExceptionListener, public MessageListener, public Runnable
{
	public:
		BT4Consumer(const std::string& brokerURI);
		virtual ~BT4Consumer();
		void close();
		void waitUntilReady();
		virtual void run();

		// Called from the consumer since this class is a registered MessageListener.
		virtual void onMessage(const cms::Message* message);

		// If something bad happens you see it here as this class is also been
		// registered as an ExceptionListener with the connection.
		virtual void onException(const CMSException& ex AMQCPP_UNUSED);

	private:
		void cleanup();

	private:
		BT4Consumer(const BT4Consumer&);
		BT4Consumer& operator=(const BT4Consumer&);

	private:
		CountDownLatch latch;
		Connection* connection;
		Session* session;
		Destination* destination;
		MessageConsumer* consumer;
		std::string brokerURI;
};

} // namespace

#endif // BT4CONSUMER_HPP

