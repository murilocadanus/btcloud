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

	struct cache_cadastro {
		uint64_t esn;
		uint64_t id;
		uint64_t antena_int;
		std::string antena_text;
		uint64_t antena_tipo;
		uint32_t projeto;
		uint32_t clioid;
		uint32_t ger1;
		uint32_t ger2;
		uint32_t ger3;
		uint32_t connumero;
		uint32_t veioid;
		uint32_t tipo_contrato;
		uint32_t classe;
		uint32_t serial0;
		uint32_t serial1;
		uint32_t is_sasgc;
		uint32_t tipo_veiculo;
		uint32_t debug;
		std::string placa;
		std::string entradas;
		std::string saidas;
		std::vector<int32_t> sensores;
		std::vector<int32_t> atuadores;
		uint32_t rpm_maximo;
		std::string clincid;
		std::string clientName;
	};

	struct EquipContrato
	{
		uint64_t esn;
		uint32_t clioid;
		uint32_t gerenciadora;
		uint32_t connumero;
		uint32_t veioid;
		uint32_t tipo_contrato;
		uint32_t classe;
		uint32_t protocolo;
		uint32_t serial;
		uint32_t porta_panico;
		uint32_t porta_bloqueio;
		uint32_t id;
		uint32_t is_sasgc;
		uint32_t tipo_veiculo;
	};

	struct Telemetry
	{
		uint32_t trecho;
		uint32_t dataHora;
		double velocidade;
		uint32_t rpm;
		double odometro;
		double horimetro;
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
		string operacao;
	};

	struct OdoVel
	{
		uint32_t odometro;
		uint32_t velocidade;
	};

	struct EquipFlags
	{
		uint32_t bloqueio;
		uint32_t alerta_int;
		uint32_t somente_posicao;
		uint32_t entrada_ponto;
		uint32_t saida_ponto;
		uint32_t alerta_bat;
		uint32_t pad1;
		uint32_t periferico;
		uint32_t cod_periferico;
		uint32_t pad2;
		uint32_t viagem_aberta;
		uint32_t viagem_fechada;
		uint32_t fuga_cerca;
		uint32_t ponto_de_ref;
		uint32_t ignicao;
		uint32_t tem_mensagem;
		uint32_t anti_theft;
	};

	struct EquipPosicao
	{
		uint32_t datahora;
		uint32_t datachegada;
		uint32_t entrada;
		uint32_t saida;
		double lat2;
		double long2;
		OdoVel odo_vel_gps;
		uint32_t id_ponto;
		//t32_flags flags;
		//t32_csq_rot csq_id_rota;
		//t32_sai_cfg saida_cfg;
		//t32_ent_gpsi entrada_gps_info;
		uint32_t indice_posicao;
		uint32_t gps_altitude;
		//t32_inf_mot inf_motorista;
		//t32_horimetro info_horimetro;
		uint32_t tmp_ult_val;
		uint32_t ad1;
		uint32_t ad2;
		uint32_t ad3;
		uint32_t ad4;
		uint32_t estado_seguranca_saidas;
		uint32_t estado_seguranca_entradas;
		uint32_t vcc_alim;
		uint32_t id_area;
		Telemetry info_telemetria;
		uint32_t tipo;
		uint32_t origem;
		uint32_t ponto_ref;
		uint32_t dist_ref;
		uint32_t ang_ref;
		uint32_t rota;
		uint32_t operacao;
		EquipFlags eventoflag;
	};

	struct PacoteEnriquecido
	{
		EquipContrato equipContrato;
		EquipPosicao equipPosicao;
	};

	struct Bluetec400
	{
		PacoteEnriquecido pacoteEnriquecido;
		Telemetry telemetry;

		void Clear()
		{

		}
	};

}} // namespace

#endif // BLUETEC

