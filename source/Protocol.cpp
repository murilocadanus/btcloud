#include "Protocol.hpp"
#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include <util/Log.hpp>
#include "mongo/client/dbclient.h" // for the driver
#include "Configuration.hpp"
#include <api/mysql/MySQLConnector.hpp>
#include <System.hpp>
#include <util/String.hpp>

#define TAG "[Protocol] "

using namespace Sascar;

namespace BTCloud
{

Protocol::Protocol()
	: iLastPositionDate(false)
	, iLapsoSize(sizeof(long int) + sizeof(double) * 6 + sizeof(int) * 13 + 16)
{
	pData = &cPackage.data;
	pTelemetry = &cPackage.telemetry;

	pPosition = &pData->position;
	pEventFlag = &pPosition->eventFlag;
	pOdoVel = &pPosition->odoVel;
}

Protocol::~Protocol()
{
}

void Protocol::ParseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo)
{
	Info(TAG "Init parse HFULL");

	Bluetec::HFull hfull;
	Bluetec::HeaderDataFile header;
	char tBuffer[500000];
	uint32_t tSize;

	char buffer[255];
	buffer[0] = (unsigned char) strHfull.at(0);
	BTCloud::Util::Output *p;
	p = (BTCloud::Util::Output*) buffer;

	hfull.lapso = 0;
	hfull.lapso += p->saida0 * 1;
	hfull.lapso += p->saida1 * 2;
	hfull.lapso += p->saida2 * 4;
	hfull.lapso += p->saida3 * 8;
	hfull.lapso += p->saida4 * 16;
	//hfull.idVeiculo = strHfull.substr(1, 8);
	//hfull.versaoFirmware = strHfull.substr(9, 2);
	//hfull.versaoHardware = (unsigned int)strHfull.at(12);
	//hfull.configHardware = (unsigned int)strHfull.at();
	//hfull.reservado[5];
	//hfull.constanteVelocidade2;
	//hfull.constanteVelocidade1;
	//hfull.constanteRPM;
	//hfull.confDiversas;
	//hfull.reservado1[8];
	hfull.acelx = (unsigned int ) strHfull.at( 33 );
	//hfull.reservado2[3];
	hfull.acely = (unsigned int ) strHfull.at( 37 );
	//hfull.tipoAcelerometro;
	//hfull.limiteAcelFrenTom[3];
	hfull.spanAcel = (unsigned int ) strHfull.at( 42 );
	//hfull.reservado3[3];
	//hfull.verConsistenciaSetup;
	//hfull.confHardwareRe; //Hardware config (Redundancy)
	//hfull.limiteAnalogico1;
	//hfull.limiteAnalogico234[3];
	//hfull.lapsosGravacaoEvento; //Event lapses
	///hfull.reservado4[8];
	//hfull.volume;
	//hfull.parametrosGPS;
	//hfull.ajusteHoraPorGps;
	//hfull.parametrosGerais;
	//hfull.reservado5[7];
	//hfull.alarmes[58];
	//hfull.reservado6[6];
	//hfull.gerenciaMemoria1;
	//hfull.gerenciaMemoria2;
	//hfull.reversao;
	//hfull.inicioHorarioVerao[3];
	//hfull.reservado7[9];
	//hfull.listaBloqueioOuPermissao[65];
	//hfull.reservado8[18];
	//hfull.numeroSerial[3];
	//hfull.reservado9;
	//hfull.tipoTeclado;
	//hfull.finalHorarioVerao[3];
	//hfull.reservado10;
	//hfull.hfull[5];

	cFileManager.SaveHfull(dataCache.veioId, hfull);

	Dbg(TAG "Fixing file cursor before and after HFULL");

	// Verify case something matches at the end of this HFULL
	if(cFileManager.getBufferFile(dataCache.veioId, ponteiroFim + 1, arquivo, tBuffer, tSize, header) &&
			header.beginPointer == ponteiroFim + 1 &&
			(header.dataType == Bluetec::enumDataType::DADOS ||
			header.dataType == Bluetec::enumDataType::DADOS_FINAL ||
			header.dataType == Bluetec::enumDataType::FINAL))
	{
		Dbg(TAG "HFULL + TEMP");

		// Remove this to overwrite it
		cFileManager.DelFile(dataCache.veioId, header.headerFile);

		// Update the start cursor
		header.beginPointer -= 512;

		// Save changes
		Dbg(TAG "Saving buffer file header data type %d", header.dataType);
		cFileManager.SaveBufferFile(dataCache.veioId, tBuffer, tSize, header);
	}

	// Verify case something matches at the start of this HFULL
	if(cFileManager.getBufferFile(dataCache.veioId, ponteiroIni - 1, arquivo, tBuffer, tSize, header) &&
			header.endPointer == ponteiroIni - 1 &&
			(header.dataType == Bluetec::enumDataType::DADOS ||
			header.dataType == Bluetec::enumDataType::HSYNS ||
			header.dataType == Bluetec::enumDataType::HSYNS_DADOS))
	{
		Dbg(TAG "TEMP + HFULL");

		// Remove this to overwrite it
		cFileManager.DelFile(dataCache.veioId, header.headerFile);

		// Update the end cursor
		header.endPointer += 512;

		// Save changes
		Dbg(TAG "Saving buffer file header data type %d", header.dataType);
		cFileManager.SaveBufferFile(dataCache.veioId, tBuffer, tSize, header);
	}
}

