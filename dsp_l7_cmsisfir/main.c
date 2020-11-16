#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "msp432_boostxl_init.h"
#include "msp432_arm_dsp.h"

#define BUFLEN BUFLEN_8
#define BUFLEN_SZ 8

#define NUMTAPS 32
int16_t taps[NUMTAPS + BUFLEN_SZ - 1];
int coefficients[NUMTAPS] = { (int) (1 * (1 << 15)) };
arm_fir_instance_q15 F;

initfir(arm_fir_instance_q15 *F) {
    F->numTaps  = NUMTAPS;
    F->pState   = taps;
    F->pCoeffs  = coefficients;
}

// #define NOISE

void processBuffer(uint16_t x[BUFLEN_SZ], uint16_t y[BUFLEN_SZ]) {

#ifdef NOISE
    uint32_t i;
    for (i = 0; i<BUFLEN_SZ; i++)
        x[i] = 0x1C00 + rand() % 0x800;
#endif

    adc14_to_q15_vec(x, x, BUFLEN_SZ);
    arm_fir_q15(&F, x, y, BUFLEN_SZ);
    q15_to_dac14_vec(y,y, BUFLEN_SZ);
}


#include <stdio.h>

   int main(void) {
       WDT_A_hold(WDT_A_BASE);

       initfir(&F);

       msp432_boostxl_init_dma(FS_32000_HZ, BOOSTXL_J1_2_IN, BUFLEN, processBuffer);

       uint32_t c = measurePerfBuffer(processBuffer);
       printf("Cycles: %d\n", c);

       msp432_boostxl_run();

       return 1;
   }

