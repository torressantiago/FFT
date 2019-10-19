/**
 * \file  mcaspPlayBk.c
 *
 * \brief Sample application for McASP. This application loops back the input
 *        at LINE_IN of the EVM to the LINE_OUT of the EVM.
 */

/*
* Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "edma_event.h"
#include "interrupt.h"
#include "soc_C6748.h"
#include "hw_syscfg0_C6748.h"
#include "lcdkC6748.h"
#include "codecif.h"
#include "mcasp.h"
#include "aic31.h"
#include "edma.h"
#include "psc.h"
#include "gpio.h"
#include "hw_types.h"

#include "hw_psc_C6748.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/******************************************************************************
**                    EXTERNAL MACRO DEFINITIONS
******************************************************************************/
#define ModoFFT          0
#define ModoUART         1
#define IDLE             2
#define BUFF_COPY        3

/**------------------UART------------------**/
#define BAUD_128000                         128000*12
#define BAUD_9600                           9600*12


/**------------------GPIO------------------**/
#define INT_CHANNEL_GPIOB                   (3u)
#define bankNum                             (2u)
#define GPIOBank6Pin12_DSP                  (109u)
#define GPIOBank2Pin4_DSP                   (37u)
#define GPIO_INT_TYPE_SELECTED              GPIO_INT_TYPE_FALLEDGE

#define PINMUX6_GPIO2_4_ENABLE    (SYSCFG_PINMUX6_PINMUX6_15_12_GPIO2_4  << \
                                   SYSCFG_PINMUX6_PINMUX6_15_12_SHIFT)


/******************************************************************************
**                    INTERNAL MACRO DEFINITIONS - MCASP
******************************************************************************/
/*
** Values which are configurable
*/
/* Slot size to send/receive data */
#define SLOT_SIZE                             (16u)

/* Word size to send/receive data. Word size <= Slot size */
#define WORD_SIZE                             (16u)

/* Sampling Rate which will be used by both transmit and receive sections */
#define SAMPLING_RATE                         (48000u)

/* Number of channels, L & R */
#define NUM_I2S_CHANNELS                      (2u)

/* Number of samples to be used per audio buffer --------------------------------------------------------------------------------------------------*/
#define NUM_SAMPLES_PER_AUDIO_BUF             (2048u)//(1500u)//(2000u)

/* Number of buffers used per tx/rx */
#define NUM_BUF                               (3u)

/* Number of linked parameter set used per tx/rx */
#define NUM_PAR                               (2u)

/* Specify where the parameter set starting is */
#define PAR_ID_START                          (40u)

/* Number of samples in loop buffer */
#define NUM_SAMPLES_LOOP_BUF                  (10u)

/* AIC3106 codec address */
#define I2C_SLAVE_CODEC_AIC31                 (0x18u)

/* Interrupt channels to map in AINTC */
#define INT_CHANNEL_I2C                       (2u)
#define INT_CHANNEL_MCASP                     (2u)
#define INT_CHANNEL_EDMACC                    (2u)

/* McASP Serializer for Receive */
#define MCASP_XSER_RX                         (14u)

/* McASP Serializer for Transmit */
#define MCASP_XSER_TX                         (13u)

/*
** Below Macros are calculated based on the above inputs
*/
#define NUM_TX_SERIALIZERS                    ((NUM_I2S_CHANNELS >> 1) \
                                               + (NUM_I2S_CHANNELS & 0x01))
#define NUM_RX_SERIALIZERS                    ((NUM_I2S_CHANNELS >> 1) \
                                               + (NUM_I2S_CHANNELS & 0x01))
#define I2S_SLOTS                             ((1 << NUM_I2S_CHANNELS) - 1)

#define BYTES_PER_SAMPLE                      ((WORD_SIZE >> 3) \
                                               * NUM_I2S_CHANNELS)

#define AUDIO_BUF_SIZE                        (NUM_SAMPLES_PER_AUDIO_BUF \
                                               * BYTES_PER_SAMPLE)

#define TX_DMA_INT_ENABLE                     (EDMA3CC_OPT_TCC_SET(1) | (1 \
                                               << EDMA3CC_OPT_TCINTEN_SHIFT))
#define RX_DMA_INT_ENABLE                     (EDMA3CC_OPT_TCC_SET(0) | (1 \
                                               << EDMA3CC_OPT_TCINTEN_SHIFT))

#define PAR_RX_START                          (PAR_ID_START)
#define PAR_TX_START                          (PAR_RX_START + NUM_PAR)

/*
** Definitions which are not configurable
*/
#define SIZE_PARAMSET                         (32u)
#define OPT_FIFO_WIDTH                        (0x02 << 8u)

/****************************************************************************/
/*                      LOCAL FUNCTION PROTOTYPES                           */
/****************************************************************************/
/**------------------UART------------------**/

void UARTConfigTx(void);
void UARTTxEnable (unsigned int baseAdd);
void UARTFIFOTxEnable (unsigned int baseAdd);

void UARTSend (unsigned int length);

/**------------------GPIO------------------**/


static void SetupInt(void);
static void ConfigureIntGPIO_AINTC(void);
static void ConfigureIntGPIO(void);
static void ButtonISR(void);

void GPIOConfig(void);
void GPIOBank2Pin4PinMuxSetup(void);

void GPIOPinMuxSetup_Switches(void);
void SwitchAsInputs(void);

/**------------------EDMA3------------------**/
void EDMA3ChannelConfig(void);
void EDMA_MCASP_PSC(void);

/**------------------MCASP------------------**/
void MCASPI2SConfig(void);

/**------------------FFT------------------**/

void DFT_DT(void);

