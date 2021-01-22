/*
 * vibrationCalculations.c
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */


#include "vibrationCalculations.h"
#include "errorHandling.h"
#include "stdio.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "app_bluenrg_ms.h"

/********** FFT Calculation ***********/
uint16_t adc_buf[FFT_NUMBER_SAMPLES*2];
float32_t fft_out[FFT_NUMBER_SAMPLES];
float32_t fft_in[FFT_NUMBER_SAMPLES];
int fft_db[FFT_NUMBER_SAMPLES/2];
float32_t fft_abs[FFT_NUMBER_SAMPLES/2];
char is_data_ready_for_fft = 0;
arm_rfft_fast_instance_f32 fft_handler;

uint16_t fft_idx_start_moinitoring = 75.0  * FFT_NUMBER_SAMPLES / SAMPLINNG_FREQUENCY; // 75 Hz
uint16_t fft_idx_end_moinitoring   = 85.0 * FFT_NUMBER_SAMPLES / SAMPLINNG_FREQUENCY; // 150 Hz
float32_t fft_freq_start_moinitoring, fft_freq_end_moinitoring;
float32_t fft_threshold = 15.0;

void init_fft(){
  arm_status fft_init_status = arm_rfft_fast_init_f32 (&fft_handler, FFT_NUMBER_SAMPLES);
  if ( fft_init_status != ARM_MATH_SUCCESS )
  {
    Error_Handler();
  }
  fft_freq_start_moinitoring = fft_idx_start_moinitoring * SAMPLINNG_FREQUENCY / FFT_NUMBER_SAMPLES;
  fft_freq_end_moinitoring   = fft_idx_end_moinitoring   * SAMPLINNG_FREQUENCY / FFT_NUMBER_SAMPLES;
}

// help from
// https://github.com/YetAnotherElectronicsChannel/STM32_FFT_Spectrum_Analysis/blob/master/code%20STM32/Src/main.c
float32_t complexABS(float real, float compl) {
  return sqrtf(real*real+compl*compl);
}

void calculate_fft()
{
  // calculate the fft
  arm_rfft_fast_f32(&fft_handler, fft_in, fft_out, 0);

  // mix the real and imaginary values of the fft output
  for (size_t i = 0; i < FFT_NUMBER_SAMPLES/2 ; i+=1)
  {
    fft_abs[i] = complexABS(fft_out[2*i], fft_out[2*i+1]);
//    fft_db[i] = (int)(20*log10f(complexABS(fft_out[2*i], fft_out[2*i+1])));
  }
}

uint32_t time_alarm_led_is_on = 0;
unsigned char alarm_status = 0;

uint32_t largest_freqs;
uint32_t largest_fft;

void check_fft_results()
{
  largest_freqs = 0;
  largest_fft = 0;
  for (uint16_t idx = fft_idx_start_moinitoring; idx < fft_idx_end_moinitoring; idx++){
    if (fft_abs[idx] > fft_threshold)
    {
      if (fft_abs[idx]>largest_fft)
      {
	largest_fft = fft_abs[idx];
	largest_freqs = idx;
      }
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
      time_alarm_led_is_on = HAL_GetTick();
      alarm_status = 1;
      MX_BlueNRG_Notify();
    }
  }
}

void fft_process(){
  float32_t offset = 3.3/2.0;
  float32_t adc_to_voltage = 3.3 / 65535;
  if (is_data_ready_for_fft == 1)
  {
    // copy half of adc buffer to fft buffer
    for (size_t i = 0; i < FFT_NUMBER_SAMPLES ; i++)
    {
      fft_in[i] = (float32_t)(adc_buf[i]) * adc_to_voltage - offset;
    }
    calculate_fft();
    check_fft_results();
    is_data_ready_for_fft = 0;
  }
  else if (is_data_ready_for_fft == 2)
  {
    // copy the other half of adc buffer to fft buffer
    for (size_t i = 0; i < FFT_NUMBER_SAMPLES; i++)
    {
      fft_in[i] = (float32_t)(adc_buf[i+FFT_NUMBER_SAMPLES]) * adc_to_voltage - offset;
    }
    calculate_fft();
    check_fft_results();
    is_data_ready_for_fft = 0;
  }

  if ( alarm_status == 1)
  {
    uint32_t passed_time = HAL_GetTick() - time_alarm_led_is_on;
    if (passed_time > 1000){
      HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
      alarm_status = 0;
    }
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
//  mean_square_init(512, &vibrationMeanSquare);
  init_fft();
}

void vibration_process()
{
  fft_process();

//  if(new_measurement_arrived == 0)
//  {
//    return;
//  }
//  if(vibrationMeanSquare.mean_square > mean_square_threshold)
//  {
//    // send notification
//  }
//
//  new_measurement_arrived = 0;
}
