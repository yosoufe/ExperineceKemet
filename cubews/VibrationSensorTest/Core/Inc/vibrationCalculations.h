/*
 * vibrationCalculations.h
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */

#ifndef INC_VIBRATIONCALCULATIONS_H_
#define INC_VIBRATIONCALCULATIONS_H_

#include "arm_math.h"

// ADC Buffer Length
#define FFT_NUMBER_SAMPLES 4096

extern uint16_t adc_buf[FFT_NUMBER_SAMPLES*2];
extern float32_t fft_out[FFT_NUMBER_SAMPLES];
extern int freqs[FFT_NUMBER_SAMPLES/2];
extern char is_data_ready_for_fft;


void init_fft();
float complexABS(float real, float compl);
void calculate_fft();
void fft_process();

#endif /* INC_VIBRATIONCALCULATIONS_H_ */
