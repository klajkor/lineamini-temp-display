# Linea Mini temperature display
Retrofit temperature display for LaMarzocco Linea Mini espresso machine

Board: Arduino Pro Mini AT168P 16MHz 5V

Extension modules used:
- INA219, I2C
- SSD1306 OLED dsiplay, SPI


Libraries used:
- Adafruit INA219 by Adafruit - Copyright (c) 2012, Adafruit Industries
- SSD1306Ascii by Bill Greiman - Copyright (c) 2019, Bill Greiman

Arduino Pro Mini pinout connections

I2C bus:
- SCK - pin A5
- SDA - pin A4

INA219 power pins:
- GND - D2
- VCC - D3

OLED on SPI bus:
- CLK - pin D13
- MOSI - pin D11
- CS - pin D10
- D/C - pin D9

Toolchain: VSCode + Platform.IO

Copyright (c) 2019, Robert Klajko

