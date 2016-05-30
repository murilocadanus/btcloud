#include "BTCloudApp.hpp"
#include "Configuration.hpp"

#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include <thread>
#include "FileSystem.hpp"
#include "entities/Message.hpp"

#include <mutex>

#define TAG "[BlueTec400] "

std::mutex mutexQueue;

using namespace std;

namespace BTCloud {

BTCloudApp::BTCloudApp()
	: cFileManager()
	, cDBConnection()
	, connection(NULL)
	, session(NULL)
	, destination(NULL)
	, consumer(NULL)
	, latch(1)
{
}

BTCloudApp::~BTCloudApp()
{
	// Release mongodb connection
	mongo::client::shutdown();
}

bool BTCloudApp::Initialize()
{
	Info(TAG "Initializing...");

	// Set app listening path
	cFileManager.SetPath(pConfiguration->GetAppListeningPath());

	activemq::library::ActiveMQCPP::initializeLibrary();

	try
	{
		Info(TAG "Starting run");

		// Init mongo client
		mongo::client::initialize();

		// Connect to mongo client
		cDBConnection.connect(pConfiguration->GetMongoDBHost());
		Info(TAG "Connected to mongodb");

		// Case a database, user and password are defined, use it to authenticate
		if(pConfiguration->GetMongoDBDatabase() != "" && pConfiguration->GetMongoDBUser() != "" && pConfiguration->GetMongoDBPassword() != "")
		{
			BSONObj authCredentials = BSON("user" << pConfiguration->GetMongoDBUser() <<
											"db" << pConfiguration->GetMongoDBDatabase() <<
											"pwd" << pConfiguration->GetMongoDBPassword() <<
											"digestPassword" << true <<
											"mechanism" << pConfiguration->GetMongoDBAuthMechanism());

			cDBConnection.auth(authCredentials);
		}


		// Create a ConnectionFactory
		auto_ptr<ConnectionFactory> connectionFactory(ConnectionFactory::createCMSConnectionFactory(pConfiguration->GetActiveMQTarget()));

		// Create a Connection
		connection = connectionFactory->createConnection();
		Info(TAG "Created queue connection with URI: %s", pConfiguration->GetActiveMQTarget().c_str());

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
	catch (DBException& e)
	{
		Error(TAG "Exception occured: %d: %s", e.getCode(), e.getInfo().msg.c_str());
		return false;
	}
	catch (CMSException& e)
	{
		// Indicate we are ready for messages.
		latch.countDown();
		Error(TAG "Exception occured: %s", e.getMessage().c_str());
		return false;
	}

	return true;
}

void BTCloudApp::onMessage(const cms::Message* message)
{
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
					mutexQueue.lock();

					messageReceived.SetMessageSize(byteMessage->getBodyLength());
					messageReceived.SetMessage(buffer);

					Dbg(TAG "Message byte message %d", byteMessage->getBodyLength());
					Dbg(TAG "Message body length %d", messageReceived.GetMessageSize());
					Info(TAG "Received message, name: %s type: %s client: %s plate: %s source: %s date: %i",
						messageReceived.GetName().c_str(), messageReceived.GetType().c_str(),
						messageReceived.GetClient().c_str(), messageReceived.GetPlate().c_str(),
						messageReceived.GetSource().c_str(), messageReceived.GetUpdatedAt());

					bool written = pFileSystem->SaveFile(messageReceived.GetClient(), messageReceived.GetPlate(),
						messageReceived.GetName(), messageReceived.GetMessage(), byteMessage->getBodyLength());

					if(written)
					{
						string filePath = pFileSystem->GetPath() + messageReceived.GetClient() + "/" + messageReceived.GetPlate() + "/" + messageReceived.GetName();
						qQueueFiles.push(filePath);
					}

					sleep(pConfiguration->GetSleepProcessInterval());

					mutexQueue.unlock();
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

void BTCloudApp::onException(const CMSException& ex AMQCPP_UNUSED)
{
	Error(TAG "Exception occured: %s", ex.getMessage().c_str());
}

bool BTCloudApp::Update(float dt)
{
	(void)dt;

	if(!qQueueFiles.empty())
	{
		mutexQueue.lock();

		string filePath = qQueueFiles.front();
		qQueueFiles.pop();

		Protocol protocol;
		protocol.Process(filePath.c_str(), filePath.length(), &cDBConnection);

		mutexQueue.unlock();
	}

	return true;
}

bool BTCloudApp::Shutdown()
{
	Info(TAG "Shutting down...");
	activemq::library::ActiveMQCPP::shutdownLibrary();

	return true;
}

}
