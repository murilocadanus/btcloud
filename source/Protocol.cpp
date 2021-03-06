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
#include "mongo/util/assert_util.h"
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
	pData = nullptr;
	pTelemetry = nullptr;
	pPosition = nullptr;
	pEventFlag = nullptr;
	pOdoVel = nullptr;
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
	hfull.configHardware = (unsigned int)strHfull.at(12);
	//hfull.reservado[5];
	//hfull.constanteVelocidade2;
	//hfull.constanteVelocidade1;
	//hfull.constanteRPM;
	//hfull.confDiversas;
	//hfull.reservado1[8];
	hfull.acelx = (unsigned int) strHfull.at(33);
	//hfull.reservado2[3];
	hfull.acely = (unsigned int) strHfull.at(37);
	//hfull.tipoAcelerometro;
	//hfull.limiteAcelFrenTom[3];
	hfull.spanAcel = (unsigned int) strHfull.at(42);
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
	//hfull.parametrosGerais[0] = (unsigned int) strHfull.at(70);
	//hfull.parametrosGerais[1] = (unsigned int) strHfull.at(71);
	//hfull.reservado5[7];
	//hfull.alarmes[58];
	//hfull.reservado6[6];
	//hfull.gerenciaMemoria1;
	//hfull.gerenciaMemoria2;
	hfull.reversao = (unsigned int) strHfull.at(145);
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
	bool hasBlock = false;
	Bluetec::HFull hfull;

	if(!cFileManager.GetHfull(dataCache.veioId, hfull))
	{
		hfull.configHardware = Bluetec::enumDefaultValues::HIGH_PRECISION_GPS;
	}

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

	// Get the current status of bluetec board computer
	switch ((unsigned char)hsync.at(8))
	{
		case Bluetec::enumHsyncBoardStatus::RESTART: // Equipment restart
			pEventFlag->block = 0;
			hasBlock = true;
		break;
		case Bluetec::enumHsyncBoardStatus::TURN_OFF: // Equipment shutdown
			pEventFlag->block = 1;
			hasBlock = true;
		break;
		default: hasBlock = false; break;
	}

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
	bool isGPS = (unsigned char) hsync.at(1) >= 0xA0 && !((hsync.at(1) >> 3) & 0x01);

	double lat = isGPS ? BTCloud::Util::ParseLatitude(hfull.configHardware, hsync.substr(2, 3), 1) : 0.0;
	double lon = isGPS ? BTCloud::Util::ParseLongitude(hfull.configHardware, hsync.substr(5, 3), 1, 0) : 0.0;

	Dbg(TAG "Lat Long -> %20.18f %20.18f", lat, lon);

	// Setting data to position package
	pPosition->lat = lat;
	pPosition->lon = lon;

	pPosition->dateTime = lapso.timestamp;
	pPosition->dateArrive = pTimer->GetCurrentTime();
	pPosition->inf_motorista.id = lapso.ibtMotorista;

	if(lapso.rpm > 0)
		pEventFlag->ignition = 1;
	else
		pEventFlag->ignition = 0;

	pOdoVel->velocity = lapso.velocidade;

	Dbg(TAG "Timestamp before transmition %d", lapso.timestamp);

	// Send position package to queue in final protobuf format
	//bluetecPacote.SerializeToString(&serializado);
	BTCloud::Util::LapsoToTelemetry(pTelemetry, lapso);

	// Set lapse raw data
	std::vector<unsigned char> rawLapse;
	const char *values = hsync.c_str();
	const char *end = values + hsync.size();
	rawLapse.insert(rawLapse.end(), values, end);

	// Save JSON at MongoDB
	CreatePosition(false, false, false, false, hasBlock, Bluetec::enumDataType::HSYNC, rawLapse);

	// Reset entity
	cPackage.Clear();
}

