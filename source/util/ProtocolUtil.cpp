#include <iomanip>
#include "util/ProtocolUtil.hpp"
#include "util/Log.hpp"
#include <math.h>

#define TAG "[ProtocolUtil] "


namespace BTCloud { namespace Util {

void LapsoSetup(string lapso, struct Lapse& setup)
{
	if(lapso.length() == 0)
		return;

	Dbg(TAG "Lapso setup: %s", lapso.c_str());

	size_t index = 0;
	setup.velocidade = stoi(lapso.substr(index, lapso.find("##", index)));
	Dbg(TAG "Index: %d Speed: %5.2f", index, setup.velocidade);

	index = lapso.find("##", index) + 2;
	setup.rpm = stoi(lapso.substr(index, lapso.find("##", index)));
	Dbg(TAG "Index: %d Rpm: %5.2f", index, setup.rpm);

	index = lapso.find("##", index ) + 2;
	setup.acelx = stoi(lapso.substr(index, lapso.find("##", index)));
	Dbg(TAG "Index: %d Acel x: %5.2f", index, setup.acelx);

	index = lapso.find("##", index) + 2;
	setup.acely = stoi(lapso.substr(index, lapso.find("##", index)));
	Dbg(TAG "Index: %d Acel y: %5.2f", index, setup.acely);

	index = lapso.find("##", index) + 1;

	setup.ed1 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 1: %c", index, setup.ed1);

	setup.ed2 = (int) (lapso.at(++index));
	Dbg(TAG "Index %d Ed 2: %c", index, setup.ed2);

	setup.ed3 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 3: %c", index, setup.ed3);

	setup.ed4 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 4: %c", index, setup.ed4);

	setup.ed5 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 5: %c", index, setup.ed5);

	setup.ed6 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 6: %c", index, setup.ed6);

	setup.ed7 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 7: %c", index, setup.ed7);

	setup.ed8 = (int) (lapso.at(++index));
	Dbg(TAG "Index: %d Ed 8: %c", index, setup.ed8);

	index = lapso.find("##", index) + 2;
	setup.an1 = stoi(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d An 1: %d", index, setup.an1);

	index = lapso.find("##", index) + 2;
	setup.an2 = stoi(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d An 2: %d", index, setup.an2);

	index = lapso.find("##", index) + 2;
	setup.an3 = stoi(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d An 3: %d", index, setup.an3);

	index = lapso.find("##", index) + 2;
	setup.an4 = stoi(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d An 4: %d", index, setup.an4);

	index = lapso.find("##", index) + 2;
	setup.odometro = stof(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d odometer: %f", index, setup.odometro);

	index = lapso.find("##", index) + 2;
	setup.horimetro = stof(lapso.substr(index, lapso.find("##", index) - 1));
	Dbg(TAG "Index: %d Horimetro: %f", index, setup.horimetro);

	index = lapso.find("##", index) + 2;
	setup.lastIncompleteLapse = lapso.substr(index, lapso.find("##", index) - index);
	Dbg(TAG "Incomplete lapse: %d Horimetro: %s", index, setup.lastIncompleteLapse.c_str());

	index = lapso.find("##", index) + 2;
	setup.ibtMotorista = lapso.substr(index, lapso.find("##", index) - 1);

	// Force to keep maxium size
	if(setup.ibtMotorista.size() > 14)
		setup.ibtMotorista.resize(14);

	Dbg(TAG "Index: %d ibtMotorista: %s", index, setup.ibtMotorista.c_str());
}

void LapsoToTelemetry(Entities::Telemetry *tele, struct Lapse& lapso)
{
	tele->stretch = lapso.idTrecho;
	tele->dateTime = lapso.timestamp;
	tele->velocity = lapso.velocidade;
	tele->rpm = lapso.rpm;
	tele->odometer = lapso.odometro;
	tele->hourmeter = lapso.horimetro;
	tele->ed1 = lapso.ed1;
	tele->ed2 = lapso.ed2;
	tele->ed3 = lapso.ed3;
	tele->ed4 = lapso.ed4;
	tele->ed5 = lapso.ed5;
	tele->ed6 = lapso.ed6;
	tele->ed7 = lapso.ed7;
	tele->ed8 = lapso.ed8;
	tele->acelX = lapso.acelx;
	tele->acelY = lapso.acely;
	tele->an1 = lapso.an1;
	tele->an2 = lapso.an2;
	tele->an3 = lapso.an3;
	tele->an4 = lapso.an4;
	tele->operation = lapso.operacao;
}

// Transform a sLapse struct in a persistable text format
string PersistableLapse(Lapse *l)
{
	if( l == NULL )
	{
		Dbg(TAG "PersistableLapse: ");
		return "";
	}

	std::stringstream ss;

	ss << l->velocidade << "##" << l->rpm << "##" << l->acelx << "##" << l->acely << "##" << l->ed1 << l->ed2 << l->ed3 << l->ed4 << l->ed5 << l->ed6 << l->ed7 << l->ed8 << "##" << l->an1 << "##" << l->an2 << "##" << l->an3 << "##" << l->an4 << "##" << l->odometro << "##" << l->horimetro << "##" << l->lastIncompleteLapse << "##" << l->ibtMotorista;
	//ss << l->velocidade << "##" << l->rpm << "##" << l->acelx << "##" << l->acely << "##" << l->ed1 << l->ed2 << l->ed3 << l->ed4 << l->ed5 << l->ed6 << l->ed7 << l->ed8 << "##" << l->an1 << "##" << l->an2 << "##" << l->an3 << "##" << l->an4 << "##" << l->odometro << "##" << l->horimetro << "##" << l->ibtMotorista;

	Dbg(TAG "PersistableLapse: %s", ss.str().c_str());
	return ss.str();
}

// Receive the expansion byte and calculates his size
int ExpasionSize(char expansao)
{
	char buffer[255];
	buffer[0] = expansao;
	BTCloud::Util::Output *p;
	p = (BTCloud::Util::Output* ) buffer;

	return (p->saida3 * 7) + p->saida4 + p->saida5 + p->saida6 + (p->saida7 * 2);
}

// Receive a struct with the binary output from control byte and calc lapse size
int ExpasionSize(Output *p)
{
	return (p->saida3 * 7) + p->saida4 + p->saida5 + p->saida6 + (p->saida7 * 2);
}

// Receive the control byte with the expansion and calc lapse size
int LapseSize(char controle, char expansao)
{
	char buffer[255];
	buffer[0] = controle;
	Output *p;
	p = (Output*) buffer;
	int expLength = 0;

	if(p->saida0)
	{
		return 1 + p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5 + ExpasionSize(expansao);
	}
	return p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5;
}

// Receive a struct with the binary output from control byte and calc lapse size
int LapseSize(Output *p)
{
	return p->saida1 + p->saida2 + p->saida3 + p->saida4 + p->saida5;
}

int ParseBCDDecimal(unsigned char byte)
{
	unsigned int data = (unsigned int) (byte / 16 * 10 + byte % 16);
	return data;
}

string ParseBCDString(unsigned char byte)
{
	string data = to_string( byte / 16 * 10 ) + to_string( byte % 16 );
	return data;
}

tm* ParseTimeDate(string data)
{
	struct tm *dataHora;
	time_t currentTime = 0;
	time(&currentTime);
	dataHora = localtime(&currentTime);

	dataHora->tm_year = 2000 + ParseBCDDecimal(data.at(1)); //'YY'YYMMDDHHMMSS + YY'YY'MMDDHHMMSS
	dataHora->tm_mon = ParseBCDDecimal(data.at(2)); //YYYY'MM'DDHHMMSS
	dataHora->tm_mday = ParseBCDDecimal(data.at(3)); //YYYYMM'DD'HHMMSS
	dataHora->tm_hour = ParseBCDDecimal(data.at(4)); //YYYYMMDD'HH'MMSS
	dataHora->tm_min = ParseBCDDecimal(data.at(5)); //YYYYMMDDHH'MM'SS
	dataHora->tm_sec = ParseBCDDecimal(data.at(6)); //YYYYMMDDHHMM'SS'

	dataHora->tm_mon -= 1;
	dataHora->tm_year -= 1900;
	//currentTime = mktime(dataHora);
	return dataHora;
}

string ParseTime(string hora)
{
	string parsedHora;

	for(int i = 0; i < hora.length(); i++ )
	{
		parsedHora += to_string(ParseBCDDecimal(hora[i]));
	}
	return parsedHora;
}

double ParseHodometer(string odometro)
{
	//cout << "0 "<< dec << ((double)((unsigned char)odometro.at(0)*65536)) << endl;
	//cout << "1 "<< dec << ((double)((unsigned char)odometro.at(1)*256))<< endl;
	//cout << "2 "<< dec << ((double)((unsigned char)odometro.at(2)))<< endl;
	return ( ( (double ) ( (unsigned char ) odometro.at( 0 ) * 65536 ) ) + ( (double ) ( (unsigned char ) odometro.at( 1 ) * 256 ) ) + (double ) ( (unsigned char ) odometro.at( 2 ) ) ) / 10;
}

double ParseHourmeter(string horimetro)
{
	/*cout << hex << setw(2) << setfill('0') << int((unsigned char)horimetro.at(0));
		cout << hex << setw(2) << setfill('0') << int((unsigned char)horimetro.at(horimetro.length()-1));
		cout <<  horimetro.length() << endl;*/
	return ( ( (double ) ( (unsigned char ) horimetro.at( 0 ) * 65536 ) ) + ( (double ) ( (unsigned char ) horimetro.at( 1 ) * 256 ) ) + (double ) ( (unsigned char ) horimetro.at( 2 ) ) ) / 10;
}

double ParseLatLong(uint8_t precision, string operacao)
{
	// Check the precision of gps information to just use or calc the coordinates
	if((precision >> 3) & 0x01)
	{
		/*
		 * O CALCULO DE LATLONG E FEITO COMO NO EXEMPLO:
		 * Se tivermos uma sequencia de operacao 227477 por exemplo, cada byte deve ser covertido para
		 * decimal separadamento e entao multiplicado pelo sua grandeza em dezena. Por exemplo, 22 para
		 * deciaml e 34, vezes 65536 (equivalente a 10000 em hexa) e igual a 2228224, somado a 74
		 * convertido para decimal 116 * 256 (equivalente a 100) = 29696, somado a 77 em decimal 119.
		 * Dessa soma entao devem ser retirados grau, minuto e segundo da seguinte maneira:
		 * 	A soma do exemplo e a seguinte 2228224 + 29696 + 119 = 2258039. Entao:
		 * 	2258039/100000 = 22 graus
		 * 	2258039 - 2200000 = 58039
		 * 	58039/1000 = 58 minutos
		 * 	58039 - 58000 = 039
		 * 	039 /10 = 3.9 segundos
		 * Obtidos grau, minuto e segundo, entao a  conversao para o formato decimal de localizacao e feita:
		 * posicionamento = grau + (minuto/60) + (segundo/3600)
		*/

		double grau, minuto, segundo;
		unsigned int preConv;
		string latLong;
		preConv = ((unsigned int)((unsigned char)operacao.at(0)*65536)) +
					((unsigned int)((unsigned char)operacao.at(1)*256)) +
					((unsigned int)((unsigned char)operacao.at(2)));

		/*Dbg(TAG "%d %d %d %d %d %d %d %d %d %d %d %d ", hex, setw(2), setfill('0'), int((unsigned char)operacao.at(0),
					hex, setw(2), setfill('0'), int((unsigned char)operacao.at(1)),
					hex, setw(2), setfill('0'), int((unsigned char)operacao.at(2)));

		Dbg(TAG "Latlong %d", preConv);*/

		grau = preConv / 100000;

		Dbg(TAG "Latlong graus %d", grau);

		preConv -= grau * 100000;
		minuto = preConv/1000;

		Dbg(TAG "Latlong %d", preConv);
		Dbg(TAG "Latlong minutos %d", minuto);

		preConv -= minuto * 1000;
		segundo = ((double)preConv)/10.0;

		Dbg(TAG "Latlong %d", preConv);
		Dbg(TAG "Latlong segundos %d", segundo);
		Dbg(TAG "Latlong retorno %d", grau+(minuto/60.0)+(segundo/3600.0));

		return grau+(minuto/60.0)+(segundo/3600.0);
	}
	else
	{
		std::ostringstream grauStream;
		grauStream << std::hex << std::setfill('0');
		grauStream << std::setw(2) << (int)operacao.at(0);

		std::ostringstream minutoStream;
		minutoStream << std::hex << std::setfill('0');
		minutoStream << std::setw(2) << (int)operacao.at(1);

		std::ostringstream segundoStream;
		segundoStream << std::hex << std::setfill('0');
		segundoStream << std::setw(2) << (int)operacao.at(2);

		int grau = atoi(grauStream.str().c_str());
		int minuto = atoi(minutoStream.str().c_str());
		int segundo = atoi(segundoStream.str().c_str());

		return grau+(minuto/60.0)+(segundo/3600.0);
	}
}

double ParseLatitude(uint8_t precision, string operacao, int yAxis)
{
	double latitude = ParseLatLong(precision, operacao);

	if(yAxis)
	{
		return latitude * -1;
	}
	return latitude;
}

double ParseLongitude(uint8_t precision, string operacao, int xAxis, int complemento)
{
	double longitude = ParseLatLong(precision, operacao);

	if(complemento)
	{
		longitude += 100;
	}

	if(xAxis)
	{
		return longitude * -1;
	}

	return longitude;
}

void CreateFileNameProcessed(string *newPath, std::vector<std::string> tokens)
{
	for(std::vector<int>::size_type i = 1; i != tokens.size(); i++)
	{
		if(i == tokens.size() - 1)
			newPath->append("/p_").append(tokens[i]);
		else
			newPath->append("/").append(tokens[i]);
	}

	Info(TAG "Renamed processed file: %s", newPath->c_str());
}

double HaverSine(double lat1, double lon1, double lat2, double lon2)
{
	const double halfC = M_PI / 180;

	uint64_t r = 6371e3; // metres
	double teta1 = lat1 * halfC;
	double teta2 = lat2 * halfC;
	double deltaTeta = (lat2-lat1) * halfC;
	double deltaLambda = (lon2-lon1) * halfC;

	double a = sin(deltaTeta/2) * sin(deltaTeta/2) +
			cos(teta1) * cos(teta2) *
			sin(deltaLambda/2) * sin(deltaLambda/2);
	double c = 2 * atan2(sqrt(a), sqrt(1-a));

	double d = r * c;
	return d;
}

}} // namespace
