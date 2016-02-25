
#include "Protocol.hpp"
#include "util/ProtocolUtil.hpp"
#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include <util/Log.hpp>
#include "clock.h"
#include "mongo/client/dbclient.h" // for the driver
#include "Configuration.hpp"
#include <api/mysql/MySQLConnector.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#define TAG "[Protocol] "

using namespace boost::algorithm;

namespace Sascar
{

std::string serializado;
pacote_posicao::equip_flags *eventoflag;
pacote_posicao::equip_posicao *posicao;
static int lapsoSize = sizeof(long int) + sizeof(double) * 6 + sizeof(int) * 13 + 16;
pacote_posicao::t_telemetria_bluetec400 *telemetria;
pacote_posicao::pacote_enriquecido *pacote;
pacote_posicao::bluetec400 bluetecPacote;
cache_cadastro cad;
pacote_posicao::t32_odo_vel *odo_vel_gps;



void lapsoToTelemetria(pacote_posicao::t_telemetria_bluetec400 *tele, struct Sascar::ProtocolUtil::sLapso& lapso)
{
	tele->set_trecho( lapso.idTrecho );
	tele->set_datahora( lapso.timestamp );
	tele->set_velocidade( lapso.velocidade );
	tele->set_rpm( lapso.rpm );
	tele->set_odometro( lapso.odometro );
	tele->set_horimetro( lapso.horimetro );
	tele->set_ed1( lapso.ed1 );
	tele->set_ed2( lapso.ed2 );
	tele->set_ed3( lapso.ed3 );
	tele->set_ed4( lapso.ed4 );
	tele->set_ed5( lapso.ed5 );
	tele->set_ed6( lapso.ed6 );
	tele->set_ed7( lapso.ed7 );
	tele->set_ed8( lapso.ed8 );
	tele->set_acelx( lapso.acelx );
	tele->set_acely( lapso.acely );
	tele->set_an1( lapso.an1 );
	tele->set_an2( lapso.an2 );
	tele->set_an3( lapso.an3 );
	tele->set_an4( lapso.an4 );
	tele->set_operacao( lapso.operacao );
}

void lapsoSetup(string lapso, struct Sascar::ProtocolUtil::sLapso& setup)
{
	if( lapso.length() == 0 )
		return;
	size_t index = 0;
	//cout << "1 ... " << dec <<lapso.find("##", index) << endl;
	//cout << "'"<<lapso.substr(index, lapso.find("##", index)) << "'"<< endl;
	//cout << "'"<<lapso.substr(index, lapso.find("##", index)).length() << "'"<< endl;
	setup.velocidade = stoi( lapso.substr( index, lapso.find( "##", index ) ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "2 ... ";
	setup.rpm = stoi( lapso.substr( index, lapso.find( "##", index ) ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "3 ... ";
	setup.acelx = stoi( lapso.substr( index, lapso.find( "##", index ) ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "4 ... ";
	setup.acely = stoi( lapso.substr( index, lapso.find( "##", index ) ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "5 ... ";
	setup.ed1 = (int ) ( lapso.at( index ) );
	setup.ed2 = (int ) ( lapso.at( index++ ) );
	setup.ed3 = (int ) ( lapso.at( index++ ) );
	setup.ed4 = (int ) ( lapso.at( index++ ) );
	setup.ed5 = (int ) ( lapso.at( index++ ) );
	setup.ed6 = (int ) ( lapso.at( index++ ) );
	setup.ed7 = (int ) ( lapso.at( index++ ) );
	setup.ed8 = (int ) ( lapso.at( index++ ) );
	//	cout << "6 ... ";
	setup.an1 = stoi( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "7 ... ";
	setup.an2 = stoi( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "8 ... ";
	setup.an3 = stoi( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "9 ... ";
	setup.an4 = stoi( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "10 ... ";
	setup.odometro = stol( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "11 ... ";
	setup.horimetro = stol( lapso.substr( index, lapso.find( "##", index ) - 1 ) );
	index = lapso.find( "##", index ) + 2;
	//cout << "12 ... ";
	setup.ibtMotorista = lapso.substr( index, lapso.find( "##", index ) - 1 );
	index = lapso.find( "##", index ) + 2;
	//cout << " OK! " << endl;
}

uint32_t getVeioid()
{
	return cad.veioid;
}

void Protocol::ParseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo)
{
	Dbg(TAG "Init parse HFULL");

	bluetec::sHfull hfull;
	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize;

	char buffer[255];
	buffer[0] = (unsigned char) strHfull.at(0);
	Sascar::ProtocolUtil::saidas *p;
	p = (Sascar::ProtocolUtil::saidas*) buffer;

	hfull.lapso = 0;
	hfull.lapso += p->saida0 * 1;
	hfull.lapso += p->saida1 * 2;
	hfull.lapso += p->saida2 * 4;
	hfull.lapso += p->saida3 * 8;
	hfull.lapso += p->saida4 * 16;
	//cout << " lapso: " << dec << (int)hfull.lapso << " segundos"<< endl;

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
	//hfull.confHardwareRe; //Configuracoes de Hardware (Redundancia)
	//hfull.limiteAnalogico1;
	//hfull.limiteAnalogico234[3];
	//hfull.lapsosGravacaoEvento; //Lapsos de gravacao do evento
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

	cFileManager.saveHfull( getVeioid(), hfull );

	Dbg(TAG "Fixing file cursor before and after HFULL");

	// Verify case something matches at the end of this HFULL
	if(cFileManager.getBufferFile( getVeioid(), ponteiroFim + 1, arquivo, tBuffer, tSize, header) &&
			header.beginPointer == ponteiroFim + 1 &&
			(header.dataType == bluetec::enumDataType::DADOS ||
			header.dataType == bluetec::enumDataType::DADOS_FINAL ||
			header.dataType == bluetec::enumDataType::FINAL))
	{
		Dbg(TAG "HFULL + TEMP");

		// Remove this to overwrite it
		cFileManager.delFile(getVeioid(), header.headerFile);

		// Update the start cursor
		header.beginPointer -= 512;

		// Save changes
		Dbg(TAG "Saving buffer file header data type %d", /*dec, */ header.dataType);
		cFileManager.saveBufferFile(getVeioid(), tBuffer, tSize, header);
	}

	// Verify case something matches at the start of this HFULL
	if(cFileManager.getBufferFile(getVeioid(), ponteiroIni - 1, arquivo, tBuffer, tSize, header) &&
			header.endPointer == ponteiroIni - 1 &&
			(header.dataType == bluetec::enumDataType::DADOS ||
			header.dataType == bluetec::enumDataType::HSYNS ||
			header.dataType == bluetec::enumDataType::HSYNS_DADOS))
	{
		Dbg(TAG "TEMP + HFULL");

		// Remove this to overwrite it
		cFileManager.delFile(getVeioid(), header.headerFile);

		// Update the end cursor
		header.endPointer += 512;

		// Save changes
		Dbg(TAG "Saving buffer file header data type %d", /*dec, */ header.dataType);
		cFileManager.saveBufferFile(getVeioid(), tBuffer, tSize, header);
	}
}

void parseHSYNC(string hsync)
{

}

void Protocol::ParseA3A5A7(unsigned int ponteiroIni, unsigned int arquivo)
{
	// Search for route persisted with start cursor = 1 and remove it
	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize=0;

	Dbg(TAG "Searching the start point in this route");

	// Case something matches with the start of this route
	if(cFileManager.getBufferFile(getVeioid(), ponteiroIni - 1, arquivo, tBuffer, tSize, header) &&
			header.endPointer == ponteiroIni - 1 && header.file == arquivo)
	{
		Dbg(TAG "This route has a start point");

		switch(header.dataType)
		{
			case bluetec::enumDataType::HSYNS_DADOS:
				Dbg(TAG "Route completed, removing from temp memory");
				cFileManager.delFile( getVeioid(), header.headerFile );
			break;

			case bluetec::enumDataType::DADOS:
				Dbg(TAG "Route incomplete, increasing end route point");

				// Remove this temp file to overwrite it
				cFileManager.delFile(getVeioid(), header.headerFile);

				// Update the data type
				header.dataType = bluetec::enumDataType::DADOS_FINAL;

				// Save it
				cFileManager.saveBufferFile(getVeioid(), tBuffer, tSize, header);
			break;
		}
	}
	else
	{
		// Case this does not need to be saved temporarily
		header.file = arquivo;
		header.beginPointer = ponteiroIni;
		header.dataType = bluetec::enumDataType::FINAL;

		Dbg(TAG "Save buffer file header data type");
		cFileManager.saveBufferFile(getVeioid(), tBuffer, (uint32_t)0, header);
	}
}

void Protocol::ParseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim)
{
	Dbg(TAG "Init HSYNS %d", /*dec,*/ ponteiroFim);

	struct Sascar::ProtocolUtil::sLapso lapso;
	bluetec::sBluetecHeaderFile header;
	string pLapso;

	lapso.idTrecho = cFileManager.getNextIdTrecho();
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

	lapso.timestamp = mktime(Sascar::ProtocolUtil::ParseDataHora(hsyns.substr(8, 7)));

	// Load driver ibutton in bcd
	lapso.ibtMotorista = "";
	lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(1));

	// Standardizes the operation code to empty 81
	if(lapso.ibtMotorista == "81")
	{
		// Default ibutton
		lapso.ibtMotorista = "0";
	}
	else
	{
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(2));
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(3));
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(4));
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(5));
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(6));
		lapso.ibtMotorista += Sascar::ProtocolUtil::ParseBCDDecimal(hsyns.at(7));
	}

	Dbg(TAG "Parse timestamp Hex 0x%x", /*dec, */ hsyns.substr(8, 7).c_str());
	Dbg(TAG "Parse timestamp %d", /*dec, */ lapso.timestamp);
	lapso.odometro = Sascar::ProtocolUtil::ParseOdometro(hsyns.substr(15, 3));

	Dbg(TAG "Parse odometer %d", /*dec, */ lapso.odometro);
	Dbg(TAG "%d %d %d %d - %d %d %d %d", /*dec, */ hex, setw(2), setfill('0'), int((unsigned char)hsyns.at(0)), hex, setw(2), setfill('0'), int((unsigned char)hsyns.at(hsyns.length() - 1)));
	lapso.horimetro = Sascar::ProtocolUtil::ParseHorimetro(hsyns.substr(18, 3));

	Dbg(TAG "Parse horimeter %d", /*dec, */ lapso.horimetro);

	pLapso = Sascar::ProtocolUtil::PersistableLapso(&lapso);

	header.file = arquivo;
	header.endPointer = ponteiroFim;
	header.timestamp = lapso.timestamp;
	header.dataType = bluetec::enumDataType::HSYNS;

	Dbg(TAG "IMC 1 - header.dataType = %d", header.dataType);
	Dbg(TAG "Saving HSYNS %d .. ", /*dec,*/ header.endPointer);

	// Save route start
	Dbg(TAG "Save buffer file Header data type %d", header.dataType);
	Dbg(TAG "Save buffer file Enum data type %d", /*dec,*/ bluetec::enumDataType::HSYNS);

	Dbg(TAG "IMC 2 - header.dataType = %d", header.dataType);
	cFileManager.saveBufferFile( getVeioid(), pLapso.c_str(), pLapso.length(), header );
}

void Protocol::PersistJSONData(pacote_posicao::bluetec400 data)
{
	int idEquipment = data.pe().ec().id();
	int vehicle = data.pe().ec().veioid();
	std::string plate = "MUR0001";
	std::string client = "BT_FROTA";

	u_int64_t datePosition = data.pe().ep().datahora(); // "2015-11-05T08:40:44.000Z"
	datePosition *= 1000;
	u_int64_t dateArrival = data.pe().ep().datachegada(); //"2015-11-05T08:40:44.000Z";
	dateArrival *= 1000;
	std::string address = "";
	std::string neighborhood = "";
	std::string city = "";
	std::string province = "";
	int velocity = data.tb(0).velocidade();
	double lat2 = data.pe().ep().lat2();
	double long2 = data.pe().ep().long2();
	std::string number = "";
	std::string country = "";
	std::string velocityStreet = "";
	bool gps = true;

	int32_t analogic1 = data.tb_size() > 0 ? data.tb(0).an1() : 0;
	int32_t analogic2 = data.tb_size() > 0 ? data.tb(0).an2() : 0;
	int32_t analogic3 = data.tb_size() > 0 ? data.tb(0).an3() : 0;
	int32_t analogic4 = data.tb_size() > 0 ? data.tb(0).an4() : 0;
	bool digital1 = data.tb(0).ed7();
	bool digital2 = data.tb(0).ed6();
	bool digital3 = data.tb(0).ed5();
	bool digital4 = data.tb(0).ed4();
	int32_t horimeter = data.tb_size() > 0 ? data.tb(0).horimetro() : 0;
	double_t accelerometerX = data.tb_size() > 0 ? data.tb(0).acelx() : .0;
	double_t accelerometerY = data.tb_size() > 0 ? data.tb(0).acely() : .0;
	double_t hodometer = data.tb_size() > 0 ? data.tb(0).odometro() : .0;
	int32_t rpm = data.tb_size() > 0 ? data.tb(0).rpm() : 0;
	bool ignition = data.pe().ep().eventoflag().ignicao();
	bool breaks = data.tb(0).ed8();

	mongo::BSONObj dataPosJSON = BSON(
				"id_equipamento" << idEquipment << "veiculo" << vehicle << "placa" << plate << "cliente" << client <<
				"data_posicao" << mongo::Date_t(datePosition) << "data_chegada" << mongo::Date_t(dateArrival) <<
				"velocidade" << velocity <<	"endereco" << address << "bairro" << neighborhood << "municipio" << city
				<< "estado" << province <<
				"coordenadas" << BSON("Type" << "Point" << "coordinates" << BSON_ARRAY(long2 << lat2)) <<
				"numero" << number << "pais" << country << "velocidade_via" << velocityStreet << "gps" << gps <<
				"DadoLivre" << BSON(
					"Analogico1" << analogic1 << "Analogico2" << analogic2 << "Analogico3" << analogic3 <<
					"Analogico4" << analogic4 << "Horimetro" << horimeter << "AcelerometroX" << accelerometerX <<
					"Digital1" << digital1 << "Digital2" << digital2 << "Digital3" << digital3 <<
					"Digital4" << digital4 << "AcelerometroY" << accelerometerY << "Hodometro" << hodometer <<
					"Ignicao" << ignition << "Rpm" << rpm << "Freio" << breaks
					)
				);

	Log(TAG "%s", dataPosJSON.toString().c_str());

	pDBClientConnection->insert(pConfiguration->GetMongoDBCollection(), dataPosJSON);
}

void Protocol::ParseData(string dados, int ponteiroIni, int ponteiroFim, int arquivo)
{
	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize=0;
	bluetec::sHfull hfull;
	struct Sascar::ProtocolUtil::sLapso lapso;
	int tipoDado = 0;
	int index = 0;
	int fim = 0;
	char bufferControle[255];
	Sascar::ProtocolUtil::saidas *controle;
	char bufferExpansao[255];
	Sascar::ProtocolUtil::saidas *expansao;
	string operacao;
	int tamLapso = 0;
	char bufferED[255];
	Sascar::ProtocolUtil::saidas *ed;

	int maxSizePacote = 1500;
	int sizePacote = 0;

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
	cout << "DEBUG -- PROCESSANDO DADOS "<< dec << ponteiroIni << " " << dec << ponteiroFim << " ..." << endl;
	cout << hex << setw(2) << setfill('0') << int((unsigned char)dados.at(0))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)dados.at(dados.length()-1)) << endl;
	if( !cFileManager.getHfull( getVeioid(), hfull ) )
	{
		hfull.lapso = bluetec::enumDefaultValues::LAPSO;
		hfull.acely = bluetec::enumDefaultValues::ACELY;
		hfull.acelx = bluetec::enumDefaultValues::ACELX;
		hfull.spanAcel = bluetec::enumDefaultValues::SPANACEL;
	}

	cout << "DEBUG -- PROCURANDO ALGO ANTES DESSE ARQUIVO..."<< endl;
	// se existe algo que encaixe no comeco...
	if( cFileManager.getBufferFile( getVeioid(), ponteiroIni-1, arquivo, tBuffer, tSize, header ) &&
			header.endPointer == ponteiroIni-1 &&
			header.file == arquivo){

		cout << "DEBUG -- ENCONTRADO COM TIPO "<< dec << header.dataType <<" ..."<< endl;
		printf("IMC 3 - header.dataType = %d\n",header.dataType);
		cout << "DEBUG -- ENCONTRADO COM PONTEIRO FIM "<< dec << header.endPointer <<"..."<< endl;
		// TEMP + DADOS
		ponteiroIni = header.beginPointer;
		lapso.timestamp = header.timestamp;
		switch (header.dataType){
		case bluetec::enumDataType::HSYNS_DADOS:
		case bluetec::enumDataType::HSYNS:
			//nesse caso o trecho ja foi iniciado entao o primeiro lapso deve ser restaurado
			lapsoSetup( tBuffer, lapso );
			tipoDado = bluetec::enumDataType::HSYNS_DADOS;
			//apago o lapso temporario...
			cFileManager.delFile( getVeioid(), header.headerFile );
			break;
		case bluetec::enumDataType::DADOS:
			//nesse caso o pedaco de dados encontrado nao significa nada entao sera apenas atualizado...
			dados = string(tBuffer) + dados;
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			cFileManager.delFile( getVeioid(), header.headerFile );
			tipoDado = bluetec::enumDataType::DADOS;
			break;
		}
	}
	cout << "DEBUG -- PROCURANDO ALGO DEPOIS DESSE ARQUIVO..."<< endl;
	// se existe algo que encaixe no final...
	if( cFileManager.getBufferFile( getVeioid(), ponteiroFim+1, arquivo, tBuffer, tSize, header ) &&
			header.beginPointer == ponteiroFim+1 &&
			header.file == arquivo){

		// DADOS + TEMP
		ponteiroFim = header.endPointer;

		cout << "DEBUG -- ENCONTRADO COM TIPO "<< dec << header.dataType <<"..."<< endl;
		cout << "DEBUG -- ENCONTRADO COM PONTEIRO FIM "<< dec << header.endPointer <<"..."<< endl;
		lapso.timestamp = header.timestamp;
		switch (header.dataType){
		case bluetec::enumDataType::DADOS:
			//nesse caso o pedaco de dados encontrado nao significa nada entao sera apenas atualizado...
			dados = dados + string(tBuffer);
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			cFileManager.delFile( getVeioid(), header.headerFile );
			tipoDado = bluetec::enumDataType::DADOS;
			break;
		case bluetec::enumDataType::FINAL:
		case bluetec::enumDataType::DADOS_FINAL:
			if(header.dataType == bluetec::enumDataType::DADOS_FINAL){
				dados = dados + string(tBuffer);
			}
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			cFileManager.delFile( getVeioid(), header.headerFile );
			// ... se tem trecho aberto...
			if(tipoDado != bluetec::enumDataType::DADOS){
				tipoDado = bluetec::enumDataType::HSYNS_FINAL;
			}else{
				tipoDado = bluetec::enumDataType::DADOS_FINAL;
			}
			break;
		}

	}

	cout << "DEBUG -- VALIDANDO TRECHO TIPODADO "<<dec << tipoDado << " ..."<< endl;
	if(tipoDado == bluetec::enumDataType::DADOS_FINAL ||
			tipoDado == bluetec::enumDataType::DADOS){
		cout << "DEBUG -- TRECHO AINDA DESCONHECIDO..."<< endl;
		header.beginPointer = ponteiroIni;
		header.endPointer = ponteiroFim;
		header.file = arquivo;
		header.dataType = tipoDado;
		cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
		cFileManager.saveBufferFile( getVeioid(), dados.c_str(), dados.length(), header );
	}else if(tipoDado){
		//se tipoDado nao e bem trecho nao iniciado (DADOS E DADOS_FINAL) entao se ele existir,
		//significa que tenho um trecho iniciado, entao processo os lapsos
		cout << "DEBUG -- TRECHO CONHECIDO, INCIANDO PROCESSAMENTO..."<< endl;
		fim = dados.length();
		while(index < fim )
		{

			try
			{
				//cout << "       " << index << " 	" << fim << " tipo_dado " << dec << tipo_dado<< endl;

				//calculo o tamanho do byte de controle
				//cout << "tamanho byte @"<<dec<<index<< " length " << dec << hsyns.length() << " fim " << dec << fim << endl;
				bufferControle[0] = dados.at(index);
				controle = (Sascar::ProtocolUtil::saidas*) bufferControle;

				tamLapso = Sascar::ProtocolUtil::TamanhoLapso(controle);
				//e se o bit de expansao estiver ativo, calculo tambem o byte de expansao
				if(controle->saida0 && index + 1 <= fim)
				{
					index++; //o byte de expansao vem logo apos o byte de controle, entao ja incremento para calcular corretamente os proximos valores
					//cout << "tamanho exp @"<<dec<<index<< endl;
					bufferExpansao[0] = dados.at(index);
					expansao = (Sascar::ProtocolUtil::saidas*) bufferExpansao;
					tamLapso += (Sascar::ProtocolUtil::TamanhoLapsoExpansao(expansao) + 1);
				}

				//cout << ">> tamanho: " << dec << tamLapso << endl;
				//cout << ">>   index: " << dec << index << endl;
				//cout << ">>     fim: " << dec << fim << endl;
				//verifico se esse lapso esta completo nesse pedaco de trecho
				if( index + tamLapso <= fim )
				{
					//velocidade
					if( controle->saida5 )
					{
						//printf("------ velocidade -------");
						index++;
						//cout << "velocidade @"<<dec<<index<< endl;
						lapso.velocidade = ( (double ) ( (unsigned char ) dados.at( index ) ) );
						//cout << dec << velocidade << endl;
					}
					//rpm
					if( controle->saida4 )
					{
						//cout << "------ rpm ------ ";
						index++;
						//cout << "rpm @"<<dec<<index<< " "<< hex << setw(2) << setfill('0') << int((unsigned char)hsyns.at(index)) << endl;
						lapso.rpm = ( (int ) ( (unsigned char ) dados.at( index ) ) ) * 50;
						//cout << dec << lapso.rpm<< endl;
					}
					//acel x
					if( controle->saida3 )
					{
						//cout << "------ acelx ------ ";
						index++;
						//cout << "acelx @"<<dec<<index<<endl;
						lapso.acelx = 0;
					}
					//acel y
					if( controle->saida2 )
					{
						//cout << "------ acely ------ ";
						index++;
						//cout << dec <<((unsigned int)hsyns.at(index)) << endl;
						//cout << "acely @"<<dec<<index<< " "<< hex << setw(2) << setfill('0') << int((unsigned char)hsyns.at(index)) << endl;
						lapso.acely = 0;
					}
					//ed
					if(controle->saida1)
					{
						//cout << "------ ed ------ ";
						index++;
						//cout << "ed @"<<dec<<index<< endl;
						bufferED[0] = dados.at(index);
						ed = (Sascar::ProtocolUtil::saidas*) bufferED;

						lapso.ed1 = (unsigned int) ed->saida0;
						lapso.ed2 = (unsigned int) ed->saida1;
						lapso.ed3 = (unsigned int) ed->saida2;
						lapso.ed4 = (unsigned int) ed->saida3;
						lapso.ed5 = (unsigned int) ed->saida4;
						lapso.ed6 = (unsigned int) ed->saida5;
						lapso.ed7 = (unsigned int) ed->saida6;
						lapso.ed8 = (unsigned int) ed->saida7;
						//cout << ((unsigned int)hsyns.at(index)) << endl;
					}
					//verifico se ha byte de expansao novamente, dessa vez para processa-lo
					if( controle->saida0 )
					{
						//cout << "------ expansao ------" << endl;
						//TODO: CALCULAR AS INFORMACOES DO BYTE DE EXPANSAO
						//an 1
						if( expansao->saida7 )
						{
							//cout << "------ an1 ------" << endl;
							lapso.an1 = 0;
							index += 2;
						}
						//an 2
						if( expansao->saida6 )
						{
							//cout << "------ an2 ------" << endl;
							lapso.an2 = 0;
							index++;
						}
						//an 3
						if( expansao->saida5 )
						{
							//cout << "------ an3 ------" << endl;
							lapso.an3 = 0;
							index++;
						}
						//an 4
						if( expansao->saida4 )
						{
							//cout << "------ an4 ------" << endl;
							lapso.an4 = 0;
							index++;
						}
						//verifico se existe informacao de operacao
						if( expansao->saida3 )
						{
							operacao = dados.substr( index + 1, 7 );
							lapso.operacao = operacao;
							//cout << "controle oper @"<<dec<<0<< endl;
							unsigned char controleOper = operacao.at( 0 );
							//cout << "------ operacao  "<< asctime (gmtime ( &lapso.timestamp )) << " ------ " << hex << setw(2) << setfill('0') << int(controleOper) << " - " << hex << setw(2) << setfill('0') << int((unsigned char)operacao.at(operacao.length()-1)) << endl;
							index += 7;
							//agora preciso ver se essa informacao e de gps
							if( controleOper >= 0xA0 )
							{
								cout << "------ gps ------";

								char buffer[255];
								buffer[0] = controleOper;
								Sascar::ProtocolUtil::saidas *p;
								p = (Sascar::ProtocolUtil::saidas*) buffer;
								double lat = Sascar::ProtocolUtil::ParseLatitude(operacao.substr( 1, 3 ), p->saida2);
								double lon = Sascar::ProtocolUtil::ParseLongitude(operacao.substr( 4, 3 ), p->saida1, p->saida0);

								printf(" lat long -> %20.18f %20.18f \n ", lat, lon);

								// Setando dados do pacote de posicao
								posicao = pacote->mutable_ep();
								posicao->set_lat2( lat );
								posicao->set_long2( lon );
								posicao->set_datahora( lapso.timestamp );
								//	posicao->set_datachegada(lapso.timestamp);
								posicao->set_datachegada(clock::horaatual);
								//	printf(" Dada Chegada -> %s\n", clock::horaatual);

								eventoflag = posicao->mutable_eventoflag();
								if( lapso.rpm > 0 && lapso.velocidade > 0 )
								{
									eventoflag->set_ignicao( 1 );
								}
								else
								{
									eventoflag->set_ignicao( 0 );
								}

								odo_vel_gps = posicao->mutable_odo_vel_gps();
								odo_vel_gps->set_velocidade( lapso.velocidade );
								printf( " timestamp antes de transmitir... %d \n ", lapso.timestamp );

								cout << " posicionamento encontrado, enviando pacote com " << dec << sizePacote / lapsoSize << " lapsos de " << dec << lapsoSize << " bytes" << endl;

								//Envia para a fila o pacote de posicao ja em sem formato final protobuf.
								bluetecPacote.SerializeToString(&serializado);
								PersistJSONData(bluetecPacote);
								bluetecPacote.Clear();
								//bluetecPacote.clear_tb();
								//reinicia o tamanho maximo do pacote pois ja enviou
								sizePacote = 0;
							}

						}
						//incremento odometro
						if( expansao->saida2 )
						{
							//cout << "ODOMETRO INCREMENTADO " << dec << lapso.odometro << " + 100 = ";
							lapso.odometro += 100;
							//cout <<dec<< lapso.odometro<< endl;
						}
						//incremento horimetro
						if( expansao->saida1 )
						{
							lapso.horimetro += 6;
						}
						//vago
						if( expansao->saida0 )
						{

						}

					}

					//popula o protobuf com as variaveis de lapso

					//coloca esse novo protobuf na lista

					index++;
					//cria um novo pacote

					//verifica se esse novo pacote vai estourar o tamamho maximo
					sizePacote += lapsoSize;
					if( sizePacote > maxSizePacote )
					{
						cout << " tamanho maximo atingido, enviando pacote com " << dec << sizePacote / lapsoSize << " lapsos de " << dec << lapsoSize << " bytes" << endl;
						//se estourar ja envia antes
						bluetecPacote.SerializeToString( &serializado );
						PersistJSONData(bluetecPacote);
						bluetecPacote.clear_tb();
						//reinicia o valor do tamanho do pacote com o tamanho do lapso que nao foi enviado
						sizePacote = lapsoSize;
					}
					else
					{
						telemetria = bluetecPacote.add_tb();
						lapsoToTelemetria(telemetria, lapso);
					}

					//cout << " INCREMENTO DE TIMESTAMP " << dec << lapso.timestamp << " += "<< dec << hfull.lapso << " = ";
					lapso.timestamp += hfull.lapso;
					//cout << dec << lapso.timestamp << endl;

				}
				else
				{
					index = fim;
				}

			}
			catch(exception& e )
			{
				cout << "ERRO indice: " << index << endl;
			}

		} //end while

		bluetecPacote.SerializeToString(&serializado);
		PersistJSONData(bluetecPacote);
		bluetecPacote.clear_tb();

		//se nao tiver fim de trecho, precisa retornar o ultimo lapso para
		//persisti-lo e nao haver reprocessamento
		if(tipoDado != bluetec::enumDataType::HSYNS_FINAL)
		{
			header.beginPointer = ponteiroIni;
			header.endPointer = ponteiroFim;
			header.file = arquivo;
			header.idTrecho = lapso.idTrecho;
			string pLapso = Sascar::ProtocolUtil::PersistableLapso( &lapso );
			cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
			cFileManager.saveBufferFile( getVeioid(), pLapso.c_str(), pLapso.length(), header );
		}
	}
}

Protocol::Protocol()
{
}

Protocol::~Protocol()
{

}

void Protocol::GetClientData(cache_cadastro &retorno, std::string chave)
{
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
				retorno.veioid = atoi(mysqlRow[0]);
				retorno.placa = mysqlRow[1];
				retorno.id = atoi(mysqlRow[2]);
				retorno.clientName = mysqlRow[3];

				Dbg(TAG "Vehicle id: %d", retorno.veioid);
				Dbg(TAG "Plate: %s", retorno.placa.c_str());
				Dbg(TAG "Equipment id: %d", retorno.id);
				Dbg(TAG "Client name: %s", retorno.clientName.c_str());

				pMysqlConnector->FreeResult(mysqlResult);
			}
		}
	}

	pMysqlConnector->Disconnect();
}

uint32_t Protocol::CreateClient(std::string clientName)
{
	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	string query("");
	query.append("INSERT INTO clientes SET clinome = '").append(clientName).append("'");
	pMysqlConnector->Execute(query);

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	// return inserted registry id
	return pMysqlConnector->InsertedID();
}

uint32_t Protocol::CreateEquipment(uint32_t projectId, uint32_t equipIMei)
{
	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	string query("");
	query.append("INSERT INTO equipamentos SET equip_id = default")
			.append(", projetos_proj_id = ").append(std::to_string(projectId))
			.append(", equip_imei = ").append(std::to_string(equipIMei))
			.append(", equip_insert_time = CURRENT_TIMESTAMP");
	pMysqlConnector->Execute(query);

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	// return inserted registry id
	return pMysqlConnector->InsertedID();
}

uint32_t Protocol::CreateVehicle(uint32_t clientId, uint32_t equipId, std::string plate)
{
	// Connect to mysql
	pMysqlConnector->Connect(pConfiguration->GetMySQLHost()
							 , pConfiguration->GetMySQLUser()
							 , pConfiguration->GetMySQLPassword()
							 , pConfiguration->GetMySQLScheme());

	string query("");
	query.append("INSERT INTO veiculos SET veioid = default")
			.append(", vei_clioid = ").append(std::to_string(clientId))
			.append(", vei_equoid = ").append(std::to_string(equipId))
			.append(", vei_placa = '").append(plate).append("'");
	pMysqlConnector->Execute(query);

	// Diconnect from mysql
	pMysqlConnector->Disconnect();

	// return inserted registry id
	return pMysqlConnector->InsertedID();
}

void Protocol::FillDataContract(std::string clientName, std::string plate, cache_cadastro &retorno)
{
	retorno.veioid = 0;
	retorno.esn = atoi(plate.c_str());

	// Get client from mysql database
	GetClientData(retorno, plate);

	// Case this plate does not exist, create it
	if(retorno.veioid == 0)
	{
		uint32_t clientId = CreateClient(clientName);
		uint32_t equipId = CreateEquipment(pConfiguration->GetProjectId(), 1);
		uint32_t vehiId = CreateVehicle(clientId, equipId, plate);

		retorno.veioid = vehiId;
		retorno.placa = plate;
		retorno.id = equipId;
		retorno.clientName = clientName;
	}
}

void Protocol::Process(const char *path, int len, mongo::DBClientConnection *dbClient)
{
	cFileManager.setPath(pConfiguration->GetAppListeningPath());

	if(!pDBClientConnection)
		pDBClientConnection = dbClient;

	pacote_posicao::equip_contrato *contrato;
	FILE *in = 0;
	unsigned char bt4[6500];
	std::string sbt4;

	bluetecPacote.Clear();
	pacote = bluetecPacote.mutable_pe();
	contrato = pacote->mutable_ec();

	Dbg(TAG "Processing: %s", path);

	// Tokenize path to get client, plate and file
	std::vector<std::string> tokens;
	split(tokens, path, is_any_of("/"));

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
	if(fileName.length() < 3 || ( fileName.substr( 0, 3 ) != "BT4"))
	{
		Info(TAG "Ignoring %s, it is not in BT4 format", fileName.c_str());
		return;
	}

	// Create or use a contract data
	FillDataContract(clientName, plate.c_str(), cad);

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

		size_t pos = 0;
		int arquivo = (int) sbt4[1];

		Dbg(TAG "File: %d", /*dec,*/ arquivo);

		long ponteiroIni = ((long ) bt4[2] * 65536) + ((long) bt4[3] * 256) + (long ) bt4[4];
		Dbg(TAG "Start point %d * 65536 + %d * 256 + %d = %d", (long) bt4[2], (long) bt4[3], (long) bt4[4], /*dec,*/ ponteiroIni);

		long ponteiroFim = ((long) bt4[lSize - 8] * 65536 ) + ((long) bt4[lSize - 7] * 256 ) + (long) bt4[lSize - 6];
		Dbg(TAG "End point %d", /*dec,*/ ponteiroFim);

		// For each 1200 bytes, the bluetec protocol generate a header with 5 characters at begin and 8 at end
		// this characters must be removed
		sbt4.erase(0, 5);
		sbt4.erase(sbt4.length() - 8, 8);
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
						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - lHsync - 1)));

						inicio = i - lHsync;
					}
					if( inicio == i -  lHsync)
					{
						Dbg(TAG "Creating file of type 2 from %d a %d", inicio, i - 1);
						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - 1)));

						// Parse data
						//parseHSYNC( sbt4.substr( i - 21, 26 ) );

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", /*dec,*/ inicio, /*dec,*/ i);
				}
				else if(sbt4.compare(i,5,"HSYNS") == 0)
				{
					Dbg(TAG "Found the HSYNS in %d", i);
					if( inicio < i - lHsyns )
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, lHsyns - 1);
						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - lHsyns - 1)));

						// Parse data
						ParseData(sbt4.substr(inicio, i - inicio - lHsyns), ponteiroIni + inicio, ponteiroIni + i-lHsyns - 1, arquivo);
						inicio = i - lHsyns;
					}
					if( inicio == i -  lHsyns)
					{
						Dbg(TAG "Creating file of type 3 from %d a %d", inicio, lHfull - 1);
						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - 1)));

						// i is in 'H', count plus 4 pointers ahead, 'H' + 'SYNS'
						Dbg(TAG "Params HSYNS %d %d %d + %d + 4", /*dec,*/ inicio, /*dec,*/ arquivo, /*dec, */ ponteiroIni, i);

						// Parse data
						ParseHSYNS( sbt4.substr( inicio, lHsyns ), arquivo, ponteiroIni +i+4);

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", /*dec,*/ inicio, /*dec,*/ i);
				}
				else if(sbt4.compare(i, 5, "HFULL") == 0)
				{
					Dbg(TAG "Found the HFULL in %d", i);
					if( inicio < i - lHfull )
					{
						Dbg(TAG "Creating file of type 6 from %d a %d", inicio, lHfull - 1);
						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - 1)));

						// Parse data
						ParseData(sbt4.substr(inicio, i - inicio - lHfull), ponteiroIni + inicio, ponteiroIni + i-lHfull - 1, arquivo);
						inicio = i - lHfull;
					}

					if( inicio == i -  lHfull)
					{
						Dbg(TAG "Creating file of type 1 from %d a %d", inicio, i - 1);

						Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
							hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - 1)));

						// Parse data
						ParseHFULL( sbt4.substr( inicio, i-inicio ), ponteiroIni+inicio, i+4, arquivo);

						i += 5;
						inicio = i;
					}
					Dbg(TAG "Start %d i %d", /*dec,*/ inicio, /*dec,*/ i);
				}
			}
			else if(i+4 < sbt4.length() &&
					sbt4.at(i) == (unsigned char) 0x82 &&
					sbt4.compare(i,4,fimTrecho) == 0)
			{
				Dbg(TAG "Found the end of route in %d", i);
				if(inicio < i)
				{
					Dbg(TAG "Creating file of type 6 from %d a %d", inicio, i - 1);

					Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio)),
						hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(i - 1)));

					// Parse data
					ParseData(sbt4.substr(inicio, i - inicio), ponteiroIni + inicio, ponteiroIni + i-1, arquivo);
					inicio = i;
				}
				if( inicio == i && i + lFimTrecho <= sbt4.length())
				{
					Dbg(TAG "Closing the route of final point %d", /*dec,*/ i - 1);

					// Parse data
					ParseA3A5A7(ponteiroIni + i, arquivo);

					i += lFimTrecho + 1;
					inicio = i;
				}
				Dbg(TAG "Start %d i %d", /*dec,*/ inicio, /*dec,*/ i);
			}
		}

		if(inicio < sbt4.length())
		{
			Dbg(TAG "Sending type 6 at the end of buffer %d a %d", /*dec,*/ inicio, sbt4.length() - 1);
			Dbg(TAG "%d %d %d %d a %d %d %d %d", hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(inicio))
				, hex, setw(2), setfill('0'), int((unsigned char)sbt4.at(sbt4.length()-1)));

			// Parse data
			ParseData(sbt4.substr(inicio, sbt4.length()-inicio), ponteiroIni + inicio, ponteiroFim, arquivo);
		}
		fclose(in);
	}

}

