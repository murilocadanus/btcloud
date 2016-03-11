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

class Protocol
{
	public:
		Protocol();
		virtual ~Protocol();
		void Process(const char *path, int len, mongo::DBClientConnection *dbClient);
		void FillDataContract(string clientName, std::string plate, cache_cadastro &retorno);
		void ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo);

	private:
		void CreatePosition();
		uint64_t GetLastPosition(uint32_t vehicleId, uint64_t lastPositionDate);
		void UpdateLastPosition(int vehicleId);
		void GetClientData(cache_cadastro &retorno, std::string chave);
		uint32_t GetClient(std::string clientName);
		uint32_t CreateClient(std::string clientName);
		uint32_t CreateEquipment(uint32_t projectId, uint32_t equipIMei);
		uint32_t CreateVehicle(uint32_t clientId, uint32_t equipId, string plate);

		void ParseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo);
		void ParseA3A5A7(unsigned int ponteiroIni, unsigned int arquivo);
		void ParseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim);
		void ParseHSYNC(string hsync);

	public:
		static char nome_projeto[50];
		static int projeto;

		static char campo_ordem[20];
		static char campo_ordem_nr[5];
		static int ncampo_ordem;
		static char campo_update1[20];
		static char campo_update2[20];
		static char campo_update3[20];
		static char campo_update4[20];

		static char campo_ordem_bdcentral[20];
		static char campo_filtro_bdcentral[20];
		static char campo_bdcentral_cond[50];

		static int sleep_conexao;
		static int sleep_comandos;
		static int sleep_ack;
		static int n_tentativas_reenvio;

		static char modelos_sasgc[200];

	private:
		mongo::DBClientConnection *pDBClientConnection;
		bluetec::BlueTecFileManager cFileManager;
		uint64_t iLastPositionDate;
		uint32_t iLastPositionVehicle;
		const int iLapsoSize;
		std::string sSerializedData;

		cache_cadastro cad;
		Bluetec400::Bluetec400 cBluetecPacote;
		PacoteEnriquecido *pPacote;
		EquipPosicao *pPosition;
		EquipFlags *pEventFlag;
		Telemetry *pTelemetry;
		OdoVel *pOdoVelGPS;
};

}

#endif // PROTOCOL_HPP
