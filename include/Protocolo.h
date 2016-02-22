#ifndef __PROTOCOLO_H__

#define __PROTOCOLO_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <list>
#include <string>
#include "entities/pacote_posicao.pb.h"
#include "mongo/client/dbclient.h" // for the driver

using namespace std;

namespace Sascar {

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
	uint32_t classe ;
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
};

enum pacote_recebido {
	PACOTE_INVALIDO,
	PACOTE_ACK,
	PACOTE_NACK,
	PACOTE_DADOS,
	PACOTE_SETUP,
	PACOTE_RESTART, // restart troca de um grupo de comandos
	PACOTE_APRESENTACAO,
	PACOTE_RESTART_PARTE_COMANDO
};

#define PROTO_SAS401     0x0003
#define PROTO_VDO        0x0009
#define PROTO_MTC600     0x000d
#define PROTO_MXT150     0x000f
#define PROTO_VDOQB      0x0010
#define PROTO_MXT100     0x0011
#define PROTO_MTC550     0x0013
#define PROTO_MXT140     0x0014
#define PROTO_VDO_SBTEC  0x0017
#define PROTO_MXT150_SBTEC 0x0018

#define EQUIPAMENTO_BD_MTC           8
#define EQUIPAMENTO_BD_VDO          16
#define EQUIPAMENTO_BD_MTC600       20
#define EQUIPAMENTO_BD_MXT150       27
#define EQUIPAMENTO_BD_MXT100       29
#define EQUIPAMENTO_BD_RVS_QB       30
#define EQUIPAMENTO_BD_MTC550       46
#define EQUIPAMENTO_BD_MXT140       55
#define EQUIPAMENTO_BD_RVSSBTEC     38
#define EQUIPAMENTO_BD_MXT150SBTEC  33


class protocolo {

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

		protocolo();
		~protocolo();

	public:
		static int projeto2protocolo(uint32_t projeto);

		static string bufferToHex(const unsigned char *buffer, const int size);

		// algoritmos de crc
		static uint8_t calcula_crc_sum_8 (const unsigned char *data, int data_len);
		static uint16_t calcula_crc_16 (const unsigned char *data, int data_len);

		// metodos a serem implementados pelos subdiretorios
		static void setup();
		static int tamanho_pacote(char *buffer, int len);

		static int preenche_cadastro(pacote_posicao::equip_contrato *contrato, std::string chave, cache_cadastro &retorno);
		static void processa_pacote(const char *buffer, int len, mongo::DBClientConnection *dbClient);
		static void transmitir_dados_serializados(std::string data);
};


};

#endif
