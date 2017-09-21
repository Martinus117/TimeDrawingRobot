// Letters.h

#ifndef _LETTERS_h
#define _LETTERS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

struct LPoint
{
	float x, y;
	int upDown;  // 1=up, 0=down, -1=nix
};


#endif