void Protocol::ParseA3A5A7(string a3a5a7)
{
	struct BTCloud::Util::Lapse lapso;

	// Set lapse raw data
	std::vector<unsigned char> rawLapse;
	const char *values = a3a5a7.c_str();
	const char *end = values + a3a5a7.size();
	rawLapse.insert(rawLapse.end(), values, end);

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

	std::string data = a3a5a7.substr(4, 6);

	struct tm *dataHora;
	time_t currentTime = 0;
	time(&currentTime);
	dataHora = localtime(&currentTime);

	dataHora->tm_year = (2000 + BTCloud::Util::ParseBCDDecimal(data.at(0))); //'YY'YYMMDDHHMMSS + YY'YY'MMDDHHMMSS
	dataHora->tm_mon = BTCloud::Util::ParseBCDDecimal(data.at(1)); //YYYY'MM'DDHHMMSS
	dataHora->tm_mday = BTCloud::Util::ParseBCDDecimal(data.at(2)); //YYYYMM'DD'HHMMSS
	dataHora->tm_hour = BTCloud::Util::ParseBCDDecimal(data.at(3)); //YYYYMMDD'HH'MMSS
	dataHora->tm_min = BTCloud::Util::ParseBCDDecimal(data.at(4)); //YYYYMMDDHH'MM'SS
	dataHora->tm_sec = BTCloud::Util::ParseBCDDecimal(data.at(5)); //YYYYMMDDHHMM'SS'

	dataHora->tm_mon -= 1;
	dataHora->tm_year -= 1900;

	lapso.timestamp = mktime(dataHora);

	lapso.ibtMotorista = "";
	pPosition->dateTime = lapso.timestamp;
	pPosition->dateArrive = pTimer->GetCurrentTime();
	pPosition->inf_motorista.id = lapso.ibtMotorista;
	lapso.odometro = BTCloud::Util::ParseHodometer(a3a5a7.substr(10, 3));

	pEventFlag->ignition = 0;
	pOdoVel->velocity = lapso.velocidade;

	// Send position package to queue in final protobuf format
	//bluetecPacote.SerializeToString(&serializado);
	BTCloud::Util::LapsoToTelemetry(pTelemetry, lapso);

	// Save JSON at MongoDB
	CreatePosition(false, false, true, false, false, Bluetec::enumDataType::FINAL, rawLapse);

	// Reset entity
	cPackage.Clear();
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

void Protocol::CreatePosition(bool isOdometerIncreased, bool isHourmeterIncreased, bool routeEnd, bool hasVelocity, bool hasBlock, Bluetec::enumDataType type, std::vector<unsigned char> rawLapse)
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
	double lat2 = pPosition->lat;
	double long2 = pPosition->lon;
	std::string number = "";
	std::string country = "";
	std::string velocityStreet = "";
	bool gps = true;
	std::string driverId = pPosition->inf_motorista.id;

	int velocity = pTelemetry->velocity;
	int32_t rpm = pTelemetry->rpm;
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
	bool ignition = pEventFlag->ignition;
	bool breaks = pTelemetry->ed8;
	bool block = pEventFlag->block;
	bool reverse = pEventFlag->reverse;

	// Create BSON to be persisted
	mongo::BSONObjBuilder dataJSON;
	mongo::BSONObjBuilder freeDataJSON;

	dataJSON << "id_equipamento" << idEquipment << "veiculo" << vehicle << "placa" << plate << "cliente" << client <<
		"data_posicao" << mongo::Date_t(datePosition) << "data_chegada" << mongo::Date_t(dateArrival) <<
		"endereco" << address << "bairro" << neighborhood << "municipio" << city << "numero" << number <<
		"estado" << province;

	if(hasVelocity) dataJSON << "velocidade" << velocity;

	if(abs(long2) != 0 && abs(lat2) != 0)
		dataJSON.appendElements(BSON("coordenadas" << BSON("Type" << "Point" << "coordinates" << BSON_ARRAY(long2 << lat2))));

	dataJSON << "pais" << country << "velocidade_via" << velocityStreet << "gps" << gps << "motorista_ibutton" << driverId;

	dataJSON.appendElements(BSON("entradas" <<
								BSON("ignicao" << ignition << "entrada1" << false <<
									"entrada2" << false << "entrada3" << false << "entrada4" << false <<
									"entrada5" << false << "entrada6" << false << "entrada7" << false)
								 )
							);

	dataJSON.appendElements(BSON("saidas" <<
								BSON("saida0" << false << "saida1" << false << "saida2" << false << "saida3" << false <<
									 "saida4" << false << "saida5" << false << "saida6" << false << "saida7" << false)
								 )
							);

	dataJSON << "odometro_adicionado" << isOdometerIncreased << "horimetro_adicionado" << isHourmeterIncreased;

	dataJSON << "inicio_rota" << ignition << "fim_rota" << routeEnd;

	if(hasBlock) dataJSON << "Bloqueio" << block;

	dataJSON << "EmRe" << reverse;

	freeDataJSON << "Analogico1" << analogic1 << "Analogico2" << analogic2 << "Analogico3" << analogic3 <<
						"Analogico4" << analogic4 << "Horimetro" << horimeter << "AcelerometroX" << accelerometerX <<
						"Digital1" << digital1 << "Digital2" << digital2 << "Digital3" << digital3 <<
						"Digital4" << digital4 << "AcelerometroY" << accelerometerY << "Hodometro" << hodometer <<
						"Rpm" << rpm << "Freio" << breaks;


	dataJSON.appendElements(BSON("DadoLivre" << freeDataJSON.obj()));

	string typeString;
	switch (type)
	{
		case Bluetec::enumDataType::HSYNS:
			typeString = "HSYNS";
		break;
		case Bluetec::enumDataType::HSYNC:
			typeString = "HSYNC";
		break;
		case Bluetec::enumDataType::FINAL:
			typeString = "A3A5A7";
		break;

		default:
			typeString = "DADOS";
		break;
	}

	dataJSON << "tipo" << typeString;

	if(!rawLapse.empty())
	{
		std::string lapse;
		for (std::vector<unsigned char>::iterator it = rawLapse.begin() ; it != rawLapse.end(); ++it)
		{
			char hexBuffer[4];
			sprintf(hexBuffer, "%02X", (unsigned char)*it);
			lapse = lapse.append(hexBuffer);
		}

		dataJSON << "lapso" << lapse;
	}

	mongo::BSONObj dataJSONObj = dataJSON.obj();

	Log(TAG "%s %s", dataJSONObj.toString().c_str(), pConfiguration->GetMongoDBCollections().at(0).c_str());

	try
	{
		Position pos;

		// Get last position
		GetLastPosition(vehicle, datePosition, &pos);

		double distance = BTCloud::Util::HaverSine(pos.lat, pos.lon, pPosition->lat, pPosition->lon);
		uint64_t timeInterval = pPosition->dateTime - pos.dateTime;

		if(distance > 0 && timeInterval > 0)
		{
			// Convert distance to meters and time to seconds
			double velocity = (distance * 1000) / timeInterval;

			// Verify if the velocity is ok with distance driven
			if(velocity > 200)
				dataJSONObj = dataJSONObj.removeField("coordenates");
		}
		else
			dataJSONObj = dataJSONObj.removeField("coordenates");

		// Insert json data at posicao
		pDBClientConnection->insert(pConfiguration->GetMongoDBCollections().at(0), dataJSONObj);

		// Insert/Update data at ultima_posicao
		if(pos.dateTime == 0)
		{
			Dbg(TAG "Inserting position at mongodb collection ultima_posicao");
			pDBClientConnection->insert(pConfiguration->GetMongoDBCollections().at(1), dataJSONObj);
		}
		else if(pos.dateTime < datePosition)
		{
			Dbg(TAG "Updating position at mongodb collection ultima_posicao");
			UpdateLastPosition(vehicle, datePosition, dateArrival);
		}
		else
			Dbg(TAG "Skipping position at mongodb collection ultima_posicao");
	}
	catch (mongo::DBException &e)
	{
		// duplicate key error
		string e = pDBClientConnection->getLastError();
		Error(TAG "Error on insert/update data at mongo: %s", e.c_str());
	}
	catch(std::exception &e)
	{
		Error(TAG "Error on insert/update data at mongo: %s", e.what());
	}
}

