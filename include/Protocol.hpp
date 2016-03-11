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

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <list>
#include <string>
#include "mongo/client/dbclient.h" // for the driver
#include "entities/Bluetec400.hpp"
#include "managers/BlueTecFileManager.h"

using namespace std;

using namespace Sascar::Bluetec400;

namespace Sascar {

/** \class Protocol
 *  \brief Class used to parse BT4 files and generate JSON output.
 */
class Protocol
{
	public:
		Protocol();
		virtual ~Protocol();
		void Process(const char *path, int len, mongo::DBClientConnection *dbClient);
		void FillDataContract(string clientName, std::string plate, DataCache &retorno);
		void ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo);

	private:
		void CreatePosition();
		uint64_t GetLastPosition(uint32_t vehicleId, uint64_t lastPositionDate);
		void UpdateLastPosition(int vehicleId);
		void GetClientData(DataCache &retorno, std::string chave);
		uint32_t GetClient(std::string clientName);
		uint32_t CreateClient(std::string clientName);
		uint32_t CreateEquipment(uint32_t projectId, uint32_t equipIMei);
		uint32_t CreateVehicle(uint32_t clientId, uint32_t equipId, string plate);

		void ParseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo);
		void ParseA3A5A7(unsigned int ponteiroIni, unsigned int arquivo);
		void ParseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim);
		void ParseHSYNC(string hsync);

	private:
		mongo::DBClientConnection *pDBClientConnection;
		bluetec::BlueTecFileManager cFileManager;
		uint64_t iLastPositionDate;
		uint32_t iLastPositionVehicle;
		const int iLapsoSize;
		std::string sSerializedData;

		DataCache dataCache;
		Package cPackage;
		Data *pData;
		Position *pPosition;
		EventFlag *pEventFlag;
		Telemetry *pTelemetry;
		OdoVel *pOdoVel;
};

}

#endif // PROTOCOL_HPP
