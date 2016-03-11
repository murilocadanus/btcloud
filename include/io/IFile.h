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

#ifndef IFILE_H_
#define IFILE_H_
#include <iostream>

using namespace std;

namespace Bluetec {

/** \class IFile
 *  \brief Interface that defines a File behavior.
 */
class IFile
{
	public:
		/** \class FileNotFoundException
		 *  \brief Specialized exception class.
		 */
		class FileNotFoundException{};

		/** \class PathNotFoundException
		 *  \brief Specialized exception class.
		 */
		class PathNotFoundException{};

		/** \class FileReadException
		 *  \brief Specialized exception class.
		 */
		class FileReadException{};

		/** \class FileWriteException
		 *  \brief Specialized exception class.
		 */
		class FileWriteException{};

		/** \class PathCreateException
		 *  \brief Specialized exception class.
		 */
		class PathCreateException{};

		/** \class PathNotDefinedException
		 *  \brief Specialized exception class.
		 */
		class PathNotDefinedException{};

		/** \class NotSpaceAvaiableException
		 *  \brief Specialized exception class.
		 */
		class NotSpaceAvaiableException{};

		/** \brief IFile - Default constructor. */
		IFile(){}

		/** \brief ~IFile - Default destructor. */
		virtual ~IFile(){}

		/** \brief SetPath - Set default path.
		 *
		 * \param path string
		 * \return void
		 */
		virtual void SetPath(string path) = 0;

		/** \brief SaveBufferFile - Save buffer at a file.
		 *
		 * \param pathFile string
		 * \param bufferFile const char*
		 * \param sizeBufferFile uint32_t
		 * \return void
		 */
		virtual void SaveBufferFile(string pathFile, const char *bufferFile, uint32_t sizeBufferFile) = 0;

		/** \brief GetBufferFile - Return file in a buffer.
		 *
		 * \param pathFile string
		 * \param bufferFile char*
		 * \param sizeBufferFile uint32_t&
		 * \return bool
		 */
		virtual bool GetBufferFile(string pathFile, char *bufferFile, uint32_t& sizeBufferFile) = 0;

		/** \brief DelFile - Remove file from disk.
		 *
		 * \param pathFile string
		 * \return void
		 */
		virtual void DelFile(string pathFile) = 0;

		/** \brief RenameFile - Rename file at disk.
		 *
		 * \param pathFileOld string
		 * \param pathFileNew string
		 * \return void
		 */
		virtual void RenameFile(string pathFileOld, string pathFileNew) = 0;

		/** \brief GetSizeFile - Return file size.
		 *
		 * \param veioid uint32_t
		 * \param nameFile string
		 * \return uint32_t
		 */
		virtual uint32_t GetSizeFile(uint32_t veioid, string nameFile) = 0;

		/** \brief CreateDir - Create a specified directory.
		 *
		 * \param directory string
		 * \return void
		 */
		virtual void CreateDir(string directory) = 0;
};

} // namespace

#endif // IFILE_H_