void Protocol::GetLastPosition(uint32_t vehicleId, uint64_t lastPositionDate, Position *pos)
{
	// Use a cached var to evade query execute
	if(iLastPositionDate == lastPositionDate && iLastPositionVehicle == vehicleId)
		pos->dateTime = iLastPositionDate;
	else
	{
		mongo::Query query = MONGO_QUERY("veiculo" << vehicleId);

		Dbg(TAG "Has last position %s: %s", pConfiguration->GetMongoDBCollections().at(1).c_str(), query.toString().c_str());

		auto_ptr<mongo::DBClientCursor> cursor = pDBClientConnection->query(pConfiguration->GetMongoDBCollections().at(1), query);

		// Verify if it has a register at collection
		if(cursor->more() > 0)
		{
			mongo::BSONObj query_res = cursor->next();
			mongo::Date_t positionDate = query_res.getField("data_posicao").date();
			double positionLon = query_res.getFieldDotted("coordenadas.coordinates[0]").number();
			double positionLat = query_res.getFieldDotted("coordenadas.coordinates[1]").number();

			// Set a cache value to not execute the query all the time
			iLastPositionDate = positionDate.millis;
			iLastPositionVehicle = vehicleId;

			pos->dateTime = positionDate.millis;
			pos->lat = positionLat;
			pos->lon = positionLon;
		}
		else
		{
			pos->dateTime = iLastPositionDate;
			pos->lat = 0.0;
			pos->lon = 0.0;
		}
	}
}

