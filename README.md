# [Experinece with Vibration Sensor (Kemet)](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/)

This repository contain any code related to the [Experinece with Vibration Sensor (Kemet)](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/) challenge at element14.

Here is a list of my blogs:
- [Introduction - Blog #1](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---introduction---blog-1)
- [Basic Experiments - Blog #2](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---basic-experiments---blog-2)
- [Calculating FFT on Nucleo Board - Blog #3](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---calculating-fft-on-nucleo-board---blog-3)
- [Setup to Measure Floor's Vibration and Threshold Alerts Logic - Blog #4](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---setup-to-measure-floor-s-vibration-and-threshold-alerts-logic---blog-4)
- [High Vibration Detection on Nucleo Board, BLE and Telegram Bot - Blog #5](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---high-vibration-detection-on-nucleo-board-alone-no-jetson-nano-and-telegram-bot---blog-5)
- [What I learned and Summary - Blog #6](https://community.element14.com/challenges-projects/design-challenges/experimenting-with-vibration-sensors/b/blog/posts/vibration-sensor-and-exercising---what-i-learned-and-summary---blog-6)

![Vibration sensor in my 3D Printed mount](https://user-images.githubusercontent.com/7648675/146323373-0eefc5b8-88f6-4cdc-8edf-b88515cafa4e.png)

# Measurement and Structural Analysis

In order to analyse buildings and structures it is important to 
measure the natrual frequency of the structure. It is usally tried
to make the natrual frequency of the structure away from the frequency of 
the earthquke.

Natrual frequency of the compelete structure of buildings is 
usally around 1-3 Hz. Taller the building, lower the natural frequency. 
For example very tall buildings can even have a natural frequency of 0.5 Hz.
But natural frequency of the parts of the building, for example single truss 
can be larger like around 10-20 Hz.

Vibration of the building is analysed using different sensors
like accelerometr or velocimeter.

In order to make a meaning from the sensor data, first step is to conver the 
voltage to physical value like in meters, m/s or m/s^2. Then usually 
FFT (Fast Fourier Transform) is used to extract natrual frequencies.


There can be different setup to measure the natural frequencies. For example one set of Sensor Mountings can be
- 1 sensor in the base of the structure to measure the vibration triggering input.
- 1 sensor at the top of the structure to measure the vibration caused by the trigerring input. 
- Based on the input and output, system identification is done.
- To find out different modes of buildings:
    - Mount a 3rd or more sensors in different location in the structure which has some vibrations to measure at the second, third or more modes of vibrations

There are different ways to cause the vibration in the structure:
- Waiting for earthquake, To measure the damage
    - May show None linear behavior.
    - Elasticity factors and other parameters are changing and system can be nonlinear now.
- Capturing ambient vibrations, To measure the frequencies.
    - Filtering may be needed
- Create artificial vibration.

Time step should be minimum twice of maximum frequency of the structure. (Nyquist Frequency)

Data about Earthquakes
- https://www.usgs.gov/

Test:
- Create input vibration, You should know the input and do not consider the input as output.
You should use bump method. Induce the short vibration like a hammer. From that moment system does the free 
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
<center style="max-width: 740px; text-align: justify; margin: auto;">
<p style="max-width: 740px;text-align: justify; margin: auto;"> </p>
</center>
```