/******************************************************************************
**                  INTERNAL FUNCTION PROTOTYPES - MCASP
******************************************************************************/

static void EDMA3IntSetup(void);
static void McASPErrorIsr(void);
static void McASPErrorIntSetup(void);
static void AIC31I2SConfigure(void);
static void McASPI2SConfigure(void);
static void McASPTxDMAComplHandler(void);
static void McASPRxDMAComplHandler(void);
static void EDMA3CCComplIsr(void);
static void I2SDataTxRxActivate(void);
static void I2SDMAParamInit(void);
static void ParamTxLoopJobSet(unsigned short parId);
static void BufferTxDMAActivate(unsigned int txBuf, unsigned short numSamples,
                                unsigned short parToUpdate,
                                unsigned short linkAddr);
static void BufferRxDMAActivate(unsigned int rxBuf, unsigned short parId,
                                unsigned short parLink);

/****************************************************************************/
/*                      GLOBAL VARIABLES                                    */
/****************************************************************************/

unsigned int Modo = 0; //0: FFT, 1: UART

/**------------------UART------------------**/
char txArray[] = {"Esto si funciona\n"};
short txArrayN[1024];

short S[1024] = {512,519,525,532,539,545,552,559,566,572,579,585,592,599,605,612,618,625,632,638,645,651,657,664,670,677,683,689,695,702,708,714,720,726,732,738,744,750,756,762,768,774,780,785,791,796,802,807,813,818,824,829,834,839,845,850,855,860,864,869,874,879,883,888,892,897,901,906,910,914,918,922,926,930,934,938,941,945,949,952,955,959,962,965,968,971,974,977,980,982,985,988,990,992,995,997,999,1001,1003,1005,1007,1008,1010,1011,1013,1014,1015,1017,1018,1019,1020,1020,1021,1022,1022,1023,1023,1024,1024,1024,1024,1024,1024,1024,1023,1023,1022,1022,1021,1020,1020,1019,1018,1017,1015,1014,1013,1011,1010,1008,1007,1005,1003,1001,999,997,995,992,990,988,985,982,980,977,974,971,968,965,962,959,955,952,949,945,941,938,934,930,926,922,918,914,910,906,901,897,892,888,883,879,874,869,864,860,855,850,845,839,834,829,824,818,813,807,802,796,791,785,780,774,768,762,756,750,744,738,732,726,720,714,708,702,695,689,683,677,670,664,657,651,645,638,632,625,618,612,605,599,592,585,579,572,566,559,552,545,539,532,525,519,512,505,499,492,485,479,472,465,458,452,445,439,432,425,419,412,406,399,392,386,379,373,367,360,354,347,341,335,329,322,316,310,304,298,292,286,280,274,268,262,256,250,244,239,233,228,222,217,211,206,200,195,190,185,179,174,169,164,160,155,150,145,141,136,132,127,123,118,114,110,106,102,98,94,90,86,83,79,75,72,69,65,62,59,56,53,50,47,44,42,39,36,34,32,29,27,25,23,21,19,17,16,14,13,11,10,9,7,6,5,4,4,3,2,2,1,1,0,0,0,0,0,0,0,1,1,2,2,3,4,4,5,6,7,9,10,11,13,14,16,17,19,21,23,25,27,29,32,34,36,39,42,44,47,50,53,56,59,62,65,69,72,75,79,83,86,90,94,98,102,106,110,114,118,123,127,132,136,141,145,150,155,160,164,169,174,179,185,190,195,200,206,211,217,222,228,233,239,244,250,256,262,268,274,280,286,292,298,304,310,316,322,329,335,341,347,354,360,367,373,379,386,392,399,406,412,419,425,432,439,445,452,458,465,472,479,485,492,499,505,512,519,525,532,539,545,552,559,566,572,579,585,592,599,605,612,618,625,632,638,645,651,657,664,670,677,683,689,695,702,708,714,720,726,732,738,744,750,756,762,768,774,780,785,791,796,802,807,813,818,824,829,834,839,845,850,855,860,864,869,874,879,883,888,892,897,901,906,910,914,918,922,926,930,934,938,941,945,949,952,955,959,962,965,968,971,974,977,980,982,985,988,990,992,995,997,999,1001,1003,1005,1007,1008,1010,1011,1013,1014,1015,1017,1018,1019,1020,1020,1021,1022,1022,1023,1023,1024,1024,1024,1024,1024,1024,1024,1023,1023,1022,1022,1021,1020,1020,1019,1018,1017,1015,1014,1013,1011,1010,1008,1007,1005,1003,1001,999,997,995,992,990,988,985,982,980,977,974,971,968,965,962,959,955,952,949,945,941,938,934,930,926,922,918,914,910,906,901,897,892,888,883,879,874,869,864,860,855,850,845,839,834,829,824,818,813,807,802,796,791,785,780,774,768,762,756,750,744,738,732,726,720,714,708,702,695,689,683,677,670,664,657,651,645,638,632,625,618,612,605,599,592,585,579,572,566,559,552,545,539,532,525,519,512,505,499,492,485,479,472,465,458,452,445,439,432,425,419,412,406,399,392,386,379,373,367,360,354,347,341,335,329,322,316,310,304,298,292,286,280,274,268,262,256,250,244,239,233,228,222,217,211,206,200,195,190,185,179,174,169,164,160,155,150,145,141,136,132,127,123,118,114,110,106,102,98,94,90,86,83,79,75,72,69,65,62,59,56,53,50,47,44,42,39,36,34,32,29,27,25,23,21,19,17,16,14,13,11,10,9,7,6,5,4,4,3,2,2,1,1,0,0,0,0,0,0,0,1,1,2,2,3,4,4,5,6,7,9,10,11,13,14,16,17,19,21,23,25,27,29,32,34,36,39,42,44,47,50,53,56,59,62,65,69,72,75,79,83,86,90,94,98,102,106,110,114,118,123,127,132,136,141,145,150,155,160,164,169,174,179,185,190,195,200,206,211,217,222,228,233,239,244,250,256,262,268,274,280,286,292,298,304,310,316,322,329,335,341,347,354,360,367,373,379,386,392,399,406,412,419,425,432,439,445,452,458,465,472,479,485,492,499,505,512,519,525,532,539,545,552,559,566,572,579,585,592,599,605,612,618,625,632,638,645,651,657,664,670,677,683,689,695,702,708,714,720,726,732,738,744,750,756,762,768,774,780,785,791,796,802,807,813,818,824,829,834,839,845,850,855,860,864,869,874,879,883,888};
unsigned int N = 1024;
char ASCII[1024];
short txdata;
unsigned int Space;
unsigned int A = 0;
unsigned int length = 0;
char txdataH;
char txdataL;
volatile int InitFFT =0;
#include <complex.h>

