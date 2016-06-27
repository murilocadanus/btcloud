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
#include "entities/ProtocolEntity.hpp"
#include "managers/BlueTecFileManager.h"
#include "util/ProtocolUtil.hpp"

using namespace std;

using namespace BTCloud::Entities;

namespace BTCloud {

/** \class Protocol
 *  \brief Class used to parse BT4 files and generate JSON output.
 */
class Protocol
{
	public:
		/** \brief Protocol - Default constructor. */
		Protocol();

		/** \brief ~Protocol - Default destructor. */
		virtual ~Protocol();

		/** \brief Process -.
		 *
		 * \param path const char*
		 * \param len int
		 * \param dbClient DBClientConnection*
		 * \return void
		 */
		void Process(const char *path, int len, mongo::DBClientConnection *dbClient);

		/** \brief FillDataContract -.
		 *
		 * \param clientName string
		 * \param plate string
		 * \param retorno DataCache&
		 * \return void
		 */
		void FillDataContract(string clientName, std::string plate, DataCache &retorno);

		/** \brief ParseData -.
		 *
		 * \param dados string
		 * \param ponteiroIni int
		 * \param ponteiroFim int
		 * \param arquivo int
		 * \return void
		 */
		void ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo, bool isFinalRoute, bool isStartRoute);

		/** \brief ParseLapse -.
		 *
		 * \param lapso BTCloud::Util::Lapse
		 * \param dados string
		 * \param hfull Bluetec::HFull
		 * \param index int
		 * \param fim int

		 * \return void
		 */
		void ParseLapse(Util::Lapse &lapso, string dados, Bluetec::HFull hfull, int index, int fim, uint8_t dataType, bool isStartRoute);

	private:
		/** \brief CreatePosition -.
		 *
		 * \return void
		 */
		void CreatePosition(bool isOdometerIncreased, bool isHourmeterIncreased, bool routeEnd, bool hasVelocity, bool hasBlock, Bluetec::enumDataType type);

		/** \brief GetLastPosition -.
		 *
		 * \param vehicleId uint32_t
		 * \param lastPositionDate uint64_t
		 * \return void
		 */
		uint64_t GetLastPosition(uint32_t vehicleId, uint64_t lastPositionDate);

		/** \brief UpdateLastPosition -.
		 *
		 * \param vehicleId int
		 * \param datePosition u_int64_t
		 * \param dateArrival u_int64_t
		 * \return void
		 */
		void UpdateLastPosition(int vehicleId, u_int64_t datePosition, u_int64_t dateArrival);

		/** \brief GetClientData -.
		 *
		 * \param retorno DataCache&
		 * \param chave string
		 * \return void
		 */
		void GetClientData(DataCache &retorno, std::string chave);

		/** \brief GetClient -.
		 *
		 * \param clientName string
		 * \return uint32_t
		 */
		uint32_t GetClient(string clientName);

		/** \brief CreateClient -.
		 *
		 * \param clientName string
		 * \return uint32_t
		 */
		uint32_t CreateClient(string clientName);

		uint32_t CreateEquipmentImei();

		/** \brief CreateEquipment -.
		 *
		 * \param projectId uint32_t
		 * \param equipIMei uint32_t
		 * \param clientName string
		 * \return uint32_t
		 */
		uint32_t CreateEquipment(uint32_t projectId, uint32_t equipIMei);

		/** \brief GetVehicle -.
		 *
		 * \param clientName string
		 * \return uint32_t
		 */
		uint32_t GetVehicle(uint32_t equipId);

		/** \brief CreateVehicle -.
		 *
		 * \param clientId uint32_t
		 * \param equipId uint32_t
		 * \param plate string
		 * \return uint32_t
		 */
		uint32_t CreateVehicle(uint32_t clientId, uint32_t equipId, string plate);

		/** \brief ParseHFULL -.
		 *
		 * \param strHfull string
		 * \param ponteiroIni unsigned int
		 * \param ponteiroFim unsigned int
		 * \param arquivo unsigned int
		 * \return void
		 */
		void ParseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo);

		/** \brief ParseA3A5A7 -.
		 *
		 * \param ponteiroIni unsigned int
		 * \param arquivo unsigned int
		 * \return void
		 */
		void ParseA3A5A7(string a3a5a7);

		/** \brief ParseHSYNS -.
		 *
		 * \param hsyns string
		 * \param arquivo unsigned int
		 * \param ponteiroFim unsigned int
		 * \return void
		 */
		void ParseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim);

		/** \brief ParseHSYNC -.
		 *
		 * \param hsync string
		 * \param arquivo unsigned int
		 * \param ponteiroFim unsigned int
		 * \return void
		 */
		void ParseHSYNC(string hsync, unsigned int arquivo, unsigned int ponteiroFim);

		void DebugPass();

	private:
		mongo::DBClientConnection *pDBClientConnection;
		Bluetec::BlueTecFileManager cFileManager;
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