void Protocol::UpdateLastPosition(int vehicleId, u_int64_t datePosition, u_int64_t dateArrival)
{
	mongo::Query query = MONGO_QUERY("veiculo" << vehicleId);

	double lat2 = pPosition->lat;
	double long2 = pPosition->lon;
	mongo::BSONObj querySet;

	// Verify if has a valid position
	if(abs(long2) != 0 && abs(lat2) != 0)
	{
		// Create update query
		querySet = BSON("$set" << BSON(
									"data_posicao" << mongo::Date_t(datePosition) <<
									"data_chegada" << mongo::Date_t(dateArrival) <<
									"motorista_ibutton" << pPosition->inf_motorista.id <<
									"velocidade" << pTelemetry->velocity <<
									"coordenadas" << BSON("Type" << "Point" << "coordinates" << BSON_ARRAY(long2 << lat2))
									)
					);
	}
	else
	{
		// Create update query
		querySet = BSON("$set" << BSON(
									"data_posicao" << mongo::Date_t(datePosition) <<
									"data_chegada" << mongo::Date_t(dateArrival) <<
									"motorista_ibutton" << pPosition->inf_motorista.id <<
									"velocidade" << pTelemetry->velocity
								)
					);
	}

	Dbg(TAG "Update querySet %s: %s", pConfiguration->GetMongoDBCollections().at(1).c_str(), querySet.toString().c_str());

	// Updating based on vehicle id with multiple parameter
	pDBClientConnection->update(pConfiguration->GetMongoDBCollections().at(1), query, querySet, false, true);
}

