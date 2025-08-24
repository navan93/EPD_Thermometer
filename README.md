# EPD Thermometer
This is a room thermometer built using the 1.54" EPD pricetags. More info about these can be found [here]().

The firmware is based on this .

A BMP180 temperature sensor module is connected over I2C to the display board like shown below.

....IMAGE....

## Install
The EPDs that I have used are based on the Solum SoCs with and 8051 core and as such they need SDCC compiler. Note that it's important to install this specific version otherwise there will be weird problems with the build.

[SDCC 4.4.0](https://sourceforge.net/projects/sdcc/files/sdcc-linux-amd64/4.4.0/)

```
cd ~
mkdir tmp
cd tmp
tar xjf path/to/binary/kit/sdcc-4.4.0-amd64-unknown-linux2.5.tar.bz2
cd sdcc-4.4.0
cp -r * /usr/local
```

## Build
To build the firmware run

```
cd firmware/zbs243_Tag_FW
make
```

## Flash
The flasher is based on this project and I have used the 3D printed jig.

> [!WARNING]
> Always read the infopage before flashing for the first time. This has the factory calibration data that is needed for the display to work. Save this file per display.

```
./zbs_flasher.py -p /dev/ttyUSB0 read_infopage infopage
```

```
./zbs_flasher.py -p /dev/ttyUSB0 write firmware/zbs243_Tag_FW/main.bin
```

To reset the display run
```
./zbs_flasher.py -p /dev/ttyUSB0 reset
```