void Protocol::ParseHSYNC(string hsync, unsigned int arquivo, unsigned int ponteiroFim)
{
	Dbg(TAG "Init HSYNC %d", ponteiroFim);
	Dbg(TAG "hsync size: %d hex: %x", hsync.size(), hsync.c_str());

	struct BTCloud::Util::Lapse lapso;
	Bluetec::HeaderDataFile header;
	string pLapso;

	lapso.idTrecho = cFileManager.GetNextIdRoute();
	lapso.timestamp = 0;
	lapso.velocidade = 0;
	lapso.rpm = 0;
	lapso.odometro = 0;
	lapso.horimetro = 0;
	lapso.ed1 = 0;
	lapso.ed2 = 0;
	lapso.ed3 = 0;
	lapso.ed4 = 0;
	lapso.ed5 = 0;
	lapso.ed6 = 0;
	lapso.acelx = 0;
	lapso.acely = 0;
	lapso.an1 = 0;
	lapso.an2 = 0;
	lapso.an3 = 0;
	lapso.an4 = 0;

	lapso.timestamp = mktime(BTCloud::Util::ParseTimeDate(hsync.substr(8, 7)));

	// Load driver ibutton in bcd
	lapso.ibtMotorista = "";

	// Create a ostring to convert byte to hex with precision
	std::ostringstream driverStream;
	driverStream << std::hex << std::setfill('0');
	driverStream << std::setw(2) << (int)hsync.at(1); // 01 or 81

	Dbg(TAG "Driver: %x", hsync.substr(0, 8).c_str());

	// Standardizes the operation code to not be empty (81)
	if(driverStream.str() == "01")
	{
		// Convert all driver info id to Hex
		std::vector<unsigned char> driverElements {
			(unsigned char)hsync.at(2), (unsigned char)hsync.at(3), (unsigned char)hsync.at(4),
			(unsigned char)hsync.at(5), (unsigned char)hsync.at(6), (unsigned char)hsync.at(7)
		};
		for(int elem : driverElements) driverStream << std::setw(2) << elem;
		lapso.ibtMotorista += driverStream.str().c_str();
	}

	Dbg(TAG "Parse driver id: %s", lapso.ibtMotorista.c_str());
	Dbg(TAG "Parse timestamp Hex 0x%x", hsync.substr(8, 7).c_str());
	Dbg(TAG "Parse timestamp %d", lapso.timestamp);
	lapso.odometro = BTCloud::Util::ParseHodometer(hsync.substr(15, 3));

	Dbg(TAG "Parse odometer %d", lapso.odometro);
	Dbg(TAG "%d %d %d %d - %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)hsync.at(0)), hex, setw(2), setfill('0'), int((unsigned char)hsync.at(hsync.length() - 1)));
	lapso.horimetro = BTCloud::Util::ParseHourmeter(hsync.substr(18, 3));

	Dbg(TAG "Parse horimeter %d", lapso.horimetro);

	pLapso = BTCloud::Util::PersistableLapse(&lapso);

	header.file = arquivo;
	header.endPointer = ponteiroFim;
	header.timestamp = lapso.timestamp;

	header.dataType = Bluetec::enumDataType::HSYNC;

	Dbg(TAG "IMC 1 - header.dataType = %d", header.dataType);
	Dbg(TAG "Saving HSYNC %d .. ", header.endPointer);

	// Save route start
	Dbg(TAG "Save buffer file Header data type %d", header.dataType);
	Dbg(TAG "Save buffer file Enum data type %d", Bluetec::enumDataType::HSYNC);

	// Create a position for this HSYNC if it has a position
	bool isGPS = (unsigned char) hsync.at(1) >= 0xA0;

	double lat = isGPS ? BTCloud::Util::ParseLatitude(hsync.substr(2, 3), 1) : 0.0;
	double lon = isGPS ? BTCloud::Util::ParseLongitude(hsync.substr(5, 3), 1, 0) : 0.0;

	Dbg(TAG "Lat Long -> %20.18f %20.18f", lat, lon);

	// Setting data to position package
	pPosition->lat = lat;
	pPosition->lon = lon;

	pPosition->dateTime = lapso.timestamp;
	pPosition->dateArrive = pTimer->GetCurrentTime();
	pPosition->inf_motorista.id = lapso.ibtMotorista;

	if(lapso.rpm > 0 && lapso.velocidade > 0)
		pEventFlag->ignition = 1;
	else
		pEventFlag->ignition = 0;

	pOdoVel->velocity = lapso.velocidade;

	Dbg(TAG "Timestamp before transmition %d", lapso.timestamp);

	// Send position package to queue in final protobuf format
	//bluetecPacote.SerializeToString(&serializado);
	BTCloud::Util::LapsoToTelemetry(pTelemetry, lapso);

	// Save JSON at MongoDB
	CreatePosition(false, false);

	// Reset entity
	cPackage.Clear();
}

void Protocol::ParseA3A5A7(unsigned int ponteiroIni, unsigned int arquivo)
{
	// Search for route persisted with start cursor = 1 and remove it
	Bluetec::HeaderDataFile header;
	char tBuffer[500000];
	uint32_t tSize=0;

	Dbg(TAG "Searching the start point in this route");

	// Case something matches with the start of this route
	if(cFileManager.getBufferFile(dataCache.veioId, ponteiroIni - 1, arquivo, tBuffer, tSize, header) &&
			header.endPointer == ponteiroIni - 1 && header.file == arquivo)
	{
		Dbg(TAG "This route has a start point");

		switch(header.dataType)
		{
			case Bluetec::enumDataType::HSYNS_DADOS:
				Dbg(TAG "Route completed, removing from temp memory");
				cFileManager.DelFile(dataCache.veioId, header.headerFile);
			break;

			case Bluetec::enumDataType::DADOS:
				Dbg(TAG "Route incomplete, increasing end route point");

				// Remove this temp file to overwrite it
				cFileManager.DelFile(dataCache.veioId, header.headerFile);

				// Update the data type
				header.dataType = Bluetec::enumDataType::DADOS_FINAL;

				// Save it
				cFileManager.SaveBufferFile(dataCache.veioId, tBuffer, tSize, header);
			break;
		}
	}
	else
	{
		// Case this does not need to be saved temporarily
		header.file = arquivo;
		header.beginPointer = ponteiroIni;
		header.dataType = Bluetec::enumDataType::FINAL;

		Dbg(TAG "Save buffer file header data type");
		cFileManager.SaveBufferFile(dataCache.veioId, tBuffer, (uint32_t)0, header);
	}
}

void Protocol::ParseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim)
{
	Dbg(TAG "Init HSYNS %d", ponteiroFim);
	Dbg(TAG "hsyns size: %d hex: %x", hsyns.size(), hsyns.c_str());

	struct BTCloud::Util::Lapse lapso;
	Bluetec::HeaderDataFile header;
	string pLapso;

	lapso.idTrecho = cFileManager.GetNextIdRoute();
	lapso.timestamp = 0;
	lapso.velocidade = 0;
	lapso.rpm = 0;
	lapso.odometro = 0;
	lapso.horimetro = 0;
	lapso.ed1 = 0;
	lapso.ed2 = 0;
	lapso.ed3 = 0;
	lapso.ed4 = 0;
	lapso.ed5 = 0;
	lapso.ed6 = 0;
	lapso.acelx = 0;
	lapso.acely = 0;
	lapso.an1 = 0;
	lapso.an2 = 0;
	lapso.an3 = 0;
	lapso.an4 = 0;

	lapso.timestamp = mktime(BTCloud::Util::ParseTimeDate(hsyns.substr(8, 7)));

	// Load driver ibutton in bcd
	lapso.ibtMotorista = "";

	// Create a ostring to convert byte to hex with precision
	std::ostringstream driverStream;
	driverStream << std::hex << std::setfill('0');
	driverStream << std::setw(2) << (int)hsyns.at(1); // 01 or 81

	Dbg(TAG "Driver: %x", hsyns.substr(0, 8).c_str());

	// Standardizes the operation code to not be empty (81)
	if(driverStream.str() == "01")
	{
		// Convert all driver info id to Hex
		std::vector<unsigned char> driverElements {
			(unsigned char)hsyns.at(2), (unsigned char)hsyns.at(3), (unsigned char)hsyns.at(4),
			(unsigned char)hsyns.at(5), (unsigned char)hsyns.at(6), (unsigned char)hsyns.at(7)
		};
		for(int elem : driverElements) driverStream << std::setw(2) << elem;
		lapso.ibtMotorista += driverStream.str().c_str();
	}

	Dbg(TAG "Parse driver id: %s", lapso.ibtMotorista.c_str());
	Dbg(TAG "Parse timestamp Hex 0x%x", hsyns.substr(8, 7).c_str());
	Dbg(TAG "Parse timestamp %d", lapso.timestamp);
	lapso.odometro = BTCloud::Util::ParseHodometer(hsyns.substr(15, 3));

	Dbg(TAG "Parse odometer %d", lapso.odometro);
	Dbg(TAG "%d %d %d %d - %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)hsyns.at(0)), hex, setw(2), setfill('0'), int((unsigned char)hsyns.at(hsyns.length() - 1)));
	lapso.horimetro = BTCloud::Util::ParseHourmeter(hsyns.substr(18, 3));

	Dbg(TAG "Parse horimeter %d", lapso.horimetro);

	pLapso = BTCloud::Util::PersistableLapse(&lapso);

	header.file = arquivo;
	header.endPointer = ponteiroFim;
	header.timestamp = lapso.timestamp;

	header.dataType = Bluetec::enumDataType::HSYNS;

	Dbg(TAG "IMC 1 - header.dataType = %d", header.dataType);
	Dbg(TAG "Saving HSYNS %d .. ", header.endPointer);

	// Save route start
	Dbg(TAG "Save buffer file Header data type %d", header.dataType);
	Dbg(TAG "Save buffer file Enum data type %d", Bluetec::enumDataType::HSYNS);

	Dbg(TAG "IMC 2 - header.dataType = %d", header.dataType);
	cFileManager.SaveBufferFile(dataCache.veioId, pLapso.c_str(), pLapso.length(), header);
}

void Protocol::CreatePosition(bool isOdometerIncreased, bool isHourmeterIncreased)
{
	// Get all values from bluetec package
	int idEquipment = dataCache.id;
	int vehicle = dataCache.veioId;
	std::string plate = dataCache.plate;
	std::string client = dataCache.clientName;

	u_int64_t datePosition = pTelemetry->dateTime == 0
			? pTimer->GetCurrentTime()
			: pPosition->dateTime; // "2015-11-05T08:40:44.000Z"
	datePosition *= 1000;

	u_int64_t dateArrival = pPosition->dateArrive == 0
			? pTimer->GetCurrentTime()
			: pPosition->dateArrive; //"2015-11-05T08:40:44.000Z";
	dateArrival *= 1000;

	std::string address = "";
	std::string neighborhood = "";
	std::string city = "";
	std::string province = "";
	int velocity = pTelemetry->velocity;
	double lat2 = pPosition->lat;
	double long2 = pPosition->lon;
	std::string number = "";
	std::string country = "";
	std::string velocityStreet = "";
	bool gps = true;
	std::string driverId = pPosition->inf_motorista.id;

	int32_t analogic1 = pTelemetry->an1;
	int32_t analogic2 = pTelemetry->an2;
	int32_t analogic3 = pTelemetry->an3;
	int32_t analogic4 = pTelemetry->an4;
	bool digital1 = pTelemetry->ed7;
	bool digital2 = pTelemetry->ed6;
	bool digital3 = pTelemetry->ed5;
	bool digital4 = pTelemetry->ed4;
	double_t horimeter = pTelemetry->hourmeter;
	double_t accelerometerX = pTelemetry->acelX;
	double_t accelerometerY = pTelemetry->acelY;
	double_t hodometer = pTelemetry->odometer;
	int32_t rpm = pTelemetry->rpm;
	bool ignition = pEventFlag->ignition;
	bool breaks = pTelemetry->ed8;

	// Create BSON to be persisted
	mongo::BSONObj dataPosJSON = BSON(
				"id_equipamento" << idEquipment << "veiculo" << vehicle << "placa" << plate << "cliente" << client <<
				"data_posicao" << mongo::Date_t(datePosition) << "data_chegada" << mongo::Date_t(dateArrival) <<
				"velocidade" << velocity <<	"endereco" << address << "bairro" << neighborhood << "municipio" << city
				<< "estado" << province <<
				"coordenadas" << BSON("Type" << "Point" << "coordinates" << BSON_ARRAY(long2 << lat2)) << "numero" << number <<
				"pais" << country << "velocidade_via" << velocityStreet << "gps" << gps << "motorista_ibutton" << driverId <<
				"entradas" << BSON("ignicao" << ignition << "entrada1" << false << "entrada2" << false << "entrada3" << false <<
					"entrada4" << false << "entrada5" << false << "entrada6" << false << "entrada7" << false) <<
				"saidas" << BSON("saida0" << false << "saida1" << false << "saida2" << false << "saida3" << false <<
								 "saida4" << false << "saida5" << false << "saida6" << false << "saida7" << false) <<
				"odometro_adicionado" << isOdometerIncreased << "horimetro_adicionado" << isHourmeterIncreased <<
				"DadoLivre" << BSON(
					"Analogico1" << analogic1 << "Analogico2" << analogic2 << "Analogico3" << analogic3 <<
					"Analogico4" << analogic4 << "Horimetro" << horimeter << "AcelerometroX" << accelerometerX <<
					"Digital1" << digital1 << "Digital2" << digital2 << "Digital3" << digital3 <<
					"Digital4" << digital4 << "AcelerometroY" << accelerometerY << "Hodometro" << hodometer <<
					"Rpm" << rpm << "Freio" << breaks
					)
				);

	Log(TAG "%s %s", dataPosJSON.toString().c_str(), pConfiguration->GetMongoDBCollections().at(0).c_str());

	try
	{
		// Insert json data at posicao
		pDBClientConnection->insert(pConfiguration->GetMongoDBCollections().at(0), dataPosJSON);

		// Verify if has a valid position
		if(abs(long2) != 0 && abs(lat2) != 0)
		{
			// Get last position
			uint64_t lastPositionDate = GetLastPosition(vehicle, datePosition);

			// Insert/Update data at ultima_posicao
			if(lastPositionDate == 0)
			{
				Dbg(TAG "Inserting position at mongodb collection ultima_posicao");
				pDBClientConnection->insert(pConfiguration->GetMongoDBCollections().at(1), dataPosJSON);
			}
			else if(lastPositionDate < datePosition)
			{
				Dbg(TAG "Updating position at mongodb collection ultima_posicao");
				UpdateLastPosition(vehicle, datePosition, dateArrival);
			}
			else
				Dbg(TAG "Skipping position at mongodb collection ultima_posicao");
		}
	}
	catch(std::exception &e)
	{
		Error(TAG "Error on insert/update data at mongo: %s", e.what());
	}
}

uint64_t Protocol::GetLastPosition(uint32_t vehicleId, uint64_t lastPositionDate)
{
	// Use a cached var to evade query execute
	if(iLastPositionDate == lastPositionDate && iLastPositionVehicle == vehicleId)
		return iLastPositionDate;

	mongo::Query query = QUERY("veiculo" << vehicleId);

	Dbg(TAG "Has last position %s: %s", pConfiguration->GetMongoDBCollections().at(1).c_str(), query.toString().c_str());

	auto_ptr<mongo::DBClientCursor> cursor = pDBClientConnection->query(pConfiguration->GetMongoDBCollections().at(1), query);

	// Verify if it has a register at collection
	if(cursor->more() > 0)
	{
		mongo::BSONObj query_res = cursor->next();
		mongo::Date_t positionDate = query_res.getField("data_posicao").date();

		// Set a cache value to not execute the query all the time
		iLastPositionDate = positionDate.millis;
		iLastPositionVehicle = vehicleId;

		return positionDate.millis;
	}
	else
		return 0;
}

void Protocol::UpdateLastPosition(int vehicleId, u_int64_t datePosition, u_int64_t dateArrival)
{
	mongo::Query query = QUERY("veiculo" << vehicleId);

	// Create update query
	mongo::BSONObj querySet = BSON("$set" << BSON(
										"data_posicao" << mongo::Date_t(datePosition) <<
										"data_chegada" << mongo::Date_t(dateArrival) <<
										"motorista_ibutton" << pPosition->inf_motorista.id <<
										"velocidade" << pTelemetry->velocity <<
										"coordenadas" << BSON("Type" << "Point" <<
															"coordinates" << BSON_ARRAY(pPosition->lon << pPosition->lat))
										)
							);

	Dbg(TAG "Update querySet %s: %s", pConfiguration->GetMongoDBCollections().at(1).c_str(), querySet.toString().c_str());

	// Updating based on vehicle id with multiple parameter
	pDBClientConnection->update(pConfiguration->GetMongoDBCollections().at(1), query, querySet, false, true);
}

void Protocol::ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo, bool isFinalRoute)
{
	Bluetec::HeaderDataFile header;
	Bluetec::HeaderDataFile hsynsHeader;
	char tBuffer[500000];
	uint32_t tSize=0;
	Bluetec::HFull hfull;
	struct BTCloud::Util::Lapse lapso;
	int tipoDado = Bluetec::enumDataType::DADOS;
	int index = 0;
	int fim = 0;
	bool hasLapses = false;

	lapso.idTrecho = 0;
	lapso.timestamp = 0;
	lapso.velocidade = 0;
	lapso.rpm = 0;
	lapso.odometro = 0;
	lapso.horimetro = 0;
	lapso.ed1 = 0;
	lapso.ed2 = 0;
	lapso.ed3 = 0;
	lapso.ed4 = 0;
	lapso.ed5 = 0;
	lapso.ed6 = 0;
	lapso.acelx = 0;
	lapso.acely = 0;
	lapso.an1 = 0;
	lapso.an2 = 0;
	lapso.an3 = 0;
	lapso.an4 = 0;

	Dbg(TAG "Processing data %d %d...", ponteiroIni, ponteiroFim);
	Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)dados.at(0)), hex, setw(2), setfill('0'), int((unsigned char)dados.at(dados.length()-1)));

	if(!cFileManager.GetHfull(dataCache.veioId, hfull))
	{
		hfull.lapso = Bluetec::enumDefaultValues::LAPSO;
		hfull.acely = Bluetec::enumDefaultValues::ACELY;
		hfull.acelx = Bluetec::enumDefaultValues::ACELX;
		hfull.spanAcel = Bluetec::enumDefaultValues::SPANACEL;
	}

	// TODO: Validate warning
	// Case some data exists
	if((cFileManager.getBufferFile(dataCache.veioId, ponteiroIni, arquivo, tBuffer, tSize, header) &&
			(header.endPointer == ponteiroIni) && header.file == arquivo)
		|| (cFileManager.getBufferFile(dataCache.veioId, ponteiroIni-1, arquivo, tBuffer, tSize, header) &&
						(header.endPointer == ponteiroIni-1) && header.file == arquivo))
	{
		Dbg(TAG "Found data before this file with type %d", header.dataType);
		Dbg(TAG "IMC 3 - header.dataType = %d", header.dataType);
		Dbg(TAG "Found with end point %d", header.endPointer);

		// Temp + data
		ponteiroIni = header.beginPointer;
		lapso.timestamp = header.timestamp;
		switch(header.dataType)
		{
			case Bluetec::enumDataType::HSYNS_DADOS:

			case Bluetec::enumDataType::HSYNS:

				// This route was already started then the first lapse must be restored
				BTCloud::Util::LapsoSetup(tBuffer, lapso);
				tipoDado = isFinalRoute ? Bluetec::enumDataType::HSYNS_FINAL : Bluetec::enumDataType::HSYNS_DADOS;

				// Clean the temporary lapse
				cFileManager.DelFile(dataCache.veioId, header.headerFile);

				// Cache header info
				hsynsHeader = header;

				hasLapses = true;
			break;

			case Bluetec::enumDataType::DADOS:
				// This case of data shard found does not have a meaning, it will be updated
				dados = string(tBuffer) + dados;

				// Clean file temporarily to overwrite it
				cFileManager.DelFile(dataCache.veioId, header.headerFile);
				tipoDado = Bluetec::enumDataType::DADOS;
			break;
		}
	}

	// TODO: Validate warning
	if((cFileManager.getBufferFile(dataCache.veioId, ponteiroFim + 1, arquivo, tBuffer, tSize, header) &&
			(header.beginPointer == ponteiroFim + 1) &&	header.file == arquivo)
		|| (cFileManager.getBufferFile(dataCache.veioId, ponteiroFim, arquivo, tBuffer, tSize, header) &&
				(header.beginPointer == ponteiroFim) &&	header.file == arquivo))
	{
		// Data + Temp
		ponteiroFim = header.endPointer;

		Dbg(TAG "Found data after this file with type %d", header.dataType);
		Dbg(TAG "Found with end point %d", header.endPointer);

		//lapso.timestamp = header.timestamp;

		switch(header.dataType)
		{
			case Bluetec::enumDataType::DADOS:
				// This case of data shard found does not have a meaning, it will be updated
				dados += string(tBuffer, tSize);

				// Clean file temporarily to overwrite it
				cFileManager.DelFile(dataCache.veioId, header.headerFile);
				tipoDado = Bluetec::enumDataType::DADOS;
			break;

			case Bluetec::enumDataType::FINAL:

			case Bluetec::enumDataType::DADOS_FINAL:
				if(header.dataType == Bluetec::enumDataType::DADOS_FINAL)
					dados += string(tBuffer, tSize);

				// Clean file temporarily to overwrite it
				cFileManager.DelFile(dataCache.veioId, header.headerFile);

				// Verify if has an opened route
				if(tipoDado != Bluetec::enumDataType::DADOS)
					tipoDado = Bluetec::enumDataType::HSYNS_FINAL;
				else
					tipoDado = Bluetec::enumDataType::DADOS_FINAL;
			break;
		}
	}

	if(tipoDado == Bluetec::enumDataType::DADOS && !hasLapses)
	{
		Dbg(TAG "Unknown route");

		header.beginPointer = ponteiroIni;
		header.endPointer = ponteiroFim;
		header.file = arquivo;
		header.dataType = tipoDado;

		Dbg(TAG "Save buffer file header data type %d", header.dataType);
		cFileManager.SaveBufferFile(dataCache.veioId, dados.c_str(), dados.length(), header);
	}
	else
	{
		header.beginPointer = hsynsHeader.beginPointer;
		header.dataType = hsynsHeader.dataType;
		header.timestamp = hsynsHeader.timestamp;

		ParseLapse(lapso, dados, hfull, index, fim);

		if(tipoDado != Bluetec::enumDataType::HSYNS_FINAL)
		{
			// Case do not exist a end of route, the last lapse must be returned
			// to persist and to not reprocess
			header.idTrecho = lapso.idTrecho;
			header.timestamp = lapso.timestamp;
			header.endPointer = ponteiroFim;
			Dbg(TAG "-> ibtMotorista: %s", lapso.ibtMotorista.c_str());
			string pLapso = BTCloud::Util::PersistableLapse(&lapso);

			Dbg(TAG "Save buffer file header data type %d", header.dataType);
			cFileManager.SaveBufferFile(dataCache.veioId, pLapso.c_str(), pLapso.length(), header);
		}
	}
}

