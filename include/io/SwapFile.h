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

#ifndef SWAPFILE_H_
#define SWAPFILE_H_
#include "File.h"

namespace Bluetec {

/** \class SwapFile
 *  \brief Extension of File, this class specializes the persistence of files at disk.
 */
class SwapFile : public File
{
	public:
		SwapFile();
		virtual ~SwapFile();

		/*
		 * Salva o buffer em um arquivo de swap e posteriormente move para o arquivo original.
		 */
		virtual void SaveBufferFile(std::string pathFile, const char *bufferFile, uint32_t sizeBufferFile);

		/*
		 * Retorna o arquivo em um buffer.
		 */
		virtual bool GetBufferFile(std::string pathFile, char *bufferFile, uint32_t& sizeBufferFile);
};

} /* namespace bluetec */

#endif /* SWAPFILE_H_ */
