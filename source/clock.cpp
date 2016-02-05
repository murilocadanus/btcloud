#include "clock.h"

#include <time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <atomic>

clock::clock() {
}

clock::~clock() {
}

time_t clock::horaatual = 0;
std::atomic<int> clock::stat_posicoes(0);
int clock::stat_posicoes_seg = 0;
std::atomic<int> clock::posicoes_seg(0);

void clock::atualiza_hora()
{
//	info("THREAD[%s]", __METHOD_NAME__);

	prctl(PR_SET_NAME,(char *)"parser_clock");

	while(1) {
		horaatual = time(NULL);
		stat_posicoes_seg = posicoes_seg;
		sleep(1);
	}

}

void clock::posicao_processada() {
	stat_posicoes++;
	posicoes_seg++;
}
