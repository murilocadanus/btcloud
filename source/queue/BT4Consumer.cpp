#include "queue/BT4Consumer.hpp"
#include "util/Log.hpp"
#include "Configuration.hpp"
#include "entities/Message.hpp"
#include <unistd.h>
#include "FileSystem.hpp"
#include <mutex>

#define TAG "[BT4Consumer] "

std::mutex g_lock;

namespace Sascar {

BT4Consumer::BT4Consumer(const std::string& brokerURI)
	: latch(1)
	, connection(NULL)
	, session(NULL)
	, destination(NULL)
	, consumer(NULL)
	, brokerURI(brokerURI)
	, cDBConnection()
{
}

BT4Consumer::~BT4Consumer()
{
	cleanup();
}

void BT4Consumer::close()
{
	this->cleanup();
}

void BT4Consumer::waitUntilReady()
{
	latch.await();
}

void BT4Consumer::run()
{
	try
	{
		Info(TAG "Starting run");

		// Init mongo client
		mongo::client::initialize();

		// Connect to mongo client
		cDBConnection.connect(pConfiguration->GetMongoDBHost());
		Info(TAG "Connected to mongodb");

		// Create a ConnectionFactory
		auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory(brokerURI));

		// Create a Connection
		connection = connectionFactory->createConnection();
		Info(TAG "Created queue connection with URI: %s", brokerURI.c_str());

		connection->start();
		Info(TAG "Connection started");

		connection->setExceptionListener(this);

		// Create a Session
		session = connection->createSession(Session::AUTO_ACKNOWLEDGE);
		Info(TAG "Created session, ackMode: AUTO_ACKNOWLEDGE");

		// Create the destination (Topic or Queue)
		destination = session->createQueue(pConfiguration->GetActiveMQQueue());
		Info(TAG "Created queue: %s", pConfiguration->GetActiveMQQueue().c_str());

		// Create a MessageConsumer from the Session to the Topic or Queue
		consumer = session->createConsumer(destination);

		consumer->setMessageListener(this);
	}
	catch (CMSException& e)
	{
		// Indicate we are ready for messages.
		latch.countDown();
		Error(TAG "Exception occured: %s", e.getMessage().c_str());
	}
}

// Called from the consumer since this class is a registered MessageListener.
void BT4Consumer::onMessage(const cms::Message* message)
{
	Info(TAG "Intercepted message");

	try
	{
		uint8_t *buffer = NULL;
		const BytesMessage *byteMessage = dynamic_cast<const BytesMessage*>(message);
		Sascar::Message messageReceived;

		messageReceived.SetName(byteMessage->getStringProperty("arquivo"));
		messageReceived.SetType(byteMessage->getStringProperty("tipo"));
		messageReceived.SetPlate(byteMessage->getStringProperty("placa"));
		messageReceived.SetClient(byteMessage->getStringProperty("cliente"));
		messageReceived.SetUpdatedAt(byteMessage->getLongProperty("data"));
		messageReceived.SetSource(byteMessage->getStringProperty("origem"));

		buffer = byteMessage->getBodyBytes();

		if(buffer != NULL)
		{
			if (byteMessage->getBodyLength() > 0)
			{
				if(messageReceived.GetType() == "BT4")
				{
					messageReceived.SetMessageSize(byteMessage->getBodyLength());
					messageReceived.SetMessage(buffer);

					Dbg(TAG "Message byte message %d", byteMessage->getBodyLength());
					Dbg(TAG "Message body length %d", messageReceived.GetMessageSize());
					Info(TAG "Received message, name: %s type: %s client: %s plate: %s source: %s date: %i",
						messageReceived.GetName().c_str(), messageReceived.GetType().c_str(),
						messageReceived.GetClient().c_str(), messageReceived.GetPlate().c_str(),
						messageReceived.GetSource().c_str(), messageReceived.GetUpdatedAt());

					/*
					bool written = pFileSystem->SaveFile(messageReceived.GetClient(), messageReceived.GetPlate(),
						messageReceived.GetName(), messageReceived.GetMessage(), byteMessage->getBodyLength());

					if(written)
					{
						string filePath = pFileSystem->GetPath() + messageReceived.GetClient() + "/" + messageReceived.GetPlate() + "/" + messageReceived.GetName();
						cProtocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);
					}*/

					sleep(pConfiguration->GetSleepProcessInterval());
				}
				else
				{
					Dbg(TAG "Not a BT4 message, name: %s type: %s client: %s plate: %s source: %s date: %i",
						 messageReceived.GetName().c_str(), messageReceived.GetType().c_str(),
						messageReceived.GetClient().c_str(), messageReceived.GetPlate().c_str(),
						 messageReceived.GetSource().c_str(), messageReceived.GetUpdatedAt());
				}
			}
			else
			{
				Error(TAG "Message with 0 bytes");
				delete buffer;
			}
		}
		else
			Error(TAG "Empty message");


	}
	catch(CMSException& e)
	{
		Error(TAG "Exception occured: %s", e.getMessage().c_str());
	}
}

// If something bad happens you see it here as this class is also been
// registered as an ExceptionListener with the connection.
void BT4Consumer::onException(const CMSException& ex AMQCPP_UNUSED)
{
	Error(TAG "Exception occured: %s", ex.getMessage().c_str());
	exit(1);
}

void BT4Consumer::cleanup()
{
	if(connection != NULL)
	{
		try
		{
			connection->close();
		}
		catch(cms::CMSException& ex)
		{
			Error(TAG "Exception occured: %s", ex.getMessage().c_str());
		}
	}

	// Disconnect from mongodb
	BSONObj info;
	cDBConnection.logout(pConfiguration->GetMongoDBHost(), info);
	Info(TAG "Disconnected from mongodb");

	// Destroy resources.
	try
	{
		delete destination;
		destination = NULL;
		delete consumer;
		consumer = NULL;
		delete session;
		session = NULL;
		delete connection;
		connection = NULL;
	}
	catch (CMSException& e)
	{
		Error(TAG "Exception occured: %s", e.getMessage().c_str());
	}
}

}