#define VECT_SIZE 1024
#define NVAL VECT_SIZE
#define NUM_BITS 10
short Reversed[NVAL];
double complex FFTReversed[NVAL];

/**------------------GPIO------------------**/
char NAH = '\n';

/**------------------CTRL------------------**/
unsigned int CtrlFFT = 0;
unsigned int CtrlUART = 0;


int Bit_Reverse(short *in, short *out);
int fftdit(complex *);

/******************************************************************************
**                  INTERNAL VARIABLE DEFINITIONS - MCASP
******************************************************************************/


static unsigned char loopBuf[NUM_SAMPLES_LOOP_BUF * BYTES_PER_SAMPLE] = {0};

/*
** Transmit buffers. If any new buffer is to be added, define it here and
** update the NUM_BUF.
*/
static unsigned char txBuf0[AUDIO_BUF_SIZE];
static unsigned char txBuf1[AUDIO_BUF_SIZE];
static unsigned char txBuf2[AUDIO_BUF_SIZE];

/*
** Receive buffers. If any new buffer is to be added, define it here and
** update the NUM_BUF.
*/
static unsigned char rxBuf0[AUDIO_BUF_SIZE];
static unsigned char rxBuf1[AUDIO_BUF_SIZE];
static unsigned char rxBuf2[AUDIO_BUF_SIZE];

/*
** Receive TEMPORAL buffers for making operations and changing data
*/
static short tempBuf[(AUDIO_BUF_SIZE)/2];
/*
** Next buffer to receive data. The data will be received in this buffer.
*/
static volatile unsigned int nxtBufToRcv = 0;
/*
** The RX buffer which filled latest.
*/
static volatile unsigned int lastFullRxBuf = 0;
/*
** The offset of the paRAM ID, from the starting of the paRAM set.
*/
static volatile unsigned short parOffRcvd = 0;

/*
** The offset of the paRAM ID sent, from starting of the paRAM set.
*/
static volatile unsigned short parOffSent = 0;

/*
** The offset of the paRAM ID to be sent next, from starting of the paRAM set.
*/
static volatile unsigned short parOffTxToSend = 0;

/*
** The transmit buffer which was sent last.
*/
static volatile unsigned int lastSentTxBuf = NUM_BUF - 1;

/******************************************************************************
**                      INTERNAL CONSTATNT DEFINITIONS
******************************************************************************/
/* Array of receive buffer pointers */
static unsigned int const rxBufPtr[NUM_BUF] =
       {
           (unsigned int) rxBuf0,
           (unsigned int) rxBuf1,
           (unsigned int) rxBuf2
       };

/* Array of transmit buffer pointers */
static unsigned int const txBufPtr[NUM_BUF] =
       {
           (unsigned int) txBuf0,
           (unsigned int) txBuf1,
           (unsigned int) txBuf2
       };

/*
** Default paRAM for Transmit section. This will be transmitting from
** a loop buffer.
*/
static struct EDMA3CCPaRAMEntry const txDefaultPar =
       {
           (unsigned int)(EDMA3CC_OPT_DAM  | (0x02 << 8u)), /* Opt field */
           (unsigned int)loopBuf, /* source address */
           (unsigned short)(BYTES_PER_SAMPLE), /* aCnt */
           (unsigned short)(NUM_SAMPLES_LOOP_BUF), /* bCnt */
           (unsigned int) SOC_MCASP_0_DATA_REGS, /* dest address */
           (short) (BYTES_PER_SAMPLE), /* source bIdx */
           (short)(0), /* dest bIdx */
           (unsigned short)(PAR_TX_START * SIZE_PARAMSET), /* link address */
           (unsigned short)(0), /* bCnt reload value */
           (short)(0), /* source cIdx */
           (short)(0), /* dest cIdx */
           (unsigned short)1 /* cCnt */
       };

/*
** Default paRAM for Receive section.
*/
static struct EDMA3CCPaRAMEntry const rxDefaultPar =
       {
           (unsigned int)(EDMA3CC_OPT_SAM  | (0x02 << 8u)), /* Opt field */
           (unsigned int)SOC_MCASP_0_DATA_REGS, /* source address */
           (unsigned short)(BYTES_PER_SAMPLE), /* aCnt */
           (unsigned short)(1), /* bCnt */
           (unsigned int)rxBuf0, /* dest address */
           (short) (0), /* source bIdx */
           (short)(BYTES_PER_SAMPLE), /* dest bIdx */
           (unsigned short)(PAR_RX_START * SIZE_PARAMSET), /* link address */
           (unsigned short)(0), /* bCnt reload value */
           (short)(0), /* source cIdx */
           (short)(0), /* dest cIdx */
           (unsigned short)1 /* cCnt */
       };