void Protocol::ParseLapse(BTCloud::Util::Lapse &lapso, string dados, Bluetec::HFull hfull, int index, int fim)
{
	BTCloud::Util::Output *controle;
	char bufferControle[255];
	char bufferExpansao[255];
	BTCloud::Util::Output *expansao;
	string operacao;
	int tamLapso = 0;
	char bufferED[255];
	BTCloud::Util::Output *ed;

	int maxSizePacote = 1500;
	int sizePacote = 0;

	// When a route is known, so it exists and it can be processed
	Dbg(TAG "Known route, processing...");
	fim = dados.length();
	bool isHourmeterIncreased = false;
	bool isOdometerIncreased = false;

	while(index < fim)
	{
		Dbg(TAG "%d < %d", index, fim);
		try
		{
			// Calc control byte size
			bufferControle[0] = dados.at(index);
			controle = (BTCloud::Util::Output*) bufferControle;
			tamLapso = BTCloud::Util::LapseSize(controle);

			// Case the expasion bit is enable, calc the expasion byte
			if(controle->saida0 && index + 1 <= fim)
			{
				// The expasion byte comes after control byte, so increment it to calculate correctly next data
				index++;

				bufferExpansao[0] = dados.length() == index ? dados.at(index - 1) : dados.at(index);
				expansao = (BTCloud::Util::Output*) bufferExpansao;
				tamLapso += (BTCloud::Util::ExpasionSize(expansao) + 1);
			}

			// Validate route lapse at this point
			if(index + tamLapso <= fim)
			{
				// Speed
				if(controle->saida5)
				{
					index++;
					lapso.velocidade = ((double) ((unsigned char) dados.at(index)));
				}
				// Rpm
				if(controle->saida4)
				{
					index++;
					int rpm = 0;

					try
					{
						rpm = (int) ((unsigned char) dados.at(index));
					}
					catch (const std::out_of_range& e){}

					lapso.rpm = rpm * 50;
				}
				// Acel x
				if(controle->saida3)
				{
					index++;
					lapso.acelx = 0;
				}
				// Acel y
				if(controle->saida2)
				{
					index++;
					lapso.acely = 0;
				}
				// Ed
				if(controle->saida1)
				{
					index++;
					bufferED[0] = dados.length() == (unsigned int)index ? dados.at(index - 1) : dados.at(index);
					ed = (BTCloud::Util::Output*) bufferED;

					lapso.ed1 = (unsigned int) ed->saida0;
					lapso.ed2 = (unsigned int) ed->saida1;
					lapso.ed3 = (unsigned int) ed->saida2;
					lapso.ed4 = (unsigned int) ed->saida3;
					lapso.ed5 = (unsigned int) ed->saida4;
					lapso.ed6 = (unsigned int) ed->saida5;
					lapso.ed7 = (unsigned int) ed->saida6;
					lapso.ed8 = (unsigned int) ed->saida7;
				}

				// Verify if exists a expasion byte, this time to process it
				if(controle->saida0)
				{
					Dbg(TAG "Expansion");

					// An 1
					if(expansao->saida7)
					{
						Dbg(TAG "An 1");
						lapso.an1 = 0;
						index += 2;
					}
					// An 2
					if(expansao->saida6)
					{
						Dbg(TAG "An 2");
						lapso.an2 = 0;
						index++;
					}
					// An 3
					if(expansao->saida5)
					{
						Dbg(TAG "An 3");
						lapso.an3 = 0;
						index++;
					}
					// An 4
					if(expansao->saida4)
					{
						Dbg(TAG "An 4");
						lapso.an4 = 0;
						index++;
					}
					// Verify if there is a operation information
					if(expansao->saida3)
					{
						operacao = dados.substr(index + 1, 7);
						lapso.operacao = operacao;
						unsigned char controleOper = operacao.at(0);
						index += 7;

						// Verify if is a GPS
						if(controleOper >= 0xA0)
						{
							Dbg(TAG "GPS");

							char buffer[255];
							buffer[0] = controleOper;
							BTCloud::Util::Output *p;
							p = (BTCloud::Util::Output*) buffer;
							double lat = BTCloud::Util::ParseLatitude(operacao.substr(1, 3), p->saida2);
							double lon = BTCloud::Util::ParseLongitude(operacao.substr(4, 3), p->saida1, p->saida0);

							Dbg(TAG "Lat Long -> %20.18f %20.18f", lat, lon);

							// Setting data to position package
							pPosition->lat = lat;
							pPosition->lon = lon;
						}
					}

					// Increase odometer
					if(expansao->saida2)
					{
						lapso.odometro += 1;
						isOdometerIncreased = true;
					}
					else isOdometerIncreased = false;

					// Increase horimeter
					if(expansao->saida1)
					{
						lapso.horimetro += 0.1;
						isHourmeterIncreased = true;
					}
					else isHourmeterIncreased = false;

					// Empty
					if(expansao->saida0)
					{
					}
				}

				// Set protobuf vars with lapse values
				index++;

				// Validates package size
				sizePacote += iLapsoSize;

				if(sizePacote > maxSizePacote)
				{
					Dbg(TAG "Maximum size reached, sending package with %d lapses with %d bytes", sizePacote / iLapsoSize, iLapsoSize);

					// Reset the value of package with the size of lapse
					sizePacote = iLapsoSize;
				}
				else
				{
					BTCloud::Util::LapsoToTelemetry(pTelemetry, lapso);
				}

				pPosition->dateTime = lapso.timestamp;
				pPosition->dateArrive = pTimer->GetCurrentTime();
				pPosition->inf_motorista.id = lapso.ibtMotorista;

				if(lapso.rpm > 0)
					pEventFlag->ignition = 1;
				else
					pEventFlag->ignition = 0;

				pOdoVel->velocity = lapso.velocidade;

				Dbg(TAG "Timestamp before transmition %d", lapso.timestamp);
				Dbg(TAG "Position found, sending package with %d lapses of %d bytes", sizePacote / iLapsoSize, iLapsoSize);

				// Save JSON at MongoDB
				CreatePosition(isOdometerIncreased, isHourmeterIncreased);

				// Reset entity
				cPackage.Clear();

				// Reset maximum size of packet
				sizePacote = 0;

				lapso.timestamp += hfull.lapso;
			}
			else
			{
				index = fim;
			}
		}
		catch(exception& e)
		{
			Error(TAG "Index error %d %s", index, e.what());
		}
	}
}

