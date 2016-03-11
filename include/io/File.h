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

#ifndef FILE_H_
#define FILE_H_
#include <iostream>
#include <stdint.h>
#include <dirent.h>
#include "IFile.h"

namespace bluetec {

class File : public IFile
{
	public:

		File();
		virtual ~File();

		/*
		 * Set default path
		 */
		virtual void setPath(std::string path);

		/*
		 * Save buffer at a file
		 */
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		/*
		 * Return file in a buffer
		 */
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);

		/*
		 * Remove file from disk
		 */
		virtual void delFile(std::string pathFile);

		/*
		 * Rename file at disk
		 */
		virtual void renameFile(std::string pathFileOld, std::string pathFileNew);

		/*
		 * Return file size
		 */
		virtual uint32_t getSizeFile(uint32_t veioid, std::string nameFile);

		virtual void createDir(std::string directory);

	private:
		std::string path;
		DIR *dirPath;
		int dirFd;


};

} /* namespace bluetec */

#endif /* FILE_H_ */
