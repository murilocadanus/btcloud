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

namespace bluetec {

class FileMod
{
	public:
		FileMod();
		~FileMod();
		void setPath(std::string path);

		/*
		 * Save buffer file from veioid
		 */
		void saveBufferFileVeioid(uint32_t veioid, const char *bufferFile, int sizeBufferFile, std::string nameFile);

		/*
		 * Return file buffer from veioid
		 */
		bool getBufferFileVeioid(uint32_t veioid, std::string nameFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		 * Remove file from veioid
		 */
		void delFileVeioid(uint32_t veioid, std::string nameFile);

		/*
		 * Rename file at disk
		 */
		void renameFile(std::string pathFileOld, std::string pathFileNew);

		void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		 * Set the value to calculate veioid
		 */
		void setMod(uint32_t mod);

		/*
		 * Set the pointer to IFile
		 */
		uint32_t getSizeFile(uint32_t veioid, std::string nameFile);

		void setFile(IFile *file);

	private:
		IFile *file;
		uint32_t mod;
		std::string getIndice(uint32_t veioid);
		std::string getDirectory(uint32_t veioid);
};

} // namespace

#endif