void Protocol::GetClientData(DataCache &retorno, std::string chave)
{
	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());
	Dbg(TAG "Connected to MySQL");

	string query("");
	query.append("SELECT V.veioid AS veiculo_id, "
					"V.vei_placa AS placa, "
					"E.equip_id AS equip_id, "
					"C.clinome AS cliente "
				"FROM clientes C "
				"JOIN veiculos V ON V.vei_clioid=C.id "
				"JOIN equipamentos E ON V.vei_equoid=E.equip_id "
				"WHERE "
					"V.vei_placa = '")
			.append(chave)
			.append("'");

	Dbg(TAG "Query: %s", query.c_str());

	if(pMysqlConnector->Execute(query))
	{
		auto mysqlResult = pMysqlConnector->Result();
		if(mysqlResult)
		{
			auto mysqlRow = pMysqlConnector->FetchRow(mysqlResult);
			if(mysqlRow)
			{
				retorno.veioId = atoi(mysqlRow[0]);
				retorno.plate = mysqlRow[1];
				retorno.id = atoi(mysqlRow[2]);
				retorno.clientName = mysqlRow[3];

				Dbg(TAG "Vehicle id: %d", retorno.veioId);
				Dbg(TAG "Plate: %s", retorno.plate.c_str());
				Dbg(TAG "Equipment id: %d", retorno.id);
				Dbg(TAG "Client name: %s", retorno.clientName.c_str());

				pMysqlConnector->FreeResult(mysqlResult);
			}
		}
	}

	pMysqlConnector->Disconnect();
}

