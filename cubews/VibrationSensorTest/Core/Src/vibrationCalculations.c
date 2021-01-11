/*
 * vibrationCalculations.c
 *
 *  Created on: Jan 10, 2021
 *      Author: yousof
 */


#include "vibrationCalculations.h"
#include "errorHandling.h"

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
