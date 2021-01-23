
```
python3 -m pip install -U setuptools pip wheel jupyter

sudo apt-get install libbluetooth-dev \
    pkg-config \
    libboost-python-dev \
    libboost-thread-dev \
    libbluetooth-dev \
    libglib2.0-dev \
    python-dev 

python3 -m pip install -U gattlib bluepy # two different BLE lib
python3 -m pip install -U futures blue-st-sdk # not that much usefull, needs sudo
python3 -m pip install -U pybluez # for testing L2CAP

python3 -m pip uninstall serial

python3 -m pip install jupyterlab pyserial bokeh pyrealsense2

python3 -m pip install python-telegram-bot

```


For jetson
```
sudo iw dev wlan0 set power_save off
sudo jetson_clocks
sudo nvpmodel -m 0
```