uint32_t Protocol::GetClient(std::string clientName)
{
	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	uint32_t clientId = 0;

	string query("");
	query.append("SELECT id FROM clientes WHERE clinome = '").append(clientName).append("'");
	Dbg(TAG "Query: %s", query.c_str());

	// Get client id
	if(pMysqlConnector->Execute(query))
	{
		Dbg(TAG "Query executed");
		auto mysqlResult = pMysqlConnector->Result();
		if(mysqlResult)
		{
			Dbg(TAG "Query resulted");

			auto mysqlRow = pMysqlConnector->FetchRow(mysqlResult);
			if(mysqlRow)
			{
				clientId = atoi(mysqlRow[0]);
				pMysqlConnector->FreeResult(mysqlResult);
			}
		}
	}

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	Dbg(TAG "ClientID: %d", clientId);
	return clientId;
}

uint32_t Protocol::CreateClient(std::string clientName)
{
	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	string query("");
	query.append("INSERT INTO clientes SET clinome = '").append(clientName).append("'");
	Dbg(TAG "Query: %s", query.c_str());

	bool exec = pMysqlConnector->Execute(query);
	Dbg(TAG "Query executed: %d", exec);

	int id = 0;

	// return inserted registry id
	if(exec) id = pMysqlConnector->InsertedID();

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	Dbg(TAG "Inserted client id: %d", id);
	return id;
}

