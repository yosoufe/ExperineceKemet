/*
 * vibrationCalculations.c
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */


#include "vibrationCalculations.h"
#include "errorHandling.h"
#include "stdio.h"

/********** FFT Calculation ***********/
uint16_t adc_buf[FFT_NUMBER_SAMPLES*2];
float32_t fft_out[FFT_NUMBER_SAMPLES];
float32_t fft_in[FFT_NUMBER_SAMPLES];
int freqs[FFT_NUMBER_SAMPLES/2];
char is_data_ready_for_fft = 0;
arm_rfft_fast_instance_f32 fft_handler;


void init_fft(){
  arm_status fft_init_status = arm_rfft_fast_init_f32 (&fft_handler, FFT_NUMBER_SAMPLES);
  if ( fft_init_status != ARM_MATH_SUCCESS )
  {
    Error_Handler();
  }
}

// help from
// https://github.com/YetAnotherElectronicsChannel/STM32_FFT_Spectrum_Analysis/blob/master/code%20STM32/Src/main.c
float complexABS(float real, float compl) {
  return sqrtf(real*real+compl*compl);
}

void calculate_fft()
{
  // calculate the fft
  arm_rfft_fast_f32(&fft_handler, fft_in, fft_out, 0);

  // mix the real and imaginary values of the fft output
  for (size_t i = 0; i < FFT_NUMBER_SAMPLES/2 ; i+=1)
  {
    freqs[i] = (int)(20*log10f(complexABS(fft_out[2*i], fft_out[2*i+1])));
  }
}

void fft_process(){
  if (is_data_ready_for_fft == 1)
  {
    // copy half of adc buffer to fft buffer
    for (size_t i = 0; i < FFT_NUMBER_SAMPLES ; i++)
    {
      fft_in[i] = (float32_t)(adc_buf[i]) * 3.3 / 65535;
    }
    calculate_fft();
    is_data_ready_for_fft = 0;
  }
  else if (is_data_ready_for_fft == 2)
  {
    // copy the other half of adc buffer to fft buffer
    for (size_t i = 0; i < FFT_NUMBER_SAMPLES; i++)
    {
      fft_in[i] = (float32_t)(adc_buf[i+FFT_NUMBER_SAMPLES]) * 3.3 / 65535;
    }
    calculate_fft();
    is_data_ready_for_fft = 0;
  }
}


/********* Moving Mean Square (MS) Calculation **********/
void mean_square_init(uint16_t window_length, MeanSquare* ms){
  if (window_length > MAX_MEAN_SQUARE_WINDOW_SIZE)
  {
    Error_Handler();
  }
  ms->window_length = window_length;
  ms->sum_square = 0;
  ms->mean_square = 0;
  ms->idx_start = 0;
  ms->idx_end = 0;
}

void mean_square_add_value(int32_t value, MeanSquare* ms){
  // calculate the square
  uint32_t square = value * value;
  // add the square at the index to history
  ms->history[ms->idx_end] = square;
  // calculate the new end index in history
  ms->idx_end = (ms->idx_end + 1) % MAX_MEAN_SQUARE_WINDOW_SIZE;
  uint16_t count_valid_elements = (ms->idx_end - ms->idx_start) % MAX_MEAN_SQUARE_WINDOW_SIZE;
  if (count_valid_elements > ms->window_length)
  {
    uint16_t target_start_idx = (ms->idx_end - ms->window_length) % MAX_MEAN_SQUARE_WINDOW_SIZE;
    while(ms->idx_start != target_start_idx){
      // remove the start from the sum
      ms->sum_square -= ms->history[ms->idx_start];
      ms->idx_start = (ms->idx_start+1) % MAX_MEAN_SQUARE_WINDOW_SIZE;
    }
    count_valid_elements = (ms->idx_end - ms->idx_start) % MAX_MEAN_SQUARE_WINDOW_SIZE;
  }
  // add the square to the sum
  ms->sum_square += square;
  // update the mean
  ms->mean_square = ms->sum_square / count_valid_elements;
}

void mean_square_update_window_length(uint16_t window_length, MeanSquare* ms){
  uint16_t count_valid_elements = ms->idx_end - ms->idx_start;
  if(window_length < count_valid_elements){
    ms->idx_start = (ms->idx_end - count_valid_elements + 1) % MAX_MEAN_SQUARE_WINDOW_SIZE;
    ms->sum_square = 0;
    uint16_t idx = ms->idx_start;
    while(idx != ms->idx_end){
      ms->sum_square += ms->history[idx];
      idx = (idx+1) % MAX_MEAN_SQUARE_WINDOW_SIZE;
    }
    ms->mean_square = ms->sum_square / window_length;
  }
  ms->window_length = window_length;
}


/******* VIBRATION ANALYSIS ********/
MeanSquare vibrationMeanSquare;
char new_measurement_arrived = 0;
float mean_square_threshold = 50000;

void vibration_init()
{
  mean_square_init(512, &vibrationMeanSquare);
}

void vibration_process()
{
  if(new_measurement_arrived == 0)
  {
    return;
  }
  if(vibrationMeanSquare.mean_square > mean_square_threshold)
  {
    // send notification
  }

  new_measurement_arrived = 0;
}
