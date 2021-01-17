/*
 * vibrationCalculations.h
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */

#ifndef INC_VIBRATIONCALCULATIONS_H_
#define INC_VIBRATIONCALCULATIONS_H_

#include "arm_math.h"

/********** FFT Calculation ***********/
#define FFT_NUMBER_SAMPLES 1024


extern uint16_t adc_buf[FFT_NUMBER_SAMPLES*2];
extern float32_t fft_out[FFT_NUMBER_SAMPLES];
extern int freqs[FFT_NUMBER_SAMPLES/2];
extern char is_data_ready_for_fft;


void init_fft();
float complexABS(float real, float compl);
void calculate_fft();
void fft_process();


/********* Moving Mean Square (MS) Calculation **********/
#define MAX_MEAN_SQUARE_WINDOW_SIZE 2048U

typedef struct _MeanSquare {
  uint16_t window_length; 	/* number of samples you would like to calculate MS over it */
  uint64_t sum_square;		/* sum of the square */
  float mean_square;		/* mean of the square */
  uint16_t idx_start; 		/* index of the beginning of valid data in the history */
  uint16_t idx_end;		/* index of the end of valid data in the history */
  uint32_t history[MAX_MEAN_SQUARE_WINDOW_SIZE]; /* history */
} MeanSquare;

void mean_square_init(uint16_t window_length, MeanSquare* ms);
void mean_square_add_value(int32_t value, MeanSquare* ms);
void mean_square_update_window_length(uint16_t window_length, MeanSquare* ms);

/******* VIBRATION ANALYSIS ********/
extern MeanSquare vibrationMeanSquare;
extern char new_measurement_arrived;
extern float mean_square_threshold;

void vibration_init();
void vibration_process();

#endif /* INC_VIBRATIONCALCULATIONS_H_ */