uint32_t Protocol::CreateEquipmentImei()
{
	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	uint32_t id = 0;

	string query("");
	query.append("SELECT (RAND() * 9999999999999) + 99");
	Dbg(TAG "Query: %s", query.c_str());

	// Get vehicle id
	if(pMysqlConnector->Execute(query))
	{
		Dbg(TAG "Query executed");
		auto mysqlResult = pMysqlConnector->Result();
		if(mysqlResult)
		{
			Dbg(TAG "Query resulted");

			auto mysqlRow = pMysqlConnector->FetchRow(mysqlResult);
			if(mysqlRow)
			{
				id = atoi(mysqlRow[0]);
				pMysqlConnector->FreeResult(mysqlResult);
			}
		}
	}

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	return id;
}

uint32_t Protocol::CreateEquipment(uint32_t projectId, uint32_t equipIMei)
{
	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	string query("");

	query.append("INSERT INTO equipamentos SET ")
			.append("projetos_proj_id = ").append(std::to_string(projectId))
			.append(", equip_imei = ").append(std::to_string(equipIMei));

	Dbg(TAG "Query: %s", query.c_str());

	bool exec = pMysqlConnector->Execute(query);
	Dbg(TAG "Query executed: %d", exec);

	int id = 0;

	// return inserted registry id
	if(exec)
	{
		string query("");
		query.append("SELECT equip_id FROM equipamentos WHERE equip_imei = ").append(std::to_string(equipIMei));
		Dbg(TAG "Query: %s", query.c_str());

		// Get equip id
		if(pMysqlConnector->Execute(query))
		{
			Dbg(TAG "Query executed");
			auto mysqlResult = pMysqlConnector->Result();
			if(mysqlResult)
			{
				Dbg(TAG "Query resulted");

				auto mysqlRow = pMysqlConnector->FetchRow(mysqlResult);
				if(mysqlRow)
				{
					id = atoi(mysqlRow[0]);
					pMysqlConnector->FreeResult(mysqlResult);
				}
			}
		}
	}

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	Dbg(TAG "Inserted equipment id: %d", id);
	return id;
}

