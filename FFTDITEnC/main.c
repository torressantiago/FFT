#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "Num_Complejos.h"

#define DATA_TYPE uint16_t
#define VECT_SIZE 4096
#define NUM_BITS 12
#define MY_RAND_MAX 4000

#define NVAL 8
#define MAXSTAGE 3

int Bit_Reverse(DATA_TYPE *, DATA_TYPE *);
uint16_t fftdit(COMPLEX *);

DATA_TYPE Signal[NVAL] = {5,5,5,5,5,5,5,5};

int main(){

    // BIT REVERSE
    DATA_TYPE SReverse[NVAL];
    COMPLEX SReverseComplex[NVAL], FFTVector[NVAL];
    Bit_Reverse(&Signal[0], &SReverse[0]);
    uint16_t i = 0;
    // CONVERTIR DATOS A POLARES
    // COPIAR DATOS EN OTRO VECTOR PARA NO PERDERLO
    for(i = 0; i < NVAL; i++){
        SReverseComplex[i].x = SReverse[i];
        SReverseComplex[i].m = SReverse[i];
        SReverseComplex[i].y = 0;
        SReverseComplex[i].a = 0;

        FFTVector[i] =  SReverseComplex[i];

    }
    // FFT
    fftdit(&FFTVector[0]);
    return 0;
}

int Bit_Reverse(DATA_TYPE *in, DATA_TYPE *out){
    uint16_t vect_pos = 0, bit = 0;
    uint16_t res_index;

    for(vect_pos = 0; vect_pos<VECT_SIZE; vect_pos++){  // RECORRER POSICIONES DEL VECTOR
        res_index = 0;
        for(bit = 0; bit<NUM_BITS; bit++){              // RECORRER BITS DEL INDICE DE CADA POSICION DEL VECTOR
            if(vect_pos & (1<<bit))                     // PREGUNTAR SI EL BIT DEL INDICE ES 1
              res_index |= (1<<(NUM_BITS-bit-1));       // PONER EN 1 LA POSICION DEL BIT INVERTIDO EN EL INDICE
        }
        *(out+res_index) = *(in+vect_pos);
    }
    return 1;
}

uint16_t fftdit(COMPLEX *x){
    uint16_t Half = 1;
    uint16_t Stage = 1;
    uint16_t Butterfly = 0;
    uint16_t n = 0;
    uint16_t pos;

    double r;

    COMPLEX Wn, a, b;

    for(Stage = 1; Stage <= MAXSTAGE ; Stage++){
        for(Butterfly = 0; Butterfly < NVAL; Butterfly += pow(2,Stage)){
            for(n = 0; n < Half; n++){
                pos = n + Butterfly;
                r = pow(2,(MAXSTAGE - Stage)) * n;
                Wn = WnCalculator(r);
                *(x+pos)        = ComplexSuma(*(x+pos), ComplexMult(Wn, *(x+pos+Half)));
                *(x+pos+Half)   = ComplexResta(*(x+pos), ComplexMult(Wn, *(x+pos+Half)));
            }
        }
        Half *= 2;
    }
    return 1;
}
