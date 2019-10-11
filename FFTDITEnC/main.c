#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "Num_Complejos.h"

#define DATA_TYPE uint16_t
#define VECT_SIZE 8
#define NVAL VECT_SIZE
#define NUM_BITS 3



int Bit_Reverse(DATA_TYPE *in, DATA_TYPE *out);
uint16_t fftdit(COMPLEX *);

DATA_TYPE Signal[NVAL] = {5,5,5,5,5,5,5,5};

int main(){

    // BIT REVERSE
    DATA_TYPE SReverse[NVAL];
    COMPLEX SReverseComplex[NVAL], FFTVector[NVAL];
    (Bit_Reverse(&Signal[0], &SReverse[0]))? printf("EXITOS EN BIT REVERSE!\n") :printf("FATAL ERROR EN BIT REVERSE\n");
    int p;
    for(int p = 0; p<VECT_SIZE; p++){
        printf("%d \t", SReverse[p]);
    }
    printf ("\n");

    // CONVERTIR DATOS A POLARES
    // COPIAR DATOS EN OTRO VECTOR PARA NO PERDERLO
    uint16_t i = 0;
    for(i = 0; i < NVAL; i++){
        SReverseComplex[i].x = (double)SReverse[i];
        SReverseComplex[i].m = (double)SReverse[i];
        SReverseComplex[i].y = 0;
        SReverseComplex[i].a = 0;

        FFTVector[i] =  SReverseComplex[i];
    }
    for(int p = 0; p<VECT_SIZE; p++){
        printf("%f \t", FFTVector[p].x);
    }
    printf ("\n");
    // FFT
    ((fftdit(FFTVector))? printf("EXITOS EN FFT!\n") :printf("FATAL ERROR EN FFT\n"));
    printf ("Parte Real \n ");
    for(int p = 0; p<VECT_SIZE; p++){
        printf("%0.2f \t", FFTVector[p].x);
    }
    printf ("\n Parte Imaginaria \n ");
    /**/
    for(int p = 0; p<VECT_SIZE; p++){
        printf("%0.2f \t", FFTVector[p].y);
    }
    printf ("\n");

    //printf("%ff", SReverseComplex[0].x );
    //printf("%d\n", Signal[0]);

    return 0;
}

int Bit_Reverse(DATA_TYPE *in, DATA_TYPE *out){
    uint16_t vect_pos = 0, bit = 0;
    uint16_t res_index;
    //printf("EXITOS\n");
    for(vect_pos = 0; vect_pos<VECT_SIZE; vect_pos++){  // RECORRER POSICIONES DEL VECTOR
        res_index = 0;
        //printf("vect_pos %d \n", vect_pos);
        for(bit = 0; bit<NUM_BITS; bit++){              // RECORRER BITS DEL INDICE DE CADA POSICION DEL VECTOR
            //printf("bit %d \t \n", bit);
            if(vect_pos & (1<<bit))                     // PREGUNTAR SI EL BIT DEL INDICE ES 1
              res_index |= (1<<(NUM_BITS-bit-1));       // PONER EN 1 LA POSICION DEL BIT INVERTIDO EN EL INDICE
        }
        *(out+res_index) = *(in+vect_pos);
    }
    return 1;
}

uint16_t fftdit(COMPLEX x[]){
    uint16_t Half = 1;
    uint16_t Stage = 1;
    uint16_t Butterfly = 0;
    uint16_t n = 0;
    uint16_t pos;

    double r;

    COMPLEX Wn, a, b;

    for(Stage = 1; Stage <= NUM_BITS ; Stage++){
        printf("Stage %d Half %d \n", Stage, Half);
        for(Butterfly = 0; Butterfly < NVAL; Butterfly += pow(2,Stage)){
            printf("\t Butterfly%d \n", Butterfly);
            for(n = 0; n < Half; n++){
                printf("\t \t n %d \n", n);
                pos = n + Butterfly;
                printf("\t \t \t pos %d \n", pos);
                r = pow(2,(NUM_BITS - Stage)) * n;
                printf("\t \t \t r %0.2f \n", r);
                Wn = WnCalculator(r);
                printf("\t \t \t Wn %0.2f + %0.2fj \n", (float)Wn.x, (float)Wn.y);
                printf("\t \t \t a %0.3f + %0.3fj \n", (float)x[pos].x, (float)x[pos].y);
                printf("\t \t \t b %0.3f + %0.3fj \n", (float)x[pos+Half].x, (float)x[pos+Half].y);
                x[pos]          = ComplexSuma   (x[pos], ComplexMult(Wn, x[pos+Half])); // OPERACIONES
                x[pos+Half]     = ComplexResta  (x[pos], ComplexMult(Wn, x[pos+Half]));
                printf("\t \t \t aa %0.3f + %0.3fj \n", (float)x[pos].x, (float)x[pos].y);
                printf("\t \t \t bb %0.3f + %0.3fj \n", (float)x[pos+Half].x, (float)x[pos+Half].y);
            }
        }
        Half *= 2;
    }
    return 1;
}
