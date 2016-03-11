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

namespace bluetec {

class IFile
{
	public:
		class FileNotFoundException{};
		class PathNotFoundException{};
		class FileReadException{};
		class FileWriteException{};
		class PathCreateException{};
		class PathNotDefinedException{};
		class NotSpaceAvaiableException{};
		virtual ~IFile(){};
		virtual void setPath(std::string path) = 0;
		virtual void saveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile) =0;
		virtual bool getBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile) = 0;
		virtual void delFile(std::string pathFile) = 0;
		virtual void renameFile(std::string pathFileOld, std::string pathFileNew) = 0;
		virtual uint32_t getSizeFile(uint32_t veioid, std::string nameFile) = 0;
		virtual void createDir(std::string directory) = 0;
};

} /* namespace bluetec */

#endif /* IFILE_H_ */