/*
** The main function. Application starts here.
*/

int main(void)
{
    unsigned short parToSend;
    unsigned short parToLink;

    /* Set up pin mux for I2C module 0 */
    GPIOPinWrite(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_PIN_LOW);
    I2CPinMuxSetup(0);
    McASPPinMuxSetup();

    EDMA_MCASP_PSC();

    GPIOConfig();
    UARTConfigTx();
    SetupInt();
    ConfigureIntGPIO_AINTC();
    ConfigureIntGPIO();

    EDMA3ChannelConfig();
    MCASPI2SConfig();
    Modo = ModoFFT;


    /*
    ** Loop forever. if a new buffer is received, the lastFullRxBuf will be
    ** updated in the rx completion ISR. if it is not the lastSentTxBuf,
    ** buffer is to be sent. This has to be mapped to proper paRAM set.
    */
    while(1)
    {
        switch(Modo){
            case ModoFFT:{
                if(lastFullRxBuf != lastSentTxBuf){
                    /*
                    ** Start the transmission from the link paramset. The param set
                    ** 1 will be linked to param set at PAR_TX_START. So do not
                    ** update paRAM set1.
                    */
                    parToSend =  PAR_TX_START + (parOffTxToSend % NUM_PAR);
                    parOffTxToSend = (parOffTxToSend + 1) % NUM_PAR;
                    parToLink  = PAR_TX_START + parOffTxToSend;

                    lastSentTxBuf = (lastSentTxBuf + 1) % NUM_BUF;

                    memcpy((void *)tempBuf,(void *)rxBufPtr[lastFullRxBuf],AUDIO_BUF_SIZE);
                    //DFT_DT();

                    // FFT
                 }
            }
            break;
            case ModoUART: {

                UARTSend(N);
                GPIOPinWrite(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_PIN_HIGH);
            }
            break;

            case IDLE:
                break;

            case BUFF_COPY:{
                unsigned int cont;
                for(cont=0;cont<N;cont+=1){
                   txArrayN[cont] = tempBuf[4*cont];
                }

                Bit_Reverse(&txArrayN[0],&Reversed[0]);
                for(cont=0;cont<N;cont+=1){
                   FFTReversed[cont] = (double complex)Reversed[cont];
                }
                fftdit(&FFTReversed[0]);

                Modo = ModoUART;
            }
            break;
        }
    }
}

/******************************************************************************
**                          FUNCTION DEFINITIONS
******************************************************************************/

int Bit_Reverse(short *in, short *out){
    unsigned short vect_pos = 0, bit = 0;
    unsigned short res_index;
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
///---------------------------------------------------------------------------------------------------------
int fftdit(complex *x){
    unsigned short Half = 1;
    unsigned short Stage = 1;
    unsigned short Butterfly = 0;
    unsigned short n = 0;
    unsigned short pos;

    double r;

    complex Wn, temp;

    for(Stage = 1; Stage <= NUM_BITS ; Stage++){
        for(Butterfly = 0; Butterfly < NVAL; Butterfly += pow(2,Stage)){
            for(n = 0; n < Half; n++){
                pos = n + Butterfly;
                r = pow(2,(NUM_BITS - Stage)) * n;
                Wn = cexp(((I)*(2*M_PI)*r/VECT_SIZE));
                temp = (Wn * x[pos+Half]);
                x[pos+Half]     = (x[pos] - temp);
                x[pos]          = (x[pos] + temp); // OPERACIONES
            }
        }
        Half *= 2;
    }
    return 1;
}
/**--------------------------------------------UART--------------------------------------------**/

void UARTConfigTx(void){

     unsigned int config = 0;

    /* Enabling the PSC for UART2.*/
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_UART2, PSC_POWERDOMAIN_ALWAYS_ON,
             PSC_MDCTL_NEXT_ENABLE);

    /* Setup PINMUX */
    UARTPinMuxSetup(2, FALSE);

    /* Enabling the transmitter and receiver*/
    UARTTxEnable(SOC_UART_2_REGS);

    /* 1 stopbit, 8-bit character, no parity */
    config = UART_WORDL_8BITS;

    /* Configuring the UART parameters*/
    UARTConfigSetExpClk(SOC_UART_2_REGS, SOC_UART_2_MODULE_FREQ,
                        BAUD_128000, config,
                        UART_OVER_SAMP_RATE_16);

    /* Enabling the FIFO and flushing the Tx and Rx FIFOs.*/
    //UARTFIFOTxEnable(SOC_UART_2_REGS);
}

void UARTTxEnable (unsigned int baseAdd)
{
    /* Enable the Tx, Rx and the free running mode of operation. */
    HWREG(baseAdd + UART_PWREMU_MGMT) = (UART_FREE_MODE |       \
                                        UART_TX_RST_ENABLE);
}

void UARTFIFOTxEnable(unsigned int baseAdd)
{
    HWREG(baseAdd + UART_FCR) = (UART_FIFO_MODE |
                                 UART_TX_CLEAR);
}

