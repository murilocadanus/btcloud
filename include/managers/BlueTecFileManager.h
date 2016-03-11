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

#ifndef BLUETECFILEMANAGER_H_
#define BLUETECFILEMANAGER_H_

#include <iostream>
#include <stdint.h>
#include <vector>
#include "io/FileMod.h"
#include "io/SwapFile.h"

namespace Bluetec {

#define NAME_INDEX_FILE "file.idx"
#define SEQUENCE_TRECHO "sequence"
#define NAME_HFULL_FILE "HFULL"

enum enumDefaultValues
{
	LAPSO = 3,
	ACELX = 0,
	ACELY = 0,
	SPANACEL = 0
};

enum enumDataType
{
	HFULL = 1,
	HSYNC = 2,
	HSYNS = 8,
	HSYNS_DADOS = 3,
	HSYNS_FINAL = 4,
	FINAL = 5,
	DADOS = 6,
	DADOS_FINAL = 7
};

/** \class DataFile
 *  \brief This struct represents the file at disk.
 */
struct DataFile
{
	uint64_t id;
} __attribute__ ((packed)) ;

/** \class HeaderDataFile
 *  \brief This struct has a file and header data.
 */
struct HeaderDataFile
{
	uint16_t file;
	uint32_t beginPointer;
	uint32_t endPointer;
	uint32_t timestamp;
	uint8_t  dataType;
	uint32_t idTrecho;
	DataFile headerFile;

	HeaderDataFile()
	{
		file = 0;
		beginPointer = 0;
		endPointer = 0;
		timestamp = 0;
		dataType = 0;
		idTrecho = 0;
		headerFile.id = 0;
	}
} __attribute__ ((packed)) ;

/** \class HFull
 *  \brief This entity has a contract information.
 */
struct HFull
{
	uint8_t lapso;
	uint64_t idVeiculo;
	uint16_t versaoFirmware;
	uint8_t versaoHardware;
	uint8_t configHardware;
	uint8_t reservado[5];
	uint16_t constanteVelocidade2;
	uint16_t constanteVelocidade1;
	uint16_t constanteRPM;
	uint8_t confDiversas;
	uint8_t reservado1[8];
	uint8_t acelx;
	uint8_t reservado2[3];
	uint8_t acely;
	uint8_t tipoAcelerometro;
	uint8_t limiteAcelFrenTom[3];
	uint8_t spanAcel;
	uint8_t reservado3[3];
	uint8_t verConsistenciaSetup;
	uint8_t confHardwareRe;
	uint16_t limiteAnalogico1;
	uint8_t limiteAnalogico234[3];
	uint32_t lapsosGravacaoEvento;
	uint8_t reservado4[8];
	uint8_t volume;
	uint16_t parametrosGPS;
	uint16_t ajusteHoraPorGps;
	uint16_t parametrosGerais;
	uint8_t reservado5[7];
	uint8_t alarmes[58];
	uint8_t reservado6[6];
	uint8_t gerenciaMemoria1;
	uint8_t gerenciaMemoria2;
	uint8_t	reversao;
	uint8_t inicioHorarioVerao[3];
	uint8_t reservado7[9];
	uint8_t listaBloqueioOuPermissao[65];
	uint8_t reservado8[18];
	uint8_t numeroSerial[3];
	uint8_t reservado9;
	uint8_t tipoTeclado;
	uint8_t finalHorarioVerao[3];
	uint8_t reservado10;
	uint8_t hfull[5];
} __attribute__ ((packed)) ;

/** \class BlueTecFileManager
 *  \brief Manager class of Bluetec files.
 */
class BlueTecFileManager
{
	public:
		BlueTecFileManager();
		BlueTecFileManager(std::string path);
		virtual ~BlueTecFileManager();
		virtual void setPath(std::string path);

		/* se pointer = 0 pesquisar o ultimo ponteiro indendente do "file". */
		/*
		 * Retorna o buffer para um determinado arquivo.
		 */
		virtual bool getBufferFile(uint32_t veioid, uint32_t pointer, uint16_t file, char *bufferFile, uint32_t& sizeBufferFile, struct HeaderDataFile& bluetecHeaderFile);

		/*
		 * Salva o buffer em um determinado arquivo.
		 */
		virtual void saveBufferFile(uint32_t veioid, const char *bufferFile, uint32_t sizeBufferFile, struct HeaderDataFile& bluetecHeaderFile);

		/*
		 * Rename file at disk
		 */
		void renameFile(std::string pathFileOld, std::string pathFileNew);

		/*
		 * Remove um arquivo fisicamente.
		 */
		virtual void delFile(uint32_t veioid, DataFile file);

		/*
		 * Retorna um novo id de trecho.
		 */
		virtual uint32_t getNextIdTrecho();

		/*
		 * Retorna o HFUll de um equipamento especifico.
		 */
		virtual bool getHfull(uint32_t veioid, HFull& hfull);

		/*
		 * Salva salva o hfull do equipamento especifico.
		 */
		virtual void saveHfull(uint32_t veioid, HFull& hfull);


	private:
		/* Para garantir uma copia do arquivo... */
		SwapFile swap;
		FileMod files;
		virtual bool getsBluetecHeaderFile(uint32_t veioid, uint32_t pointer, uint16_t file, struct HeaderDataFile& bluetecHeaderFile);
		virtual std::vector<struct HeaderDataFile*> *getListsBluetecHeaderFile(uint32_t veioid);
		virtual void saveIndexFile(uint32_t veioid, std::vector<struct HeaderDataFile*> *listBluetecHeaderFile);
		virtual void deleteIndexFileObject(std::vector<struct HeaderDataFile*> *listBluetecHeaderFile);

		/*
		 * Libera espaço em disco utilizado pelos registros que estão armazenadas a mais de n dias.
		 */
		virtual void vacuum(uint32_t veioid, std::vector<struct HeaderDataFile*> *listBluetecHeaderFile);
};

} // namespace

#endif // BLUETECFILEMANAGER_H_
