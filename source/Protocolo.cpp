
#include "Protocolo.h"
#include "entities/bluetec400.pb.h"
#include <time.h>
#include <vector> 
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <algorithm>
#include "BlueTecFileManager.h"
#include "clock.h"

#define PROJETO 68

namespace Sascar
{

std::string serializado;
pacote_posicao::equip_flags *eventoflag;
pacote_posicao::equip_posicao *posicao;
static bluetec::BlueTecFileManager fileManager;
static int lapsoSize = sizeof(long int) + sizeof(double) * 6 + sizeof(int) * 13 + 16;
pacote_posicao::pacote_enriquecido *pacote;
pacote_posicao::bluetec400 bluetecPacote;
cache_cadastro cad;
pacote_posicao::t32_odo_vel *odo_vel_gps;

typedef struct saidas
{
		unsigned int saida0 :1;
		unsigned int saida1 :1;
		unsigned int saida2 :1;
		unsigned int saida3 :1;
		unsigned int saida4 :1;
		unsigned int saida5 :1;
		unsigned int saida6 :1;
		unsigned int saida7 :1;
}__attribute__ ((packed)) saidas;

typedef struct sLapso
{
		//long int timestamp
		long int timestamp;
		double velocidade;
		double rpm;
		double acelx;
		double acely;
		int ed1;
		int ed2;
		int ed3;
		int ed4;
		int ed5;
		int ed6;
		int ed7;
		int ed8;
		int an1;
		int an2;
		int an3;
		int an4;
		double odometro;
		double horimetro;
		int idTrecho;
		std::string operacao;
		std::string ibtMotorista;
} sLapso;

void lapsoToTelemetria(struct sLapso& lapso)
{
	/*
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
		*/
}

void lapsoSetup(string lapso, struct sLapso& setup)
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

// Transforma uma struct sLapso em um formato persistível de texto
string persistableLapso(sLapso *l)
{
	if( l == NULL )
	{
		return "";
	}

	std::stringstream ss;
	ss << l->velocidade << "##" << l->rpm << "##" << l->acelx << "##" << l->acely << "##" << l->ed1 << l->ed2 << l->ed3 << l->ed4 << l->ed5 << l->ed6 << l->ed7 << l->ed8 << "##" << l->an1 << "##" << l->an2 << "##" << l->an3 << "##" << l->an4 << "##" << l->odometro << "##" << l->horimetro << "##" << l->ibtMotorista;

	return ss.str();
}

/*
	 Recebe o byte de expansão e calcula seu tamanho
	 */
int tamanhoLapsoExpansao(char expansao)
{

	char buffer[255];
	buffer[0] = expansao;
	saidas *p;
	p = (saidas* ) buffer;

	return ( p->saida3 * 7 ) + p->saida4 + p->saida5 + p->saida6 + ( p->saida7 * 2 );
}

/*
	 Recebe um struct com as saídas binárias do byte de controle e calcula o tamanho do lapso
	 */
int tamanhoLapsoExpansao(saidas *p)
{
	return ( p->saida3 * 7 ) + p->saida4 + p->saida5 + p->saida6 + ( p->saida7 * 2 );
}

/*
	 Recebe o byte de controle junto ao de expansão e calcula o tamanho do lapso
	 */
int tamanhoLapso(char controle, char expansao)
{

	char buffer[255];
	buffer[0] = controle;
	saidas *p;
	p = (saidas* ) buffer;
	int expLength = 0;

	if( p->saida0 )
	{

		return 1 + p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5 + tamanhoLapsoExpansao( expansao );

	}
	return p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5;
}

/*
	 Recebe um struct com as saídas binárias do byte de controle e calcula o tamanho do lapso
	 */
int tamanhoLapso(saidas *p)
{
	return p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5;
}

int parseBCDDecimal(unsigned char byte)
{
	return (unsigned int ) ( byte / 16 * 10 + byte % 16 );
}

string parseBCDString(unsigned char byte)
{
	return to_string( byte / 16 * 10 ) + to_string( byte % 16 );
}

tm* parseDataHora(string data)
{

	struct tm *dataHora;
	time_t currentTime = 0;
	time( &currentTime );
	dataHora = localtime( &currentTime );

	dataHora->tm_year = ( parseBCDDecimal( data.at( 0 ) ) * 100 ) + parseBCDDecimal( data.at( 1 ) ); //'YY'YYMMDDHHMMSS + YY'YY'MMDDHHMMSS
	dataHora->tm_mon = parseBCDDecimal( data.at( 2 ) ); //YYYY'MM'DDHHMMSS
	dataHora->tm_mday = parseBCDDecimal( data.at( 3 ) ); //YYYYMM'DD'HHMMSS
	dataHora->tm_hour = parseBCDDecimal( data.at( 4 ) ); //YYYYMMDD'HH'MMSS
	dataHora->tm_min = parseBCDDecimal( data.at( 5 ) ); //YYYYMMDDHH'MM'SS
	dataHora->tm_sec = parseBCDDecimal( data.at( 6 ) ); //YYYYMMDDHHMM'SS'

	dataHora->tm_mon -= 1;
	dataHora->tm_year -= 1900;
	//currentTime = mktime(dataHora);
	return dataHora;

}

string parseHora(string hora)
{
	string parsedHora;

	for(int i = 0; i < hora.length(); i++ )
	{

		parsedHora += to_string( parseBCDDecimal( hora[i] ) );
	}

	return parsedHora;

}

double parseOdometro(string odometro)
{
	//cout << "0 "<< dec << ((double)((unsigned char)odometro.at(0)*65536)) << endl;
	//cout << "1 "<< dec << ((double)((unsigned char)odometro.at(1)*256))<< endl;
	//cout << "2 "<< dec << ((double)((unsigned char)odometro.at(2)))<< endl;
	return ( ( (double ) ( (unsigned char ) odometro.at( 0 ) * 65536 ) ) + ( (double ) ( (unsigned char ) odometro.at( 1 ) * 256 ) ) + (double ) ( (unsigned char ) odometro.at( 2 ) ) ) / 10;
}

double parseHorimetro(string horimetro)
{
	/*cout << hex << setw(2) << setfill('0') << int((unsigned char)horimetro.at(0));
		cout << hex << setw(2) << setfill('0') << int((unsigned char)horimetro.at(horimetro.length()-1));
		cout <<  horimetro.length() << endl;*/
	return ( ( (double ) ( (unsigned char ) horimetro.at( 0 ) * 65536 ) ) + ( (double ) ( (unsigned char ) horimetro.at( 1 ) * 256 ) ) + (double ) ( (unsigned char ) horimetro.at( 2 ) ) ) / 10;
}

double parseLatLong(string operacao)
{
	/*

	O CALCULO DE LATLONG É FEITO COMO NO EXEMPLO:
	
	Se tivermos uma sequencia de operação 227477 por exemplo, cada byte deve ser covertido para
	decimal separadamento e então multiplicado pelo sua grandeza em dezena. Por exemplo, 22 para
	deciaml é 34, vezes 65536 (equivalente a 10000 em hexa) é igual a 2228224, somado a 74
	convertido para decimal 116 * 256 (equivalente a 100) = 29696, somado a 77 em decimal 119.
	Dessa soma então devem ser retirados grau, minuto e segundo da seguinte maneira:
	A soma do exemplo é a seguinte 2228224 + 29696 + 119 = 2258039. Então:
		2258039/100000 = 22 graus
		2258039 - 2200000 = 58039
		58039/1000 = 58 minutos
		58039 - 58000 = 039
		039 /10 = 3.9 segundos
	Obtidos grau, minuto e segundo, então a  conversão para o formato decimal de localização é feita:
		posicionamento = grau + (minuto/60) + (segundo/3600)
		
	*/

	double grau, minuto, segundo;
	unsigned int preConv;
	string latLong;
	preConv =  ((unsigned int)((unsigned char)operacao.at(0)*65536)) +
			((unsigned int)((unsigned char)operacao.at(1)*256)) +
			((unsigned int)((unsigned char)operacao.at(2)));

	cout << endl << endl << hex << setw(2) << setfill('0') << int((unsigned char)operacao.at(0))
		 << hex << setw(2) << setfill('0') << int((unsigned char)operacao.at(1))
		 << hex << setw(2) << setfill('0') << int((unsigned char)operacao.at(2)) << endl;

	cout << "Latlong " << dec << preConv << endl;

	grau = preConv/100000;
	
	cout << "Latlong graus " << dec << grau << endl;
	
	preConv -= grau * 100000;
	minuto = preConv/1000;
	
	cout << "Latlong " << dec << preConv << endl;
	cout << "Latlong minutos " << dec << minuto << endl;
	
	preConv -= minuto * 1000;
	segundo = ((double)preConv)/10.0;

	cout << "Latlong " << dec << preConv << endl;
	cout << "Latlong segundos " << dec << segundo << endl;
	cout << "Latlong retorno " << dec << grau+(minuto/60.0)+(segundo/3600.0) << endl;

	return grau+(minuto/60.0)+(segundo/3600.0);

}

double parseLatitude(string operacao, int yAxis)
{

	double latitude = parseLatLong( operacao );

	if( yAxis )
	{
		return latitude * -1;
	}
	return latitude;

}

double parseLongitude(string operacao, int xAxis, int complemento)
{

	double longitude = parseLatLong( operacao );

	if( complemento )
	{
		longitude += 100;
	}

	if( xAxis )
	{
		return longitude * -1;
	}

	return longitude;

}

void parseHFULL(string strHfull, unsigned int ponteiroIni, unsigned int ponteiroFim, unsigned int arquivo)
{

	//cout << "DEBUG -- Parse HFULL ..." ;
	bluetec::sHfull hfull;
	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize;

	char buffer[255];
	buffer[0] = (unsigned char ) strHfull.at( 0 );
	saidas *p;
	p = (saidas* ) buffer;

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
	//hfull.confHardwareRe; //Configurações de Hardware (Redundância)
	//hfull.limiteAnalogico1;
	//hfull.limiteAnalogico234[3];
	//hfull.lapsosGravacaoEvento; //Lapsos de gravação do evento
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

	fileManager.saveHfull( getVeioid(), hfull );

	cout << "DEBUG -- CORRIGINDO PONTEIRO ANTES E DEPOIS DO HFULL..."<< endl;
	// se existe algo que encaixe no fim desse hfull...
	if( fileManager.getBufferFile( getVeioid(), ponteiroFim+1, arquivo, tBuffer, tSize, header ) &&
			header.beginPointer == ponteiroFim+1 &&
			(header.dataType == bluetec::enumDataType::DADOS ||
			 header.dataType == bluetec::enumDataType::DADOS_FINAL ||
			 header.dataType == bluetec::enumDataType::FINAL)){
		cout << "DEBUG -- HFULL + TEMP"<< endl;
		// ...deleto essa coisa para sobrescreve-la...
		fileManager.delFile( getVeioid(), header.headerFile );
		//... atualizo o ponteiro inicial dessa coisa...
		header.beginPointer -= 512;
		//... então salvo
		cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
		fileManager.saveBufferFile( getVeioid(), tBuffer, tSize, header );
	}
	// ou se existe algo que encaixe no começo desse hfull...
	if ( fileManager.getBufferFile( getVeioid(), ponteiroIni-1, arquivo, tBuffer, tSize, header ) &&
		 header.endPointer == ponteiroIni-1 &&
		 (header.dataType == bluetec::enumDataType::DADOS ||
		  header.dataType == bluetec::enumDataType::HSYNS ||
		  header.dataType == bluetec::enumDataType::HSYNS_DADOS)){
		cout << "DEBUG -- TEMP + HFULL"<< endl;
		// ...deleto essa coisa para sobrescreve-la...
		fileManager.delFile( getVeioid(), header.headerFile );
		//... atualizo o ponteiro inicial dessa coisa...
		header.endPointer += 512;
		//... então salvo
		cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
		fileManager.saveBufferFile( getVeioid(), tBuffer, tSize, header );
	}


}

void parseHSYNC(string hsync)
{

}

void parseA3A5A7(unsigned int ponteiroIni, unsigned int arquivo){
	
	//procurar trecho pre-persistido que termine com ponteiroIni-1 e apaga-lo
	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize=0;

	cout << "DEBUG -- PROCURANDO COMEÇO DESSE FIM DE TRECHO..."<< endl;
	// se existe algo que encaixe no começo desse fim de trecho...
	if( fileManager.getBufferFile( getVeioid(), ponteiroIni-1, arquivo, tBuffer, tSize, header ) &&
			header.endPointer == ponteiroIni-1 &&
			header.file == arquivo){

		cout << "DEBUG -- ESSE FIM DE TRECHO TEM UM COMEÇO..."<< endl;

		switch (header.dataType){
		case bluetec::enumDataType::HSYNS_DADOS:
			cout << "DEBUG -- TRECHO COMPLETO, APAGANDO MEMORIA TEMPORARIA..."<< endl;
			fileManager.delFile( getVeioid(), header.headerFile );
			break;
		case bluetec::enumDataType::DADOS:
			cout << "DEBUG -- TRECHO INCOMPLETO, INCREMENTANDO FINAL DE TRECHO..."<< endl;
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			fileManager.delFile( getVeioid(), header.headerFile );
			//... atualizo o tipo de dados...
			header.dataType = bluetec::enumDataType::DADOS_FINAL;
			//... então salvo
			fileManager.saveBufferFile( getVeioid(), tBuffer, tSize, header );
			break;
		}

	}else{
		//senão ele precisa ser armazenado temporariamente...
		header.file = arquivo;
		header.beginPointer = ponteiroIni;
		header.dataType = bluetec::enumDataType::FINAL;
		cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
		fileManager.saveBufferFile( getVeioid(), tBuffer, (uint32_t)0, header );
	}

}

void parseHSYNS(string hsyns, unsigned int arquivo, unsigned int ponteiroFim){
	
	struct sLapso lapso;
	bluetec::sBluetecHeaderFile header;
	string pLapso;

	lapso.idTrecho = fileManager.getNextIdTrecho();;
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


	cout << "DEBUG -- PROCESSANDO HSYNS "<< dec << ponteiroFim << " ..." << endl;

	lapso.timestamp = mktime( parseDataHora( hsyns.substr( 8, 7 ) ) );
	//carrega ibutton do motorista em bcd
	lapso.ibtMotorista = "";
	lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 1 ) );
	//padroniza operação de código vazio 81
	if( lapso.ibtMotorista == "81" )
	{
		//ibutton padrão
		lapso.ibtMotorista = "0";
	}
	else
	{
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 2 ) );
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 3 ) );
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 4 ) );
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 5 ) );
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 6 ) );
		lapso.ibtMotorista += parseBCDDecimal( hsyns.at( 7 ) );
	}
	cout << "DEBUG -- PARSE DE TIMESTAMP HEX =  " << dec << hsyns.substr( 8, 7 ) << endl;
	cout << "DEBUG -- PARSE DE TIMESTAMP =  " << dec << lapso.timestamp << endl;
	lapso.odometro = parseOdometro( hsyns.substr( 15, 3 ) );
	cout << "DEBUG -- PARSE DE ODOMETRO =  " << dec << lapso.odometro << endl;
	cout << hex << setw(2) << setfill('0') << int((unsigned char)hsyns.at(0)) << endl;
	cout << hex << setw(2) << setfill('0') << int((unsigned char)hsyns.at(hsyns.length()-1)) << endl;
	lapso.horimetro = parseHorimetro( hsyns.substr( 18, 3 ) );
	cout << "DEBUG -- PARSE DE HORIMETRO =  " << dec << lapso.horimetro << endl;

	pLapso = persistableLapso( &lapso );

	header.file = arquivo;
	header.endPointer = ponteiroFim;
	header.timestamp = lapso.timestamp;
	header.dataType = bluetec::enumDataType::HSYNS;
	printf("IMC 1 - header.dataType = %d\n",header.dataType);
	cout << "DEBUG -- GRAVANDO HSYNS "<<  dec << header.endPointer << " ..." << endl;
	//salva o início de um trecho
	cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << header.dataType << endl;
	cout << "DEBUG -- saveBufferFile ENUM   DATATYPE " << dec << bluetec::enumDataType::HSYNS << endl;

	printf("IMC 2 - header.dataType = %d\n",header.dataType);
	fileManager.saveBufferFile( getVeioid(), pLapso.c_str(), pLapso.length(), header );

}

