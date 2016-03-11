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

#ifndef FILEMOD_H_
#define FILEMOD_H_
#include "IFile.h"

using namespace std;

namespace Bluetec {

/** \class FileMod
 *  \brief This class specializes the persistence of files at disk.
 */
class FileMod
{
	public:
		/** \brief FileMod - Default constructor. */
		FileMod();

		/** \brief ~FileMod - Default destructor. */
		virtual ~FileMod();

		/** \brief SetPath - Set default path.
		 *
		 * \param path string
		 * \return void
		 */
		void SetPath(string path);

		/** \brief SaveBufferFileVeioid - Save buffer file from veioid.
		 *
		 * \param veioid uint32_t
		 * \param bufferFile const char*
		 * \param sizeBufferFile int
		 * \param nameFile string
		 * \return void
		 */
		void SaveBufferFileVeioid(uint32_t veioid, const char *bufferFile, int sizeBufferFile, string nameFile);

		/** \brief GetBufferFileVeioId - Return file buffer from veioid.
		 *
		 * \param veioid uint32_t
		 * \param nameFile string
		 * \param bufferFile char*
		 * \param sizeBufferFile uint32_t&
		 * \return bool
		 */
		bool GetBufferFileVeioId(uint32_t veioid, string nameFile, char *bufferFile, uint32_t& sizeBufferFile);

		/** \brief DelFileVeioId - Remove file from veioid.
		 *
		 * \param veioid uint32_t
		 * \param nameFile string
		 * \return void
		 */
		void DelFileVeioId(uint32_t veioid, string nameFile);

		/** \brief RenameFile - Rename file at disk.
		 *
		 * \param pathFileOld string
		 * \param  pathFileNew string
		 * \return void
		 */
		void RenameFile(string pathFileOld, string pathFileNew);

		/** \brief RenameFile - Rename file at disk.
		 *
		 * \param pathFile string
		 * \param bufferFile const char*
		 * \param sizeBufferFile uint32_t
		 * \return void
		 */
		void SaveBufferFile(string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		/** \brief RenameFile - Get Buffer file.
		 *
		 * \param pathFile string
		 * \param bufferFile char*
		 * \param sizeBufferFile uint32_t&
		 * \return bool
		 */
		bool GetBufferFile(string pathFile, char *bufferFile, uint32_t& sizeBufferFile);

		/** \brief SetMod - Set the value to calculate veioid.
		 *
		 * \param mod uint32_t
		 * \return void
		 */
		void SetMod(uint32_t mod);

		/** \brief GetSizeFile - Get the size of a IFile.
		 *
		 * \param veioid uint32_t
		 * \param nameFile string
		 * \return uint32_t
		 */
		uint32_t GetSizeFile(uint32_t veioid, string nameFile);

		/** \brief GetSizeFile - Set the pointer to IFile.
		 *
		 * \param file IFile*
		 * \return void
		 */
		void setFile(IFile *file);

	private:
		IFile *file;
		uint32_t mod;
		string getIndice(uint32_t veioid);
		string getDirectory(uint32_t veioid);
};

} // namespace

#endif
