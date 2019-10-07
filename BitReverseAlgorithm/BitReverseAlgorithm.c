#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define DATA_TYPE uint16_t
#define VECT_SIZE 4096
#define NUM_BITS 12
#define MY_RAND_MAX 4000

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

int main()
{
    DATA_TYPE in_vect[VECT_SIZE], out_vect[VECT_SIZE], i;

    for(i = 0; i<VECT_SIZE; i++){
        in_vect[i] = rand() % MY_RAND_MAX;
    }

   (Bit_Reverse(&in_vect[0], &out_vect[0]))? printf("SUCCESS!\n"): printf("FAILURE\n") ;

    printf("\t \t Entrada \t Salida \n");
   for(i = 0; i<VECT_SIZE; i++){
    printf("%d \t \t %d \t \t %d \n",i, in_vect[i], out_vect[i]);
   }
}