void UARTSend (unsigned int length){

    if(CtrlUART==1){

        int i,z;
        int asciisize;
        //short count;

        for(i=0;i<N;i++){
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
            txdata  = txArrayN[i] ;
            //printf("%d\n",txdata);
            asciisize = sprintf(ASCII,"%d\n",txdata);
            // printf("%c \t %c \t %c \t %c\n",ASCII[0],ASCII[1],ASCII[2],ASCII[3]);
            for(z=0;z<asciisize;z+=1){
                UARTCharPut(SOC_UART_2_REGS, ASCII[z]);
            }
        }
        for(i=0;i<N;i++){
        //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
                    // txdata  = FFTReversed[i];
                    //printf("%d\n",txdata);
                    asciisize = sprintf(ASCII,"%f\n",(float)cabs(FFTReversed[i]));
                    // printf("%c \t %c \t %c \t %c\n",ASCII[0],ASCII[1],ASCII[2],ASCII[3]);
                    for(z=0;z<asciisize;z+=1){
                        UARTCharPut(SOC_UART_2_REGS, ASCII[z]);
                    }
         }

        McASPRxIntEnable(SOC_MCASP_0_CTRL_REGS, MCASP_RX_DMAERROR
                                                    | MCASP_RX_CLKFAIL
                                                    | MCASP_RX_SYNCERROR
                                                    | MCASP_RX_OVERRUN);
        EDMA3EnableEvtIntr(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_RX);
        CtrlUART = 0;
        Modo = ModoFFT;

    }
}

/**--------------------------------------------GPIO--------------------------------------------**/

void GPIOConfig(void){

    /**---------------------------BUTTON---------------------------**/

    /* The Local PSC number for GPIO is 3. GPIO belongs to PSC1 module.*/
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON,
             PSC_MDCTL_NEXT_ENABLE);

    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO, PSC_POWERDOMAIN_ALWAYS_ON,
             PSC_MDCTL_NEXT_ENABLE);

    /* Pin Multiplexing of pin 12 of GPIO Bank 6.*/
    GPIOBank6Pin12PinMuxSetup();
    /* Pin Multiplexing of pin 4 of GPIO Bank 2.*/
    GPIOBank2Pin4PinMuxSetup();

    /* Sets the pin 109 (GP6[12]) as output.*/
    GPIODirModeSet(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_DIR_OUTPUT);

    /* Sets the pin 37 (GP2[4]) as input.*/
    GPIODirModeSet(SOC_GPIO_0_REGS, GPIOBank2Pin4_DSP, GPIO_DIR_INPUT);

    /**---------------------------SWITCHES---------------------------**/
    /*Power up GPIO*/
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_GPIO,                   \
                     PSC_POWERDOMAIN_ALWAYS_ON,PSC_MDCTL_NEXT_ENABLE);

    /* Set up pin mux for GPIO*/
    GPIOPinMuxSetup_Switches();
    SwitchAsInputs();

}

static void SetupInt(void)
{
#ifdef _TMS320C6X
    // Initialize the DSP interrupt controller - INTC
    IntDSPINTCInit();
#else
    /* Initialize the ARM Interrupt Controller.*/
    IntAINTCInit();
#endif

    /* Initialize the I2C 0 interface for the codec AIC31 */
    I2CCodecIfInit(SOC_I2C_0_REGS, INT_CHANNEL_I2C, I2C_SLAVE_CODEC_AIC31);

    EDMA3Init(SOC_EDMA30CC_0_REGS, 0);
    EDMA3IntSetup();

    McASPErrorIntSetup();

#ifdef _TMS320C6X
    // Enable DSP interrupts globally
    IntGlobalEnable();
#else
    /* Enable the interrupts generation at global level */
    /* Enable IRQ in CPSR.*/
    IntMasterIRQEnable();
    /* Enable the interrupts in GER of AINTC.*/
    IntGlobalEnable();
    /* Enable the interrupts in HIER of AINTC.*/
    IntIRQEnable();
#endif

}

void GPIOBank2Pin4PinMuxSetup(void)
{
    // Pinmux6
    // 8h
    //HWREG(0x01C14138) = 0x88888800;

    unsigned int savePinmux = 0;

    /*
    ** Clearing the bit in context and retaining the other bit values
    ** in PINMUX6 register.
    */

    savePinmux = (HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) &
                 ~(SYSCFG_PINMUX6_PINMUX6_15_12));

    /* Setting the pins corresponding to GP2[4] in PINMUX6 register.*/
    HWREG(SOC_SYSCFG_0_REGS + SYSCFG0_PINMUX(6)) =
         (PINMUX6_GPIO2_4_ENABLE | savePinmux);

}

static void ConfigureIntGPIO_AINTC(void)
{
#ifdef _TMS320C6X
    IntRegister(C674X_MASK_INT6, ButtonISR);
    IntEventMap(C674X_MASK_INT6, SYS_INT_GPIO_B2INT);
    IntEnable(C674X_MASK_INT6);
#else
    /* Registers the UARTIsr in the Interrupt Vector Table of AINTC. */
    IntRegister(SYS_INT_GPIO_B2INT, ButtonISR);

    /* Map the channel number 2 of AINTC to UART2 system interrupt. */
    IntChannelSet(SYS_INT_GPIO_B2INT, INT_CHANNEL_GPIOB);

    IntSystemEnable(SYS_INT_GPIO_B2INT);
#endif
}

static void ConfigureIntGPIO(void)
{

    GPIOBankIntEnable(SOC_GPIO_0_REGS, bankNum);
    GPIOIntTypeSet(SOC_GPIO_0_REGS,GPIOBank2Pin4_DSP,GPIO_INT_TYPE_SELECTED);

}

static void ButtonISR(void)
{

#ifdef _TMS320C6X
    // Clear UART2 system interrupt in DSPINTC
    IntEventClear(SYS_INT_GPIO_B2INT);
#else
    /* Clears the system interupt status of UART2 in AINTC. */
    IntSystemStatusClear(SYS_INT_GPIO_B2INT);
#endif

    if(A){
        //GPIOPinWrite(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_PIN_HIGH);
        A = 0;
        CtrlUART = 1;

    }
    else{
        //GPIOPinWrite(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_PIN_LOW);
        A = 1;
        CtrlFFT = 1;
        DFT_DT();
    }

}

