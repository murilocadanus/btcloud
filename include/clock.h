#ifndef __CLOCK_H__

#define __CLOCK_H__

#include <time.h>
#include <atomic>

class clock {

 public:

  clock();
  ~clock();

  static time_t horaatual;

  static std::atomic<int> stat_posicoes;
  static int stat_posicoes_seg;
  static std::atomic<int> posicoes_seg;

  static void atualiza_hora();
  static void posicao_processada();
};

#endif
