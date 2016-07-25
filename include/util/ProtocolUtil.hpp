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
#include "entities/ProtocolEntity.hpp"
#include "mongo/client/dbclient.h" // for the driver

using namespace std;

namespace BTCloud { namespace Util {

	/** \class Output
	 *  \brief This entity contains output data signals.
	 */
	typedef struct Output
	{
			unsigned int saida0 :1;
			unsigned int saida1 :1;
			unsigned int saida2 :1;
			unsigned int saida3 :1;
			unsigned int saida4 :1;
			unsigned int saida5 :1;
			unsigned int saida6 :1;
			unsigned int saida7 :1;
	}__attribute__ ((packed)) Output;


	/** \class Lapse
	 *  \brief This entity contains lapse data.
	 */
	typedef struct Lapse
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
			std::string lastIncompleteLapse;
	} Lapse;

	/** \brief LapsoSetup - .
	 *
	 * \param lapse string
	 * \param setup struct Lapse&
	 * \return void
	 */
	void LapsoSetup(string lapso, struct Lapse& setup);

	/** \brief LapsoToTelemetry - .
	 *
	 * \param tele Telemetry*
	 * \param lapso struct Lapse&
	 * \return void
	 */
	void LapsoToTelemetry(Entities::Telemetry *tele, struct Lapse& lapso);

	/** \brief PersistableLapse - .
	 *
	 * \param l Lapse*
	 * \return string
	 */
	string PersistableLapse(Lapse *l);

	/** \brief ExpasionSize - .
	 *
	 * \param expansao char
	 * \return int
	 */
	int ExpasionSize(char expansao);

	/** \brief ExpasionSize - .
	 *
	 * \param p Output*
	 * \return int
	 */
	int ExpasionSize(Output *p);

	/** \brief LapseSize - .
	 *
	 * \param controle char
	 * \param expansao char
	 * \return int
	 */
	int LapseSize(char controle, char expansao);

	/** \brief LapseSize - .
	 *
	 * \param p Output*
	 * \return int
	 */
	int LapseSize(Output *p);

	/** \brief ParseBCDDecimal - .
	 *
	 * \param byte unsigned char
	 * \return int
	 */
	int ParseBCDDecimal(unsigned char byte);

	/** \brief ParseBCDString - .
	 *
	 * \param byte unsigned char
	 * \return string
	 */
	string ParseBCDString(unsigned char byte);

	/** \brief ParseDataHora - .
	 *
	 * \param data string
	 * \return tm*
	 */
	tm* ParseTimeDate(string data);

	/** \brief ParseTime - .
	 *
	 * \param hora string
	 * \return string
	 */
	string ParseTime(string hora);

	/** \brief ParseHodometer - .
	 *
	 * \param odometro string
	 * \return double
	 */
	double ParseHodometer(string odometro);

	/** \brief ParseHourmeter - .
	 *
	 * \param horimetro string
	 * \return double
	 */
	double ParseHourmeter(string horimetro);

	/** \brief ParseLatLong - .
	 *
	 * \param operacao string
	 * \return double
	 */
	double ParseLatLong(uint8_t precision, string operacao);

	/** \brief ParseLatitude - .
	 *
	 * \param precision uint8_t
	 * \param operacao string
	 * \param yAxis int
	 * \return double
	 */
	double ParseLatitude(uint8_t precision, string operacao, int yAxis);

	/** \brief ParseLongitude - .
	 *
	 * \param operacao string
	 * \param xAxis int
	 * \param complemento int
	 * \return double
	 */
	double ParseLongitude(uint8_t precision, string operacao, int xAxis, int complemento);

	/** \brief CreateFileNameProcessed - .
	 *
	 * \param newPath string*
	 * \param xtokens vector<string>
	 * \return double
	 */
	void CreateFileNameProcessed(string *newPath, vector<string> tokens);

	/** \brief HaverSine - .
	 *
	 * \param lat1 double
	 * \param lon1 double
	 * \param lat2 double
	 * \param lon2 double
	 * \return double
	 */
	double HaverSine(double lat1, double lon1, double lat2, double lon2);

}} // namespace

#endif // PARSER_HPP