/**--------------------------------------------MCASP--------------------------------------------**/

void EDMA3ChannelConfig(void){

    /*
    ** Request EDMA channels. Channel 0 is used for reception and
    ** Channel 1 is used for transmission
    */
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                       EDMA3_CHA_MCASP0_TX, EDMA3_CHA_MCASP0_TX, 0);
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
                       EDMA3_CHA_MCASP0_RX, EDMA3_CHA_MCASP0_RX, 0);

}

void EDMA_MCASP_PSC(void){

    /* Power up the McASP module */
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_MCASP0, PSC_POWERDOMAIN_ALWAYS_ON,PSC_MDCTL_NEXT_ENABLE);

    /* Power up EDMA3CC_0 and EDMA3TC_0 */
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON,PSC_MDCTL_NEXT_ENABLE);
    PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON,PSC_MDCTL_NEXT_ENABLE);

}

void MCASPI2SConfig(void){

    /* Initialize the DMA parameters */
    I2SDMAParamInit();

    /* Configure the Codec for I2S mode */
    AIC31I2SConfigure();

    /* Configure the McASP for I2S */
    McASPI2SConfigure();

    /* Activate the audio transmission and reception */
    I2SDataTxRxActivate();

}
/*-----------------------------DFT-----------------------------*/

void DFT_DT(void){

    if(CtrlFFT){

        McASPRxIntDisable(SOC_MCASP_0_CTRL_REGS, MCASP_RX_DMAERROR
                          | MCASP_RX_CLKFAIL
                          | MCASP_RX_SYNCERROR
                          | MCASP_RX_OVERRUN);
        EDMA3DisableEvtIntr(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_RX);
        //GPIOPinWrite(SOC_GPIO_0_REGS, GPIOBank6Pin12_DSP, GPIO_PIN_HIGH);
        CtrlFFT = 0;
        Modo = BUFF_COPY;

    }
}

/*-----------------------------NUEVO-----------------------------*/
void GPIOPinMuxSetup_Switches(void){
    HWREG(0x01C14124) = 0x08888000;
}

void SwitchAsInputs(void){
    GPIODirModeSet(0x01E26010, 2, 1);
    GPIODirModeSet(0x01E26010, 3, 1);
    GPIODirModeSet(0x01E26010, 4, 1);
    GPIODirModeSet(0x01E26010, 5, 1);
}


/*
** Assigns loop job for a parameter set
*/
static void ParamTxLoopJobSet(unsigned short parId)
{
    EDMA3CCPaRAMEntry paramSet;

    memcpy(&paramSet, &txDefaultPar, SIZE_PARAMSET - 2);

    /* link the paRAM to itself */
    paramSet.linkAddr = parId * SIZE_PARAMSET;

    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, parId, &paramSet);
}

/*
** Initializes the DMA parameters.
** The RX basic paRAM set(channel) is 0 and TX basic paRAM set (channel) is 1.
**
** The RX paRAM set 0 will be initialized to receive data in the rx buffer 0.
** The transfer completion interrupt will not be enabled for paRAM set 0;
** paRAM set 0 will be linked to linked paRAM set starting (PAR_RX_START) of RX.
** and further reception only happens via linked paRAM set.
** For example, if the PAR_RX_START value is 40, and the number of paRAMS is 2,
** reception paRAM set linking will be initialized as 0-->40-->41-->40
**
** The TX paRAM sets will be initialized to transmit from the loop buffer.
** The size of the loop buffer can be configured.
** The transfer completion interrupt will not be enabled for paRAM set 1;
** paRAM set 1 will be linked to linked paRAM set starting (PAR_TX_START) of TX.
** All other paRAM sets will be linked to itself.
** and further transmission only happens via linked paRAM set.
** For example, if the PAR_RX_START value is 42, and the number of paRAMS is 2,
** So transmission paRAM set linking will be initialized as 1-->42-->42, 43->43.
*/
static void I2SDMAParamInit(void)
{
    EDMA3CCPaRAMEntry paramSet;
    int idx;

    /* Initialize the 0th paRAM set for receive */
    memcpy(&paramSet, &rxDefaultPar, SIZE_PARAMSET - 2);

    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_RX, &paramSet);

    /* further paramsets, enable interrupt */
    paramSet.opt |= RX_DMA_INT_ENABLE;

    for(idx = 0 ; idx < NUM_PAR; idx++)
    {
        paramSet.destAddr = rxBufPtr[idx];

        paramSet.linkAddr = (PAR_RX_START + ((idx + 1) % NUM_PAR))
                             * (SIZE_PARAMSET);

        paramSet.bCnt =  NUM_SAMPLES_PER_AUDIO_BUF;

        /*
        ** for the first linked paRAM set, start receiving the second
        ** sample only since the first sample is already received in
        ** rx buffer 0 itself.
        */
        if( 0 == idx)
        {
            paramSet.destAddr += BYTES_PER_SAMPLE;
            paramSet.bCnt -= BYTES_PER_SAMPLE;
        }

        EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, (PAR_RX_START + idx), &paramSet);
    }

    /* Initialize the required variables for reception */
    nxtBufToRcv = idx % NUM_BUF;
    lastFullRxBuf = NUM_BUF - 1;
    parOffRcvd = 0;

    /* Initialize the 1st paRAM set for transmit */
    memcpy(&paramSet, &txDefaultPar, SIZE_PARAMSET);

    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX, &paramSet);

    /* rest of the params, enable loop job */
    for(idx = 0 ; idx < NUM_PAR; idx++)
    {
        ParamTxLoopJobSet(PAR_TX_START + idx);
    }

    /* Initialize the variables for transmit */
    parOffSent = 0;
    lastSentTxBuf = NUM_BUF - 1;
}

