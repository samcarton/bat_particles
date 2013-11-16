#include "bat_rand.h"

static unsigned long g_batRandSeed = 2463534242;

unsigned long bat_rand()
{
	// intel fastrand
	g_batRandSeed = (214013*g_batRandSeed+2531011);
	return (g_batRandSeed>>16)&0x7FFF;

	// alternative, larger
	/*
	g_batRandSeed^=(g_batRandSeed<<13);
  	g_batRandSeed^=(g_batRandSeed>>17);
  	return (g_batRandSeed^=(g_batRandSeed<<5));
  	*/
}

void bat_srand(unsigned long seed)
{
	g_batRandSeed = seed;
}