#include "BTCloudApp.hpp"
#include "clock.h"

int main(int argc, char **argv)
{
	std::thread relogio(clock::atualiza_hora);

	return I9Run<BTCloudApp>(argc, argv);
}