/*
** Function to configure the codec for I2S mode
*/
static void AIC31I2SConfigure(void)
{
    volatile unsigned int delay = 0xFFF;

    AIC31Reset(SOC_I2C_0_REGS);
    while(delay--);

    /* Configure the data format and sampling rate */
    AIC31DataConfig(SOC_I2C_0_REGS, AIC31_DATATYPE_I2S, SLOT_SIZE, 0);
    AIC31SampleRateConfig(SOC_I2C_0_REGS, AIC31_MODE_BOTH, SAMPLING_RATE);

    /* Initialize both ADC and DAC */
    AIC31ADCInit(SOC_I2C_0_REGS);
    AIC31DACInit(SOC_I2C_0_REGS);
}

/*
** Configures the McASP Transmit Section in I2S mode.
*/
static void McASPI2SConfigure(void)
{
    McASPRxReset(SOC_MCASP_0_CTRL_REGS);
    McASPTxReset(SOC_MCASP_0_CTRL_REGS);

    /* Enable the FIFOs for DMA transfer */
    McASPReadFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);
    McASPWriteFifoEnable(SOC_MCASP_0_FIFO_REGS, 1, 1);

    /* Set I2S format in the transmitter/receiver format units */
    McASPRxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, WORD_SIZE, SLOT_SIZE,
                     MCASP_RX_MODE_DMA);
    McASPTxFmtI2SSet(SOC_MCASP_0_CTRL_REGS, WORD_SIZE, SLOT_SIZE,
                     MCASP_TX_MODE_DMA);

    /* Configure the frame sync. I2S shall work in TDM format with 2 slots */
    McASPRxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, 2, MCASP_RX_FS_WIDTH_WORD,
                        MCASP_RX_FS_EXT_BEGIN_ON_FALL_EDGE);
    McASPTxFrameSyncCfg(SOC_MCASP_0_CTRL_REGS, 2, MCASP_TX_FS_WIDTH_WORD,
                        MCASP_TX_FS_EXT_BEGIN_ON_RIS_EDGE);

    /* configure the clock for receiver */
    McASPRxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL, 0, 0);
    McASPRxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_POL_RIS_EDGE);
    McASPRxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLKCHCK_DIV32,
                          0x00, 0xFF);

    /* configure the clock for transmitter */
    McASPTxClkCfg(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL, 0, 0);
    McASPTxClkPolaritySet(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_POL_FALL_EDGE);
    McASPTxClkCheckConfig(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLKCHCK_DIV32,
                          0x00, 0xFF);

    /* Enable synchronization of RX and TX sections  */
    McASPTxRxClkSyncEnable(SOC_MCASP_0_CTRL_REGS);

    /* Enable the transmitter/receiver slots. I2S uses 2 slots */
    McASPRxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, I2S_SLOTS);
    McASPTxTimeSlotSet(SOC_MCASP_0_CTRL_REGS, I2S_SLOTS);

    /*
    ** Set the serializers, Currently only one serializer is set as
    ** transmitter and one serializer as receiver.
    */
    McASPSerializerRxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_RX);
    McASPSerializerTxSet(SOC_MCASP_0_CTRL_REGS, MCASP_XSER_TX);

    /*
    ** Configure the McASP pins
    ** Input - Frame Sync, Clock and Serializer Rx
    ** Output - Serializer Tx is connected to the input of the codec
    */
    McASPPinMcASPSet(SOC_MCASP_0_CTRL_REGS, 0xFFFFFFFF);
    McASPPinDirOutputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AXR(MCASP_XSER_TX));
    McASPPinDirInputSet(SOC_MCASP_0_CTRL_REGS, MCASP_PIN_AFSX
                                               | MCASP_PIN_ACLKX
                                               | MCASP_PIN_AFSR
                                               | MCASP_PIN_ACLKR
                                               | MCASP_PIN_AXR(MCASP_XSER_RX));

    /* Enable error interrupts for McASP */
    /*McASPTxIntEnable(SOC_MCASP_0_CTRL_REGS, MCASP_TX_DMAERROR
                                            | MCASP_TX_CLKFAIL
                                            | MCASP_TX_SYNCERROR
                                            | MCASP_TX_UNDERRUN);*/

    McASPRxIntEnable(SOC_MCASP_0_CTRL_REGS, MCASP_RX_DMAERROR
                                            | MCASP_RX_CLKFAIL
                                            | MCASP_RX_SYNCERROR
                                            | MCASP_RX_OVERRUN);
}

/*
** Sets up the interrupts for EDMA in AINTC
*/
static void EDMA3IntSetup(void)
{
#ifdef _TMS320C6X
    IntRegister(C674X_MASK_INT5, EDMA3CCComplIsr);
    IntEventMap(C674X_MASK_INT5, SYS_INT_EDMA3_0_CC0_INT1);
    IntEnable(C674X_MASK_INT5);
#else
    IntRegister(SYS_INT_CCINT0, EDMA3CCComplIsr);
    IntChannelSet(SYS_INT_CCINT0, INT_CHANNEL_EDMACC);
    IntSystemEnable(SYS_INT_CCINT0);
#endif
}

