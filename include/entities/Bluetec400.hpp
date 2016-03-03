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

#ifndef BLUETEC
#define BLUETEC

using namespace std;

namespace Sascar { namespace Bluetec400 {

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

}} // namespace

#endif // BLUETEC