void Protocol::ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo, bool isFinalRoute, bool isStartRoute)
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
	lapso.lastIncompleteLapse = "";

	Dbg(TAG "Processing data %d %d...", ponteiroIni, ponteiroFim);
	Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)dados.at(0)), hex, setw(2), setfill('0'), int((unsigned char)dados.at(dados.length()-1)));

	if(!cFileManager.GetHfull(dataCache.veioId, hfull))
	{
		hfull.lapso = Bluetec::enumDefaultValues::LAPSO;
		hfull.acely = Bluetec::enumDefaultValues::ACELY;
		hfull.acelx = Bluetec::enumDefaultValues::ACELX;
		hfull.spanAcel = Bluetec::enumDefaultValues::SPANACEL;
		hfull.reversao = Bluetec::enumDefaultValues::REVERSE;
		hfull.configHardware = Bluetec::enumDefaultValues::HIGH_PRECISION_GPS;
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
				//tipoDado = isStartRoute ? Bluetec::enumDataType::HSYNS : tipoDado;

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

		// Persist last incomplete lapse
		if(lapso.lastIncompleteLapse.length() > 0)
		{
			//ParseLapse(lapso, dados, hfull, index, tipoDado, isStartRoute);
			Dbg(TAG "Process missing lapse %s", lapso.lastIncompleteLapse.c_str());
			dados = lapso.lastIncompleteLapse + dados;
		}

		ParseLapse(lapso, dados, hfull, index, tipoDado, isStartRoute);

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

void Protocol::ParseLapse(BTCloud::Util::Lapse &lapso, string dados, Bluetec::HFull hfull, int index, uint8_t dataType, bool isStartRoute)
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
	int fim = dados.length() - 1;

	// When a route is known, so it exists and it can be processed
	Dbg(TAG "Known route, processing...");
	bool isHourmeterIncreased = false;
	bool isOdometerIncreased = false;

	Bluetec::enumDataType type;

	if(dataType == Bluetec::enumDataType::HSYNS || (dataType == Bluetec::enumDataType::HSYNS_FINAL && isStartRoute) || (dataType == Bluetec::enumDataType::HSYNS_DADOS && isStartRoute))
		type = Bluetec::enumDataType::HSYNS;
	else
		type = Bluetec::enumDataType::DADOS;

	while(index < fim)
	{
		Dbg(TAG "%d < %d", index, fim);
		try
		{
			// Calc control byte size
			bufferControle[0] = dados.at(index);
			controle = (BTCloud::Util::Output*) bufferControle;
			tamLapso = BTCloud::Util::LapseSize(controle);

			// Validate route lapse at this point
			if((index + tamLapso) <= fim)
			{
				std::vector<unsigned char> rawLapse;
				rawLapse.push_back(dados.at(index));

				// Case the expasion bit is enable, calc the expasion byte
				if(controle->saida0 && index + 1 <= fim)
				{
					// The expasion byte comes after control byte, so increment it to calculate correctly next data
					index++;

					bufferExpansao[0] = dados.length() == index ? dados.at(index - 1) : dados.at(index);
					expansao = (BTCloud::Util::Output*) bufferExpansao;
					tamLapso += (BTCloud::Util::ExpasionSize(expansao) + 1);
					rawLapse.push_back(bufferExpansao[0]);
				}

				// Speed
				if(controle->saida5)
				{
					index++;
					lapso.velocidade = ((double) ((unsigned char) dados.at(index)));
					rawLapse.push_back((unsigned char)dados.at(index));
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
					rawLapse.push_back((unsigned char)dados.at(index));
				}
				// Acel x
				if(controle->saida3)
				{
					index++;
					lapso.acelx = 0;
					rawLapse.push_back((unsigned char)dados.at(index));
				}
				// Acel y
				if(controle->saida2)
				{
					index++;
					lapso.acely = 0;
					rawLapse.push_back((unsigned char)dados.at(index));
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
					rawLapse.push_back(bufferED[0]);
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
						rawLapse.push_back((unsigned char)dados.at(index-1));
						rawLapse.push_back((unsigned char)dados.at(index));
					}
					// An 2
					if(expansao->saida6)
					{
						Dbg(TAG "An 2");
						lapso.an2 = 0;
						index++;
						rawLapse.push_back((unsigned char)dados.at(index));
					}
					// An 3
					if(expansao->saida5)
					{
						Dbg(TAG "An 3");
						lapso.an3 = 0;
						index++;
						rawLapse.push_back((unsigned char)dados.at(index));
					}
					// An 4
					if(expansao->saida4)
					{
						Dbg(TAG "An 4");
						lapso.an4 = 0;
						index++;
						rawLapse.push_back((unsigned char)dados.at(index));
					}
					// Verify if there is a operation information
					if(expansao->saida3)
					{
						operacao = dados.substr(index + 1, 7);
						lapso.operacao = operacao;
						unsigned char controleOper = operacao.at(0);
						index += 7;

						const char *values = operacao.c_str();
						const char *end = values + strlen(values);
						rawLapse.insert(rawLapse.end(), values, end);

						// Verify if is a GPS and is a fixed GPS
						if(controleOper >= 0xA0 && !((controleOper >> 3) & 0x01))
						{
							Dbg(TAG "GPS");

							char buffer[255];
							buffer[0] = controleOper;
							BTCloud::Util::Output *p;
							p = (BTCloud::Util::Output*) buffer;
							double lat = BTCloud::Util::ParseLatitude(hfull.configHardware, operacao.substr(1, 3), p->saida2);
							double lon = BTCloud::Util::ParseLongitude(hfull.configHardware, operacao.substr(4, 3), p->saida1, p->saida0);

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

				switch (hfull.reversao)
				{
					case 0x08: pEventFlag->reverse = (unsigned int) lapso.ed1; break;
					case 0x04: pEventFlag->reverse = (unsigned int) lapso.ed2; break;
					case 0x02: pEventFlag->reverse = (unsigned int) lapso.ed3; break;
					case 0x01: pEventFlag->reverse = (unsigned int) lapso.ed4; break;
					default: pEventFlag->reverse = false; break;
				}

				pOdoVel->velocity = lapso.velocidade;

				Dbg(TAG "Timestamp before transmition %d", lapso.timestamp);
				Dbg(TAG "Position found, sending package with %d lapses of %d bytes", sizePacote / iLapsoSize, iLapsoSize);

				// Save JSON at MongoDB
				CreatePosition(isOdometerIncreased, isHourmeterIncreased, false, true, false, type, rawLapse);

				// Reset entity
				cPackage.Clear();

				// Reset maximum size of packet
				sizePacote = 0;

				lapso.timestamp += hfull.lapso;
				type = Bluetec::enumDataType::DADOS;

				// Case reverse exists, change current status of reverse
				if(pEventFlag->reverse) pEventFlag->reverse = !pEventFlag->reverse;

				lapso.lastIncompleteLapse = "";
			}
			else
			{
				// Position start to incomplete lapse
				lapso.lastIncompleteLapse = dados.substr(index, index - dados.length());
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

		bool isStartRoute = false;

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
						ParseData(sbt4.substr(inicio, i - inicio - lHsync), ponteiroIni + inicio, ponteiroIni + i-lHsync - 1, arquivo, false, false);
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
					isStartRoute = true;

					Dbg(TAG "Found the HSYNS in %d", i);
					if(inicio < i - lHsyns)
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, lHsyns - 1);

						// Parse data
						Dbg(TAG "ParseData HSYNS %d", ponteiroIni + inicio);
						ParseData(sbt4.substr(inicio, i - inicio - lHsyns), ponteiroIni + inicio, ponteiroIni + i-lHsyns - 1, arquivo, false, isStartRoute);
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
						ParseData(sbt4.substr(inicio, i - inicio - lHfull), ponteiroIni + inicio, ponteiroIni + i-lHfull - 1, arquivo, false, isStartRoute);
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
					ParseData(sbt4.substr(inicio, i - inicio), ponteiroIni + inicio, ponteiroIni + i-1, arquivo, true, isStartRoute);
					inicio = i;
				}
				if( inicio == i && i + lFimTrecho <= sbt4.length())
				{
					Dbg(TAG "Closing the route of final point %d", i - 1);

					// Parse data
					ParseA3A5A7(sbt4.substr(inicio, lFimTrecho + 1));

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
			ParseData(sbt4.substr(inicio, sbt4.length()-inicio), ponteiroIni + inicio, ponteiroFim, arquivo, false, isStartRoute);
		}

		fclose(in);

		// Rename file to set as processed
		string newPath = "";
		Util::CreateFileNameProcessed(&newPath, tokens);
		cFileManager.RenameFile(path, newPath);
	}
}

}