uint32_t Protocol::GetVehicle(uint32_t equipId)
{
	uint32_t vehicleId = 0;

	string query("");
	query.append("SELECT veioid FROM veiculos WHERE vei_equoid = ").append(std::to_string(equipId));
	Dbg(TAG "Query: %s", query.c_str());

	// Get vehicle id
	if(pMysqlConnector->Execute(query))
	{
		Dbg(TAG "Query executed");
		auto mysqlResult = pMysqlConnector->Result();
		if(mysqlResult)
		{
			Dbg(TAG "Query resulted");

			auto mysqlRow = pMysqlConnector->FetchRow(mysqlResult);
			if(mysqlRow)
			{
				vehicleId = atoi(mysqlRow[0]);
				pMysqlConnector->FreeResult(mysqlResult);
			}
		}
	}

	Dbg(TAG "ClientID: %d", vehicleId);
	return vehicleId;
}

uint32_t Protocol::CreateVehicle(uint32_t clientId, uint32_t equipId, std::string plate)
{

	// Init mysql client
	pMysqlConnector->Initialize();

	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	//int vehicleId = GetVehicle(equipId);
	int id = 0;

	string query("");

	// FIX: Deleting and then inserting due update problem where it not affect row
	/*query.append("DELETE FROM veiculos WHERE veioid = ").append(std::to_string(vehicleId));
	Dbg(TAG "Delete Query: %s", query.c_str());

	bool exec = pMysqlConnector->Execute(query);
	Dbg(TAG "Delete query executed: %d", exec);

	query.append("UPDATE veiculos SET ")
			.append("vei_clioid = ").append(std::to_string(clientId))
			.append(", vei_placa = '").append(plate).append("'")
			.append(", vei_alias = '").append(plate).append("'")
			.append(" WHERE veioid = ").append(std::to_string(vehicleId));

	query.clear();*/

	query.append("INSERT INTO veiculos SET ")
			.append("vei_clioid = ").append(std::to_string(clientId))
			.append(", vei_equoid = ").append(std::to_string(equipId))
			.append(", vei_placa = '").append(plate).append("'");

	Dbg(TAG "Query: %s", query.c_str());

	bool exec = pMysqlConnector->Execute(query);
	Dbg(TAG "Query executed: %d", exec);

	// return inserted registry id
	if(exec) id = GetVehicle(equipId);

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	/*Dbg(TAG "Updated vehicle vei_equoid: %d", equipId);
	return vehicleId;*/

	Dbg(TAG "Inserted vehicle id: %d", id);
	return id;
}

void Protocol::FillDataContract(std::string clientName, std::string plate, DataCache &retorno)
{
	// Use a local cache to avoid access mysql db
	/*if(retorno.plate.compare(plate) != 0)
	{*/
		retorno.veioId = 0;
		retorno.esn = atoi(plate.c_str());

		// Get client from mysql database
		GetClientData(retorno, plate);

		// Case this plate does not exist, create it
		if(retorno.veioId == 0)
		{
			uint32_t clientId = GetClient(clientName);

			// Create a client case it does not exist
			if(!clientId)
				clientId = CreateClient(clientName);

			// Generate equipment imei
			srand(time(NULL));
			uint32_t imei = CreateEquipmentImei();

			// FIX: Verify why 0 is returned from pConfiguration
			uint32_t equipId = CreateEquipment(pConfiguration->GetProjectId(), imei);

			uint32_t vehiId = CreateVehicle(clientId, equipId, plate);

			retorno.veioId = vehiId;
			retorno.plate = plate;
			retorno.id = equipId;
			retorno.clientName = clientName;
		}
	//}
}