int Protocol::Project2Protocol(uint32_t projeto)
{
	switch (projeto)
	{
		case EQUIPAMENTO_BD_MTC550:
			return PROTO_MTC550;
		break;

		/*******
		 * Nao implementei para todos pois pode alterar o funcionamento dos atuais parsers
		 * O ideal eh ir descomentando/adicionando conforme forem feita as implementacoes
		 *
			  case EQUIPAMENTO_BD_MTC:
				return PROTO_SAS401;
				break;

			  case EQUIPAMENTO_BD_VDO:
				return PROTO_VDO;
				break;

			  case EQUIPAMENTO_BD_MTC600:
				return PROTO_MTC600;
				break;

			  case EQUIPAMENTO_BD_MXT150:
				return PROTO_MXT150;
				break;

			  case EQUIPAMENTO_BD_MXT100:
				return PROTO_MXT100;
				break;

			  case EQUIPAMENTO_BD_RVS_QB:
				return PROTO_VDOQB;
				break;

			  case EQUIPAMENTO_BD_MXT140:
				return PROTO_MXT140;
				break;

			   case EQUIPAMENTO_BD_RVSSBTEC:
				return PROTO_VDO_SBTEC;
				break;

			  case EQUIPAMENTO_BD_MXT150SBTEC:
				return PROTO_MXT150_SBTEC;
				break;
				*/
	}

	return projeto;
}

}
