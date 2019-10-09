#ifndef NUM_COMPLEX
#define NUM_COMPLEX

#include <math.h>

#ifndef NVAL
#define NVAL 8
#endif

typedef struct {
	double x,y; // Coordenadas cartesianas
	double m,a;	// Coordenas polares
}COMPLEX;

COMPLEX ConversionCart2Pol	(COMPLEX);

COMPLEX ConversionPol2Cart	(COMPLEX);

COMPLEX ComplexSuma			(COMPLEX, COMPLEX);

COMPLEX ComplexResta		(COMPLEX, COMPLEX);

COMPLEX ComplexMult			(COMPLEX, COMPLEX);

COMPLEX WnCalculator		(double);

#endif