void Protocol::Process(const char *path, int len, mongo::DBClientConnection *dbClient)
{
	cFileManager.SetPath(pConfiguration->GetAppListeningPath());

	pDBClientConnection = dbClient;

	FILE *in = 0;
	unsigned char bt4[6500];
	std::string sbt4;

	Dbg(TAG "Processing: %s", path);

	// Tokenize path to get client, plate and file
	std::vector<std::string> tokens;
	tokens = StringUtil::split(path, '/');

	// The client name is the parent folder at -3 position
	std::string clientName(tokens[tokens.size() - 3]);
	std::transform(clientName.begin(), clientName.end(), clientName.begin(), ::toupper);

	// The plate name is the subfolder at -2 position
	std::string plate(tokens[tokens.size() - 2]);
	std::transform(plate.begin(), plate.end(), plate.begin(), ::toupper);

	// The file name is the last position
	std::string fileName(tokens[tokens.size() - 1]);
	std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::toupper);

	Info(TAG "Processing file: %s", fileName.c_str());

	// Verify the name of file to contains BT4
	if(fileName.length() < 3 || (fileName.substr(0, 3) != "BT4"))
	{
		Dbg(TAG "Ignoring %s, it is not in BT4 format", fileName.c_str());
		return;
	}

	// Create or use a contract data
	FillDataContract(clientName, plate.c_str(), dataCache);

	// Open the data file
	in = fopen(path, "rb");

	if(in)
	{
		Dbg(TAG "Loading file - %s", fileName.c_str());

		size_t lSize;
		size_t result;

		// Read file
		fseek(in, 0, SEEK_END);
		lSize = ftell(in);
		rewind(in);
		result = fread(bt4, 1, lSize, in);

		// Validate the content of file
		if((result != lSize) || (lSize == 0))
		{
			Error(TAG "Reading error");
			return;
		}

		std::string sbt4(bt4, bt4 + lSize);

		int arquivo = (int) sbt4[1];

		Dbg(TAG "File: %d", arquivo);

		long ponteiroIni = ((long ) bt4[2] * 65536) + ((long) bt4[3] * 256) + (long ) bt4[4];
		Dbg(TAG "Start point %d * 65536 + %d * 256 + %d = %d", (long) bt4[2], (long) bt4[3], (long) bt4[4], ponteiroIni);

		long ponteiroFim = ((long) bt4[lSize - 8] * 65536 ) + ((long) bt4[lSize - 7] * 256 ) + (long) bt4[lSize - 6];
		Dbg(TAG "End point %d", ponteiroFim);

		// For each 1200 bytes, the bluetec protocol generate a header with 5 characters at begin and 8 at end
		// this characters must be removed

		// FIX: std::out_of_range
		try
		{
			sbt4.erase(0, 5);
			sbt4.erase(sbt4.length() - 8, 8);
		}
		catch (const std::out_of_range& e)
		{
			Error(TAG "Error %s. sbt4.substr(%d - 8, 8)", e.what(), sbt4.length());
		}

		lSize -= 13;

		for(size_t i = 1200; i < sbt4.length(); i += 1200)
		{
			// Removes 13 bytes(5 at start + 8 at end) in a transaction from a subpackage to another
			sbt4.erase(i, 13);
			lSize -= 13;
		}

		Dbg(TAG "Starting lapses");

		//int tipo_dado = bluetec::enumDataType::DADOS;
		size_t inicio = 0;

		// Header offsets
		size_t lFimTrecho = 12;
		size_t lHsync = 21;
		size_t lHsyns = 21;
		size_t lHfull = 250;

		string fimTrecho;
		fimTrecho.push_back((unsigned char ) 0x82);
		fimTrecho.push_back((unsigned char ) 0xA3);
		fimTrecho.push_back((unsigned char ) 0xA5);
		fimTrecho.push_back((unsigned char ) 0xA7);

		Dbg(TAG "Searching headers");

		for(size_t i = 0; i < sbt4.length(); i++)
		{
			if(sbt4.at(i) == (unsigned char) 0x48)
			{
				if(sbt4.compare(i, 5, "HSYNC") == 0)
				{
					Dbg(TAG "Found the HSYNC in %d", i);
					if(inicio < i - lHsync)
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, i - lHsync - 1);

						// Parse data
						Dbg(TAG "ParseData HSYNS until HSYNC %d", ponteiroIni + inicio);
						ParseData(sbt4.substr(inicio, i - inicio - lHsync), ponteiroIni + inicio, ponteiroIni + i-lHsync - 1, arquivo, false);
						inicio = i - lHsync;
					}
					if(inicio == i -  lHsync)
					{
						Dbg(TAG "Creating file of type 2 from %d a %d", inicio, i - 1);

						// Parse data
						//parseHSYNC( sbt4.substr( i - 21, 26 ) );
						try
						{
							Dbg(TAG "ParseHSYNC(sbt4.substr(%d, %d), %d, %d + %d + %d)", inicio, lHsync, arquivo, ponteiroIni, i, 4);
							ParseHSYNC(sbt4.substr(inicio, lHsync), arquivo, ponteiroIni + i + 4);
						}
						catch (const std::out_of_range& e)
						{

							Error(TAG "%s/%s/%s Error %s. HSYNC sbt4.substr(%d, %d)", clientName.c_str(), plate.c_str(), fileName.c_str(), e.what(), inicio, lHsync);
						}

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", inicio, i);
				}
				else if(sbt4.compare(i, 5, "HSYNS") == 0)
				{
					Dbg(TAG "Found the HSYNS in %d", i);
					if(inicio < i - lHsyns)
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, lHsyns - 1);

						// Parse data
						Dbg(TAG "ParseData HSYNS %d", ponteiroIni + inicio);
						ParseData(sbt4.substr(inicio, i - inicio - lHsyns), ponteiroIni + inicio, ponteiroIni + i-lHsyns - 1, arquivo, false);
						inicio = i - lHsyns;
					}
					if(inicio == i -  lHsyns)
					{
						Dbg(TAG "Creating file of type 3 from %d a %d", inicio, lHsyns - 1);

						// i is in 'H', count plus 4 pointers ahead, 'H' + 'SYNS'
						Dbg(TAG "Params HSYNS %d %d %d + %d + 4", inicio, arquivo, ponteiroIni, i);

						Dbg(TAG "sbt4.substr(%d, %d)", inicio, lHsyns);

						// Parse HSYNS

						// FIX: Change to parse a HSYNS splited into 2 files
						try
						{
							Dbg(TAG "ParseHSYNS(sbt4.substr(%d, %d), %d, %d + %d + %d)", inicio, lHsyns, arquivo, ponteiroIni, i, 4);
							ParseHSYNS(sbt4.substr(inicio, lHsyns), arquivo, ponteiroIni + i + 4);
						}
						catch (const std::out_of_range& e)
						{
							Error(TAG "%s/%s/%s Error %s. HSYNS sbt4.substr(%d, %d)", clientName.c_str(), plate.c_str(), fileName.c_str(), e.what(), inicio, lHsyns);
						}

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", inicio, i);
				}
				else if(sbt4.compare(i, 5, "HFULL") == 0)
				{
					Dbg(TAG "Found the HFULL in %d", i);
					if(inicio < i - lHfull)
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, lHfull - 1);

						// Parse data
						ParseData(sbt4.substr(inicio, i - inicio - lHfull), ponteiroIni + inicio, ponteiroIni + i-lHfull - 1, arquivo, false);
						inicio = i - lHfull;
					}

					if(inicio == i -  lHfull)
					{
						Dbg(TAG "Creating file of type 1 from %d a %d", inicio, i - 1);

						// Parse data
						// FIX: Change to parse a HFULL
						try
						{
							ParseHFULL(sbt4.substr(inicio, i-inicio), ponteiroIni+inicio, i+4, arquivo);
						}
						catch (const std::out_of_range& e)
						{
							Error(TAG "Error %s. ParseHFULL");
						}

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", inicio, i);
				}
			}
			else if(i+4 < sbt4.length() && (unsigned char)sbt4.at(i) == (unsigned char)0x82 &&
					sbt4.compare(i,4,fimTrecho) == 0)
			{
				Dbg(TAG "Found the end of route in %d", i);
				if(inicio < i)
				{
					Dbg(TAG "Creating file of type 6 from %d a %d", inicio, i - 1);

					// Parse data
					ParseData(sbt4.substr(inicio, i - inicio), ponteiroIni + inicio, ponteiroIni + i-1, arquivo, true);
					inicio = i;
				}
				if( inicio == i && i + lFimTrecho <= sbt4.length())
				{
					Dbg(TAG "Closing the route of final point %d", i - 1);

					// Parse data
					//ParseA3A5A7(ponteiroIni + i, arquivo, 0);

					i += lFimTrecho + 1;
					inicio = i;
				}
				Dbg(TAG "Start %d i %d", inicio, i);
			}
		}

		if(inicio < sbt4.length())
		{
			Dbg(TAG "Sending type 6 at the end of buffer %d a %d", inicio, sbt4.length() - 1);

			// Parse data
			ParseData(sbt4.substr(inicio, sbt4.length()-inicio), ponteiroIni + inicio, ponteiroFim, arquivo, false);
		}

		fclose(in);

		// Rename file to set as processed
		string newPath = "";
		Util::CreateFileNameProcessed(&newPath, tokens);
		cFileManager.RenameFile(path, newPath);
	}
}

}
