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

#include <string>
#include <stdint.h>
#include <vector>

#ifndef BLUETEC
#define BLUETEC

using namespace std;

namespace Sascar { namespace Bluetec400 {

	struct DataCache {
		uint64_t esn;
		uint64_t id;
		uint64_t antenaInt;
		std::string antenaText;
		uint64_t antenaType;
		uint32_t project;
		uint32_t clioId;
		uint32_t ger1;
		uint32_t ger2;
		uint32_t ger3;
		uint32_t numberCon;
		uint32_t veioId;
		uint32_t contractType;
		uint32_t classe;
		uint32_t serial0;
		uint32_t serial1;
		uint32_t isSasgc;
		uint32_t vehicleType;
		uint32_t debug;
		std::string plate;
		std::string inputs;
		std::string outputs;
		std::vector<int32_t> sensors;
		std::vector<int32_t> actuators;
		uint32_t maxRpm;
		std::string clincId;
		std::string clientName;
	};

	struct Contract
	{
		uint64_t esn;
		uint32_t clioId;
		uint32_t manager;
		uint32_t numberCon;
		uint32_t veioId;
		uint32_t contractType;
		uint32_t classe;
		uint32_t protocol;
		uint32_t serial;
		uint32_t panicDoor;
		uint32_t blockDoor;
		uint32_t id;
		uint32_t isSasgc;
		uint32_t vehicleType;
	};

	struct Telemetry
	{
		uint32_t stretch;
		uint32_t dateTime;
		double velocity;
		uint32_t rpm;
		double odometer;
		double hourmeter;
		bool ed1;
		bool ed2;
		bool ed3;
		bool ed4;
		bool ed5;
		bool ed6;
		bool ed7;
		bool ed8;
		double acelX;
		double acelY;
		uint32_t an1;
		uint32_t an2;
		uint32_t an3;
		uint32_t an4;
		string operation;
	};

	struct OdoVel
	{
		uint32_t odometer;
		uint32_t velocity;
	};

	struct EventFlag
	{
		uint32_t block;
		uint32_t alertInt;
		uint32_t somente_posicao;
		uint32_t inputPoint;
		uint32_t outputPoint;
		uint32_t alertBat;
		uint32_t pad1;
		uint32_t peripheral;
		uint32_t peripheralCod;
		uint32_t pad2;
		uint32_t openTrip;
		uint32_t closeTrip;
		uint32_t trailAround;
		uint32_t refPoint;
		uint32_t ignition;
		uint32_t hasMessage;
		uint32_t antiTheft;
	};

	struct Position
	{
		uint32_t dateTime;
		uint32_t dateArrive;
		uint32_t input;
		uint32_t output;
		double lat;
		double lon;
		OdoVel odoVel;
		uint32_t pointId;
		//t32_flags flags;
		//t32_csq_rot csq_id_rota;
		//t32_sai_cfg saida_cfg;
		//t32_ent_gpsi entrada_gps_info;
		uint32_t positionIndex;
		uint32_t gpsAltitude;
		//t32_inf_mot inf_motorista;
		//t32_horimetro info_horimetro;
		uint32_t lastValTmp;
		uint32_t ad1;
		uint32_t ad2;
		uint32_t ad3;
		uint32_t ad4;
		uint32_t securityStateOutput;
		uint32_t securityStateInput;
		uint32_t vccCharge;
		uint32_t areaId;
		Telemetry telemetryInfo;
		uint32_t type;
		uint32_t source;
		uint32_t refPoint;
		uint32_t refDist;
		uint32_t refAng;
		uint32_t route;
		uint32_t operation;
		EventFlag eventFlag;
	};

	struct Data
	{
		Contract contract;
		Position position;
	};

	struct Package
	{
		Data data;
		Telemetry telemetry;

		void Clear()
		{

		}
	};

}} // namespace

#endif // BLUETEC