void transmitir_dados_serializados(std::string data)
{
	cout << "------------JSON Begin" << endl;
	cout << data << endl;
	cout << "------------JSON END" << endl;
}

void parseDados(string dados, int ponteiroIni, int ponteiroFim, int arquivo)
{
	fileManager.setPath("/home/murilo/Documents/bluetec/data/");

	bluetec::sBluetecHeaderFile header;
	char tBuffer[500000];
	uint32_t tSize=0;
	bluetec::sHfull hfull;
	struct sLapso lapso;
	int tipoDado = 0;
	int index = 0;
	int fim = 0;
	char bufferControle[255];
	saidas *controle;
	char bufferExpansao[255];
	saidas *expansao;
	string operacao;
	int tamLapso = 0;
	char bufferED[255];
	saidas *ed;

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
	if( !fileManager.getHfull( getVeioid(), hfull ) )
	{
		hfull.lapso = bluetec::enumDefaultValues::LAPSO;
		hfull.acely = bluetec::enumDefaultValues::ACELY;
		hfull.acelx = bluetec::enumDefaultValues::ACELX;
		hfull.spanAcel = bluetec::enumDefaultValues::SPANACEL;
	}

	cout << "DEBUG -- PROCURANDO ALGO ANTES DESSE ARQUIVO..."<< endl;
	// se existe algo que encaixe no começo...
	if( fileManager.getBufferFile( getVeioid(), ponteiroIni-1, arquivo, tBuffer, tSize, header ) &&
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
				//nesse caso o trecho ja foi iniciado então o primeiro lapso deve ser restaurado
				lapsoSetup( tBuffer, lapso );
				tipoDado = bluetec::enumDataType::HSYNS_DADOS;
				//apago o lapso tempórário...
				fileManager.delFile( getVeioid(), header.headerFile );
			break;
			case bluetec::enumDataType::DADOS:
				//nesse caso o pedaço de dados encontrado não significa nada então será apenas atualizado...
				dados = string(tBuffer) + dados;
				// ...deleto esse arquivo temporario para sobrescreve-lo...
				fileManager.delFile( getVeioid(), header.headerFile );
				tipoDado = bluetec::enumDataType::DADOS;
			break;
		}
	}
	cout << "DEBUG -- PROCURANDO ALGO DEPOIS DESSE ARQUIVO..."<< endl;
	// se existe algo que encaixe no final...
	if( fileManager.getBufferFile( getVeioid(), ponteiroFim+1, arquivo, tBuffer, tSize, header ) &&
			header.beginPointer == ponteiroFim+1 &&
			header.file == arquivo){

		// DADOS + TEMP
		ponteiroFim = header.endPointer;

		cout << "DEBUG -- ENCONTRADO COM TIPO "<< dec << header.dataType <<"..."<< endl;
		cout << "DEBUG -- ENCONTRADO COM PONTEIRO FIM "<< dec << header.endPointer <<"..."<< endl;
		lapso.timestamp = header.timestamp;
		switch (header.dataType){
		case bluetec::enumDataType::DADOS:
			//nesse caso o pedaço de dados encontrado não significa nada entãos será apenas atualizado...
			dados = dados + string(tBuffer);
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			fileManager.delFile( getVeioid(), header.headerFile );
			tipoDado = bluetec::enumDataType::DADOS;
			break;
		case bluetec::enumDataType::FINAL:
		case bluetec::enumDataType::DADOS_FINAL:
			if(header.dataType == bluetec::enumDataType::DADOS_FINAL){
				dados = dados + string(tBuffer);
			}
			// ...deleto esse arquivo temporario para sobrescreve-lo...
			fileManager.delFile( getVeioid(), header.headerFile );
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
		fileManager.saveBufferFile( getVeioid(), dados.c_str(), dados.length(), header );
	}else if(tipoDado){
		//se tipoDado não é bem trecho nõa iniciado (DADOS E DADOS_FINAL) então se ele existir,
		//significa que tenho um trecho iniciado, então processo os lapsos
		cout << "DEBUG -- TRECHO CONHECIDO, INCIANDO PROCESSAMENTO..."<< endl;
		fim = dados.length();
		while(index < fim )
		{

			try
			{
				//cout << "       " << index << " 	" << fim << " tipo_dado " << dec << tipo_dado<< endl;

				//calculo o tamanho do byte de controle
				//cout << "tamanho byte @"<<dec<<index<< " length " << dec << hsyns.length() << " fim " << dec << fim << endl;
				bufferControle[0] = dados.at( index );
				controle = (saidas* ) bufferControle;

				tamLapso = tamanhoLapso( controle );
				//e se o bit de expansão estiver ativo, calculo também o byte de expansão
				if( controle->saida0 && index + 1 <= fim )
				{
					index++; //o byte de expansao vem logo apos o byte de controle, então já incremento para calcular corretamente os próximos valores
					//cout << "tamanho exp @"<<dec<<index<< endl;
					bufferExpansao[0] = dados.at( index );
					expansao = (saidas* ) bufferExpansao;
					tamLapso += ( tamanhoLapsoExpansao( expansao ) + 1 );
				}

				//cout << ">> tamanho: " << dec << tamLapso << endl;
				//cout << ">>   index: " << dec << index << endl;
				//cout << ">>     fim: " << dec << fim << endl;
				//verifico se esse lapso está completo nesse pedaço de trecho
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
					if( controle->saida1 )
					{
						//cout << "------ ed ------ ";
						index++;
						//cout << "ed @"<<dec<<index<< endl;
						bufferED[0] = dados.at( index );
						ed = (saidas* ) bufferED;

						lapso.ed1 = (unsigned int ) ed->saida7;
						lapso.ed2 = (unsigned int ) ed->saida6;
						lapso.ed3 = (unsigned int ) ed->saida5;
						lapso.ed4 = (unsigned int ) ed->saida4;
						lapso.ed5 = (unsigned int ) ed->saida3;
						lapso.ed6 = (unsigned int ) ed->saida2;
						lapso.ed7 = (unsigned int ) ed->saida1;
						lapso.ed8 = (unsigned int ) ed->saida0;
						//cout << ((unsigned int)hsyns.at(index)) << endl;
					}
					//verifico se há byte de expansão novamente, dessa vez para processa-lo
					if( controle->saida0 )
					{
						//cout << "------ expansao ------" << endl;
						//TODO: CALCULAR AS INFORMAÇÕES DO BYTE DE EXPANSAO
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
						//verifico se existe informação de operacao
						if( expansao->saida3 )
						{
							operacao = dados.substr( index + 1, 7 );
							lapso.operacao = operacao;
							//cout << "controle oper @"<<dec<<0<< endl;
							unsigned char controleOper = operacao.at( 0 );
							//cout << "------ operacao  "<< asctime (gmtime ( &lapso.timestamp )) << " ------ " << hex << setw(2) << setfill('0') << int(controleOper) << " - " << hex << setw(2) << setfill('0') << int((unsigned char)operacao.at(operacao.length()-1)) << endl;
							index += 7;
							//agora preciso ver se essa informação é de gps
							if( controleOper >= 0xA0 )
							{
								cout << "------ gps ------";

								char buffer[255];
								buffer[0] = controleOper;
								saidas *p;
								p = (saidas* ) buffer;
								double lat = parseLatitude( operacao.substr( 1, 3 ), p->saida2 );
								double lon = parseLongitude( operacao.substr( 4, 3 ), p->saida1, p->saida0 );

								printf( " lat long -> %20.18f %20.18f \n ", lat, lon );

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

								//Envia para a fila o pacote de posição já em sem formato final protobuf.
								bluetecPacote.SerializeToString(&serializado);
								transmitir_dados_serializados(serializado);
								bluetecPacote.Clear();
								//bluetecPacote.clear_tb();
								//reinicia o tamanho máximo do pacote pois já enviou
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

					//popula o protobuf com as variáveis de lapso

					//coloca esse novo protobuf na lista

					index++;
					//cria um novo pacote

					//verifica se esse novo pacote vai estourar o tamamho máximo
					sizePacote += lapsoSize;
					if( sizePacote > maxSizePacote )
					{
						cout << " tamanho maximo atingido, enviando pacote com " << dec << sizePacote / lapsoSize << " lapsos de " << dec << lapsoSize << " bytes" << endl;
						//se estourar já envia antes
						bluetecPacote.SerializeToString( &serializado );
						transmitir_dados_serializados( serializado );
						bluetecPacote.clear_tb();
						//reinicia o valor do tamanho do pacote com o tamanho do lapso que não foi enviado
						sizePacote = lapsoSize;
					}
					else
					{
						//telemetria = bluetecPacote.add_tb();
						lapsoToTelemetria(lapso);
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

		bluetecPacote.SerializeToString( &serializado );
		transmitir_dados_serializados( serializado );
		bluetecPacote.clear_tb();

		//se não tiver  fim de trecho, precisa retornar o último lapso para
		//persisti-lo e não haver reprocessamento
		if( tipoDado != bluetec::enumDataType::HSYNS_FINAL)
		{
			header.beginPointer = ponteiroIni;
			header.endPointer = ponteiroFim;
			header.file = arquivo;
			header.idTrecho = lapso.idTrecho;
			string pLapso = persistableLapso( &lapso );
			cout << "DEBUG -- saveBufferFile HEADER DATATYPE " << dec << header.dataType << endl;
			fileManager.saveBufferFile( getVeioid(), pLapso.c_str(), pLapso.length(), header );
		}

	}

}

int protocolo::preenche_cadastro(pacote_posicao::equip_contrato *contrato, std::string chave, cache_cadastro &retorno)
{
	retorno.veioid = 0;

	retorno.esn = atoi(chave.c_str());

	//cache_cadastral::consulta_cadastro(retorno, chave);
	// MOCK
	retorno.esn = 0;
	retorno.id = 0;
	retorno.antena_int = 0;
	retorno.antena_text = "";
	retorno.antena_tipo = 0;
	retorno.projeto = 0;
	retorno.clioid = 0;
	retorno.ger1 = 0;
	retorno.ger2 = 0;
	retorno.ger3 = 0;
	retorno.connumero = 0;
	retorno.veioid = 123456;
	retorno.tipo_contrato = 0;
	retorno.classe = 0;
	retorno.serial0 = 0;
	retorno.serial1 = 0;
	retorno.is_sasgc = 0;
	retorno.tipo_veiculo = 0;
	retorno.debug = 0;
	retorno.placa = "TCM0900";
	retorno.rpm_maximo = 0;
	retorno.atuadores.clear();
	retorno.sensores.clear();

	if(retorno.veioid > 0)
	{
		//preenche_campos_cadastro(contrato, retorno);
		// MOCK
		contrato->set_esn(retorno.esn);
		contrato->set_veioid(retorno.veioid);
		contrato->set_clioid(retorno.clioid);
		contrato->set_is_sasgc(retorno.is_sasgc);
		contrato->set_tipo_contrato(retorno.tipo_contrato);
		contrato->set_classe(retorno.classe);

		// retira a nescessidade de implementacao individual em cada parser
		contrato->set_protocolo(projeto2protocolo(retorno.projeto));
		contrato->set_id(retorno.id);

		contrato->set_porta_panico(0);
		contrato->set_porta_bloqueio(0);

		contrato->set_connumero(retorno.connumero);
		contrato->set_tipo_veiculo(retorno.tipo_veiculo);

		return 1;
	}
	else
	{
		contrato->set_esn(atoi(chave.c_str()));
	}

	return 0;
}

void protocolo::processa_pacote(char *buffer, int len)
{
	pacote_posicao::equip_contrato *contrato;
	FILE *in = 0;
	unsigned char bt4[6500];
	std::string sbt4;

	bluetecPacote.Clear();
	pacote = bluetecPacote.mutable_pe();
	contrato = pacote->mutable_ec();

	/* buffer contem o path do arquivo recebido.
		 *
		 * ****************** ATENÇÃO ******************************
		 * Abrir o path como somente LEITURA, não abrir para escrita.
		 */
	std::cout << "Processando :" << buffer << std::endl;

	//obtendo a placa do path...
	std::string placa( buffer );
	placa = placa.substr( 0, placa.find_last_of( "/" ) );
	placa = placa.substr( placa.find_last_of( "/" ) + 1, placa.length() );
	std::transform( placa.begin(), placa.end(), placa.begin(), ::toupper );

	//obtendo o nome do arquivo
	std::string arquivo( buffer );
	arquivo = arquivo.substr( arquivo.find_last_of( "/" ) + 1, arquivo.length() );
	std::transform( arquivo.begin(), arquivo.end(), arquivo.begin(), ::toupper );
	std::cout << "Arquivo sendo processado :" << arquivo << std::endl;

	//no minimo o nome do arquivo tem que ser BT4
	if( arquivo.length() < 3 || ( arquivo.substr( 0, 3 ) != "BT4" ) )
	{
		std::cout << "Arquivo nao BT4 Ignorando..." << std::endl;
		return;
	}

	// Obtem informações cadastrais.
	preenche_cadastro(contrato, placa.c_str(), cad);

	if( !cad.veioid )
	{
		cout << "Placa nao cadastrada.... " << endl;
		return;
	}

	in = fopen( buffer, "rb" );

	if( in )
	{

		puts( "DEBUG -- CARREGANDO ARQUIVO" );

		size_t lSize;
		size_t result;

		fseek( in, 0, SEEK_END );

		lSize = ftell( in );

		rewind( in );

		result = fread( bt4, 1, lSize, in );

		if( ( result != lSize ) || ( lSize == 0 ) )
		{
			puts( "Reading error" );
			return;
		}


		std::string sbt4( bt4, bt4 + lSize );

		size_t pos = 0;
		int arquivo = (int ) sbt4[1];

		cout << "DEBUG -- ARQUIVO " << dec << arquivo << endl;

		long ponteiroIni = ( (long ) bt4[2] * 65536 ) + ( (long ) bt4[3] * 256 ) + (long ) bt4[4];
		cout << "DEBUG -- PONTEIRO INI " << (long ) bt4[2] << "*65536+" << (long ) bt4[3] << "*256+" << (long ) bt4[4] << "=" << dec << ponteiroIni << endl;

		long ponteiroFim = ( (long ) bt4[lSize - 8] * 65536 ) + ( (long ) bt4[lSize - 7] * 256 ) + (long ) bt4[lSize - 6];
		cout << "DEBUG -- PONTEIRO FIM " << dec << ponteiroFim << endl;

		//a cada 1200 bytes do bluetec gera um cabeçalho com 5 caracteres no começo e 8 no final
		//esses caracteres devem ser removidos
		sbt4.erase( 0, 5 );
		sbt4.erase( sbt4.length() - 8, 8 );
		lSize -= 13;

		for(size_t i = 1200; i < sbt4.length(); i += 1200 )
		{

			sbt4.erase( i, 13 ); // apagaa os 13 bytes (5 de início e 8 de fim) de uma transição de um subpacote para outro
			lSize -= 13;
		}

		puts( "DEBUG -- INICIANDO LAPSOS" );
		//int tipo_dado = bluetec::enumDataType::DADOS;
		size_t inicio = 0;

		// HEADER OFFSETS:
		size_t lFimTrecho = 12;
		size_t lHsync = 21;
		size_t lHsyns = 21;
		size_t lHfull = 250;

		string fimTrecho;
		fimTrecho.push_back( (unsigned char ) 0x82 );
		fimTrecho.push_back( (unsigned char ) 0xA3 );
		fimTrecho.push_back( (unsigned char ) 0xA5 );
		fimTrecho.push_back( (unsigned char ) 0xA7 );

		cout << "DEBUG -- BUSCANSO CABECALHOS... " << endl;
		for(size_t i = 0; i < sbt4.length(); i++)
		{
			if(sbt4.at(i) == (unsigned char) 0x48){

				if(sbt4.compare(i,5,"HSYNC")==0){
					cout << "DEBUG --  ACHOU HSYNC em " << dec << i << endl;
					if( inicio < i - lHsync )
					{
						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 6 de " <<dec<< inicio << " a " << i - lHsync - 1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i - lHsync - 1)) << endl;
						inicio = i - lHsync;
					}
					if( inicio == i -  lHsync)
					{
						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 2 de " <<dec<< inicio << " a " << i-1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio));
						cout << " a ";
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i-1)) << endl;
						//parseHSYNC( sbt4.substr( i - 21, 26 ) );
						i+=5;
						inicio=i;
					}
					cout << "DEBUG -- inicio " << dec << inicio << " i " << dec << i << endl;

					
				} else if(sbt4.compare(i,5,"HSYNS")==0){
					cout << "DEBUG --  ACHOU HSYNS em " << dec << i << endl;
					if( inicio < i - lHsyns )
					{
						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 6 de " <<dec<< inicio << " a " << i - lHsyns - 1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i - lHsyns - 1)) << endl;
						parseDados(sbt4.substr(inicio, i - inicio - lHsyns), ponteiroIni + inicio, ponteiroIni + i-lHsyns - 1, arquivo);
						inicio = i - lHsyns;
					}
					if( inicio == i -  lHsyns)
					{
						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 3 de " <<dec<< inicio << " a " << i-1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i-1)) << endl;

						//i está em 'H', então conta mais 4 ponteiros a frente, ou seja, 'H' + 'SYNS'
						cout << "DEBUG -- PARAMS HSYNS " << dec << inicio << " " << dec << arquivo << " " << dec << ponteiroIni << "+" << i << "+4" <<endl;
						parseHSYNS( sbt4.substr( inicio, lHsyns ), arquivo, ponteiroIni +i+4);
						i+=5;
						inicio=i;
					}
					cout << "DEBUG -- inicio " << dec << inicio << " i " << dec << i << endl;

					
				} else if(sbt4.compare(i,5,"HFULL")==0){

					cout << "DEBUG --  ACHOU HFULL em " << dec << i << endl;
					if( inicio < i - lHfull )
					{

						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 6 de " <<dec<< inicio << " a " << i - lHfull - 1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i - lHfull - 1)) << endl;
						parseDados(sbt4.substr(inicio, i - inicio - lHfull), ponteiroIni + inicio, ponteiroIni + i-lHfull - 1, arquivo);
						inicio = i - lHfull;
					}
					if( inicio == i -  lHfull)
					{
						cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 1 de " <<dec<< inicio << " a " << i-1 << endl;
						cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i-1)) << endl;
						parseHFULL( sbt4.substr( inicio, i-inicio ), ponteiroIni+inicio, i+4, arquivo);

						i+=5;
						inicio=i;
					}
					cout << "DEBUG -- inicio " << dec << inicio << " i " << dec << i << endl;

				}

			}else if(i+4 < sbt4.length() &&
					 sbt4.at(i) == (unsigned char) 0x82 &&
					 sbt4.compare(i,4,fimTrecho)==0)
			{
				cout << "DEBUG --  ACHOU FIM DE TRECHO EM " << dec << i << endl;
				if( inicio < i)
				{
					cout << "DEBUG -- MONTANDO ARQUIVO DO TIPO 6 de " <<dec<< inicio << " a " << i-1 << endl;
					cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(i - 1)) << endl;
					parseDados(sbt4.substr(inicio, i - inicio), ponteiroIni + inicio, ponteiroIni + i-1, arquivo);
					inicio = i;
				}
				if( inicio == i &&
						i + lFimTrecho <= sbt4.length())
				{
					cout << "DEBUG -- FECHANDO TRECHO DE PONTEIRO FINAL " <<dec<< i-1 << endl;
					parseA3A5A7(ponteiroIni + i, arquivo);
					i+=lFimTrecho+1;
					inicio=i;
				}
				cout << "DEBUG -- inicio " << dec << inicio << " i " << dec << i << endl;
			}


		}

		if(inicio < sbt4.length()){
			cout << "DEBUG -- ENVIANDO TIPO 6 NO FIM DO BUFFER " <<dec<< inicio << " a " << sbt4.length()-1 << endl;
			cout << hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(inicio))<< " a "<< hex << setw(2) << setfill('0') << int((unsigned char)sbt4.at(sbt4.length()-1)) << endl;
			parseDados(sbt4.substr(inicio, sbt4.length()-inicio), ponteiroIni + inicio, ponteiroFim, arquivo);
		}

		fclose( in );
	}

}
int protocolo::projeto2protocolo(uint32_t projeto)
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
;
