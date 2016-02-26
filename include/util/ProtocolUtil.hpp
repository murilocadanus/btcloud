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

#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <sstream>
#include "entities/bluetec400.pb.h"

using namespace std;

namespace Sascar { namespace ProtocolUtil {

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

	void LapsoSetup(string lapso, struct sLapso& setup);
	void LapsoToTelemetria(pacote_posicao::t_telemetria_bluetec400 *tele, struct sLapso& lapso);
	string PersistableLapso(sLapso *l);
	int TamanhoLapsoExpansao(char expansao);
	int TamanhoLapsoExpansao(saidas *p);
	int TamanhoLapso(char controle, char expansao);
	int TamanhoLapso(saidas *p);
	int ParseBCDDecimal(unsigned char byte);
	string ParseBCDString(unsigned char byte);
	tm* ParseDataHora(string data);
	string ParseHora(string hora);
	double ParseOdometro(string odometro);
	double ParseHorimetro(string horimetro);
	double ParseLatLong(string operacao);
	double ParseLatitude(string operacao, int yAxis);
	double ParseLongitude(string operacao, int xAxis, int complemento);

}} // namespace

#endif // PARSER_HPP

