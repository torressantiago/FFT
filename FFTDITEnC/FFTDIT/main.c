#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <complex.h>

#define DATA_TYPE uint16_t
#define VECT_SIZE 8
#define NVAL VECT_SIZE
#define NUM_BITS 3



int Bit_Reverse(DATA_TYPE *in, DATA_TYPE *out);
uint16_t fftdit(complex *);

DATA_TYPE Signal[NVAL] = {1,-1,1,-1,1,-1,1,-1};

int main(){
    // BIT REVERSE
    DATA_TYPE SReverse[NVAL];
    complex SReverseComplex[NVAL], FFTVector[NVAL];
    (Bit_Reverse(&Signal[0], &SReverse[0]));
    // CONVERTIR DATOS A POLARES
    // COPIAR DATOS EN OTRO VECTOR PARA NO PERDERLO
    uint16_t i = 0;
    for(i = 0; i < NVAL; i++){
        SReverseComplex[i] = (double complex)SReverse[i];
        FFTVector[i] =  SReverseComplex[i];
    }
    int k = 0;
    for(k = 0; k<VECT_SIZE; k++){
        printf("SigIn[%d] = %.2f %+.2fi\n",k, creal(SReverseComplex[k]), cimag(SReverseComplex[k]));
    }
    printf("\n\n\n");
    // FFT
    fftdit(FFTVector);

    printf("\n\n\n");
    int p = 0;
    for(p = 0; p<VECT_SIZE; p++){
        printf("FFT[%d] = %.2f %+.2fi\n",p, creal(FFTVector[p]), cimag(FFTVector[p]));
    }
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

uint16_t fftdit(complex x[]){
    uint16_t Half = 1;
    uint16_t Stage = 1;
    uint16_t Butterfly = 0;
    uint16_t n = 0;
    uint16_t pos;

    double r;

    complex Wn, temp;

    for(Stage = 1; Stage <= NUM_BITS ; Stage++){
        printf("Stage %d Half %d \n", Stage, Half);
        for(Butterfly = 0; Butterfly < NVAL; Butterfly += pow(2,Stage)){
            printf("\t Butterfly %d \n", Butterfly);
            for(n = 0; n < Half; n++){
                printf("\t \t n %d \n", n);
                pos = n + Butterfly;
                printf("\t \t \t pos %d \n", pos);
                r = pow(2,(NUM_BITS - Stage)) * n;
                printf("\t \t \t r %0.2f \n", r);
                Wn = cexp(((I)*(2*M_PI)*r/VECT_SIZE));
                printf("\t \t \t Wn = %f %+fi\n", creal(Wn), cimag(Wn));
                printf("\t \t \t x[pos+Half] = %f %+fi\n", creal(x[pos+Half]), cimag(x[pos+Half]));
                printf("\t \t \t x[pos] = %f %+fi\n", creal(x[pos]), cimag(x[pos]));
                temp = (Wn * x[pos+Half]);
                printf("\t \t \t temp = %f %+fi\n", creal(temp), cimag(temp));
                x[pos]          = (x[pos] + temp); // OPERACIONES
                x[pos+Half]     = (x[pos] - temp);
            }
        }
        Half *= 2;
    }
    return 1;
}
