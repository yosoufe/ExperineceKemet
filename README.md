# [Experinece with Vibration Sensor (Kemet)](https://www.element14.com/community/community/design-challenges/experimenting-with-vibration-sensors/blog/authors/yosoufe?ICID=DCH-vibrationSense-challengers)

# Measurement Techniques
Measuring the vibrations ins important:
- Measure Natural Frequencies
- Measure frequency of the earthquake
- Is the frequency of earthquake is close to the natural vibration
- Period of vibration is more important in the structure analysis

- Using the accelerometer as sensor
- Velocimeter

For Earthquake:
- Construction may need 1-3 Hz frequency
- Taller buildings even would create lower frequency like 0.5 Hz
- These are the whole buildings
- A truss may have a larger natural frequency.

Processing:
- Convert the voltage to the physical value like meter, m/s or m/s^2
- FFT is one of the methods to get the frequency of different modes.

Data about Earthquakes
- https://www.usgs.gov/


Sensor Mountings:
- 1 sensor in the head as output
- 1 sensor in base as input
- Based on the input and output they do the system identification.
- To find out different modes of buildings:
    - Mount a 3rd sensor in different point which has some vibrations to measure the second mode.

How to trigger:
- Waiting for earthquake, To measure the damage
    - May show None linear behavior.
    - Elasticity factors and other parameters are changing and nonlinear now.
- Capturing ambient vibrations, To measure the frequencies.
    - Filtering may be needed


Time step should be minimum twice of maximum frequency of the structure. (Nyquist Frequency)

Test:
- Create input vibration, You should know the input and do not consider the input as output.
You should use bump method. Induce the short vibration with like a hammer. From that moment system does the free 
vibration. Sudden hit can be [0.1 - 0.2] seconds. One jump or dropping a weight could simulate it.
The hit should be close to the sensor.

# Remote Programming Nucleo

On jetson I compiled OpenOCD from source and I am using that. I added it's bin directory path to $PATH then

To test your installation and version:
```
$ openocd -v
Open On-Chip Debugger 0.11.0-rc1+dev-00026-gaaa6110d9-dirty (2021-01-16-20:33)
Licensed under GNU GPL v2
For bug reports, read
        http://openocd.org/doc/doxygen/bugs.html
```

```
cd /home/yousof/software/openocd/install/share/openocd/scripts/board/
openocd -c 'bindto 0.0.0.0' -f st_nucleo_h743zi.cfg
```

# Some Random Notes:

`p` tag in the report
```html
<p style="max-width: 740px;text-align: justify; margin: auto;"> </p>
```