/*
** Sets up the error interrupts for McASP in AINTC
*/
static void McASPErrorIntSetup(void)
{
#ifdef _TMS320C6X
    IntRegister(C674X_MASK_INT7, McASPErrorIsr);
    IntEventMap(C674X_MASK_INT7, SYS_INT_MCASP0_INT);
    IntEnable(C674X_MASK_INT7);
#else
    /* Register the error ISR for McASP */
    IntRegister(SYS_INT_MCASPINT, McASPErrorIsr);

    IntChannelSet(SYS_INT_MCASPINT, INT_CHANNEL_MCASP);
    IntSystemEnable(SYS_INT_MCASPINT);
#endif
}

/*
** Activates the data transmission/reception
** The DMA parameters shall be ready before calling this function.
*/
static void I2SDataTxRxActivate(void)
{
    /* Start the clocks */
    McASPRxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_RX_CLK_EXTERNAL);
    McASPTxClkStart(SOC_MCASP_0_CTRL_REGS, MCASP_TX_CLK_EXTERNAL);

    /* Enable EDMA for the transfer */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_RX,
                        EDMA3_TRIG_MODE_EVENT);
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS,
                        EDMA3_CHA_MCASP0_TX, EDMA3_TRIG_MODE_EVENT);

    /* Activate the  serializers */
    McASPRxSerActivate(SOC_MCASP_0_CTRL_REGS);
    McASPTxSerActivate(SOC_MCASP_0_CTRL_REGS);

    /* make sure that the XDATA bit is cleared to zero */
    while(McASPTxStatusGet(SOC_MCASP_0_CTRL_REGS) & MCASP_TX_STAT_DATAREADY);

    /* Activate the state machines */
    McASPRxEnable(SOC_MCASP_0_CTRL_REGS);
    //McASPTxEnable(SOC_MCASP_0_CTRL_REGS);
}

/*
** Activates the DMA transfer for a parameterset from the given buffer.
*/
void BufferTxDMAActivate(unsigned int txBuf, unsigned short numSamples,
                         unsigned short parId, unsigned short linkPar)
{
    EDMA3CCPaRAMEntry paramSet;

    /* Copy the default paramset */
    memcpy(&paramSet, &txDefaultPar, SIZE_PARAMSET - 2);

    /* Enable completion interrupt */
    paramSet.opt |= TX_DMA_INT_ENABLE;
    paramSet.srcAddr =  txBufPtr[txBuf];
    paramSet.linkAddr = linkPar * SIZE_PARAMSET;
    paramSet.bCnt = numSamples;

    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, parId, &paramSet);
}

/*
** Activates the DMA transfer for a parameter set from the given buffer.
*/
static void BufferRxDMAActivate(unsigned int rxBuf, unsigned short parId,
                                unsigned short parLink)
{
    EDMA3CCPaRAMEntry paramSet;

    /* Copy the default paramset */
    memcpy(&paramSet, &rxDefaultPar, SIZE_PARAMSET - 2);

    /* Enable completion interrupt */
    paramSet.opt |= RX_DMA_INT_ENABLE;
    paramSet.destAddr =  rxBufPtr[rxBuf];
    paramSet.bCnt =  NUM_SAMPLES_PER_AUDIO_BUF;
    paramSet.linkAddr = parLink * SIZE_PARAMSET ;

    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, parId, &paramSet);
}

/*
** This function will be called once receive DMA is completed
*/
static void McASPRxDMAComplHandler(void)
{
    unsigned short nxtParToUpdate;

    /*
    ** Update lastFullRxBuf to indicate a new buffer reception
    ** is completed.
    */
    lastFullRxBuf = (lastFullRxBuf + 1) % NUM_BUF;
    nxtParToUpdate =  PAR_RX_START + parOffRcvd;
    parOffRcvd = (parOffRcvd + 1) % NUM_PAR;

    /*
    ** Update the DMA parameters for the received buffer to receive
    ** further data in proper buffer
    */
    BufferRxDMAActivate(nxtBufToRcv, nxtParToUpdate,
                        PAR_RX_START + parOffRcvd);

    /* update the next buffer to receive data */
    nxtBufToRcv = (nxtBufToRcv + 1) % NUM_BUF;
}

/*
** This function will be called once transmit DMA is completed
*/
static void McASPTxDMAComplHandler(void)
{
    ParamTxLoopJobSet((unsigned short)(PAR_TX_START + parOffSent));

    parOffSent = (parOffSent + 1) % NUM_PAR;
}

/*
** EDMA transfer completion ISR
*/
static void EDMA3CCComplIsr(void)
{
#ifdef _TMS320C6X
    IntEventClear(SYS_INT_EDMA3_0_CC0_INT1);
#else
    IntSystemStatusClear(SYS_INT_CCINT0);
#endif

    /* Check if receive DMA completed */
    if(EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS) & (1 << EDMA3_CHA_MCASP0_RX))
    {
        /* Clear the interrupt status for the 0th channel */
        EDMA3ClrIntr(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_RX);
        McASPRxDMAComplHandler();
    }

    /* Check if transmit DMA completed */
    if(EDMA3GetIntrStatus(SOC_EDMA30CC_0_REGS) & (1 << EDMA3_CHA_MCASP0_TX))
    {
        /* Clear the interrupt status for the first channel */
        EDMA3ClrIntr(SOC_EDMA30CC_0_REGS, EDMA3_CHA_MCASP0_TX);
        McASPTxDMAComplHandler();
    }
}

/*
** Error ISR for McASP
*/
static void McASPErrorIsr(void)
{
#ifdef _TMS320C6X
    IntEventClear(SYS_INT_MCASP0_INT);
#else
    IntSystemStatusClear(SYS_INT_MCASPINT);
#endif

    ; /* Perform any error handling here.*/
}

/***************************** End Of File ***********************************/
