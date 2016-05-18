/*
* Copyright (c) 2016, Sascar
* All rights reserved.
*
* THIS SOFTWARE IS PROVIDED BY SASCAR ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL SASCAR BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BTCLOUD_APP_HPP
#define BTCLOUD_APP_HPP

#include <cstdlib>
#include <iostream>
#include <api/mysql/MySQLConnector.hpp>
#include <interface/IEventFileSystemListener.hpp>
#include "queue"
#include <mongo/client/dbclient.h>
#include "managers/BlueTecFileManager.h"
#include "Defines.hpp"
#include "Configuration.hpp"
#include "Protocol.hpp"

using namespace Sascar;
using namespace Bluetec;
using namespace std;
using namespace mongo;

namespace BTCloud {

/** \class BTCloudApp
 *  \brief Entry point for btcloud app.
 *  \details This class is responsable for start, intercept and finish
 * btcloud app.
 */
class BTCloudApp : public IApp//, public IEventFileSystemListener
{
	public:
		/** \brief BTCloudApp - Default constructor. */
		BTCloudApp();

		/** \brief ~BTCloudApp - Default destructor. */
		virtual ~BTCloudApp();

		/** \brief Initialize - Code to be executed at the beggining of app.
		 *
		 * \return void
		 *
		 */
		virtual bool Initialize() override;

		/** \brief Update - Code to be executed in a loop.
		 *
		 * \param dt float
		 * \return bool
		 *
		 */
		virtual bool Update(float dt) override;

		/** \brief Shutdown - Code to finish all objects used.
		 *
		 * \return void
		 *
		 */
		virtual bool Shutdown() override;

		/** \brief OnFileSystemNotifyChange - Implementation of callback method to intercept
		 * file system modification at specified folder.
		 *
		 * \param ev EventFileSystem*
		 * \return void
		 *
		 */
		//void OnFileSystemNotifyChange(const EventFileSystem *ev);

	private:
		std::queue<string> qQueueBT4FileNames;
		DBClientConnection cDBConnection;
		BlueTecFileManager cFileManager;
		Protocol cProtocol;
};
}

#endif // BTCLOUD_APP_HPP
