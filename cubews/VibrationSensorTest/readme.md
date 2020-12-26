# ADC
- Timer 3 is set to generate events at 10kHz
- ADC 1 is triggered by Timer 3 output event
- DMA Copies 4096 samples to the adc_buf
- The fft is being calculated on the adc_buf and saved on fft_out
- absolute of fft_out is calculated into freqs vector

```
Fs = 10000; 			  # sampling frequency
period = 1/Fs
L = 4096;  # Length of signal
frequency = Fs / L * index_of_freqs
```