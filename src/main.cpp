#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiSpi.h"

/**
#define SERIAL_DEBUG_ENABLED 1
*/

/* Declarations and initializations */

// Thermistor calculation values
// Original idea and code from Jimmy Roasts, https://github.com/JimmyRoasts/LaMarzoccoTempSensor

// resistance at 25 degrees C
#define THERMISTORNOMINAL_V1 50000  // version 1
#define THERMISTORNOMINAL_V2 49120  // version 2 updated calculation
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 100
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT_V1 4400  // version 1
#define BCOEFFICIENT_V2 3977  // version 2, updated calculation
// the value of the 'other' resistor
#define SERIESRESISTOR_V1 6960
#define SERIESRESISTOR_V2 6190  // version 2, measured on board
//scaling value to convert voltage
#define VOLTAGESCALE 12.1
//reference voltage
//#define VOLTAGEREF 4.585
#define VOLTAGEREF 4.16

#define LMREF  5.07 //measured from LMBoard --- GND Board


// Current and voltage sensor class
Adafruit_INA219 ina219_monitor;
#define INA219_VCC_PIN 3
#define INA219_GND_PIN 2

// OLED pin definitions
#define SCK_PIN 13  // HW SPI SCK pin, reminder only
#define MOSI_PIN 11 // HW MOSI/SDA pin, reminder only
// Pin connections for OLED displays without RST pin:
#define CS_PIN  10
#define DC_PIN  9
#define RST_PIN 8  // not connected
/**
// Pin connections for OLED displays without CS pin:
#define CS_PIN  8
#define RST_PIN 10
#define DC_PIN  9
*/
SSD1306AsciiSpi oled;

/* Global variables */

static const char str_Linea_Mini[]="Linea Mini";

// INA219 sensor variables
float bus_Voltage_V;    /** Measured bus voltage in V*/
float bus_Voltage_mV;    /** Measured bus voltage in mV*/
char volt_String[] = "99999.9";         /** String to store measured voltage value in mV */

// Calculated temperature
float calc_Temperature_V1 = 0.0;
char temperature_String_V1[] = "  999.9";         /** String to store calculated temperature in Celsius degree */
float steinhart = 0.0;
float calc_Temperature_V2 = 0.0;
char temperature_String_V2[] = "  999.9";
float thermistor_Res = 0.00; // Thermistor calculated resistance

/* Function definitions */

void oled_Init(void);
void ina219_Init(void);
void get_Voltage(void);
void display_Values(void);
void display_Values_V2(void);
void calculate_Temperature_V1(void);
void calculate_Temperature_V2(void);

void setup() {
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.begin(115200);
  Serial.println(F("Start"));
  #endif
  ina219_Init();
  oled_Init();
}


void loop() {
  get_Voltage();
  calculate_Temperature_V1();
  calculate_Temperature_V2();
  //display_Values();
  display_Values_V2();
  delay(1000);
}

void ina219_Init(void)
{
  pinMode(INA219_GND_PIN, OUTPUT);
  pinMode(INA219_VCC_PIN, OUTPUT);
  digitalWrite(INA219_GND_PIN, LOW);
  digitalWrite(INA219_VCC_PIN, HIGH);
  delay(100);
  ina219_monitor.begin();
  //Serial.println(F("INA219 begin done"));
  // begin calls:
  // configure() with default values RANGE_32V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT, CONT_SH_BUS
  // calibrate() with default values D_SHUNT=0.1, D_V_BUS_MAX=32, D_V_SHUNT_MAX=0.2, D_I_MAX_EXPECTED=2
  // in order to work directly with ADAFruit's INA219B breakout
}

void oled_Init(void) {
  // Use next line if no RST_PIN or reset is not required.
  oled.begin(&Adafruit128x64, CS_PIN, DC_PIN);  
  // oled.begin(&Adafruit128x64, CS_PIN, DC_PIN, RST_PIN);
  //oled.setFont(System5x7);
  oled.clear();
  oled.setFont(fixed_bold10x15);
  oled.clear();
  oled.println(str_Linea_Mini);
  oled.println(F("temperature"));
  oled.println(F("display    "));
  delay(2000);
  oled.clear();
}

void get_Voltage(void)
{
  //measure voltage and current
  bus_Voltage_V = (ina219_monitor.getBusVoltage_V());
  bus_Voltage_mV = bus_Voltage_V*1000;
  //convert to text
  dtostrf(bus_Voltage_mV, 7, 1, volt_String);
  // debug display
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print(volt_String);
  Serial.println(F(" mV"));
  #endif
}

void display_Values(void) {
  oled.setRow(0);
  oled.setCol(0);
  oled.print(temperature_String_V1);
  oled.println(F(" *C"));
  oled.print(volt_String);
  oled.println(F(" mV"));
  oled.setRow(5);
  oled.print(F(" "));
  oled.print(str_Linea_Mini);  
}

void display_Values_V2(void) {
  oled.setRow(0);
  oled.setCol(0);
  oled.print(temperature_String_V1);
  oled.println(F(" *C"));
  oled.print(temperature_String_V2);
  oled.println(F(" *C"));
  oled.setRow(5);
  oled.print(volt_String);
  oled.println(F(" mV"));  
}

void calculate_Temperature_V1(void) {
  steinhart = (SERIESRESISTOR_V1 / (VOLTAGEREF - bus_Voltage_V))/ THERMISTORNOMINAL_V1;     // (R/Ro) Steinhart resistance/Nominal resistance
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT_V1;                   // 1/B * ln(R/Ro)
  steinhart += (1.0 / (TEMPERATURENOMINAL + 273.15)); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  calc_Temperature_V1 = (float) steinhart - 273.15;                         // convert to C
  dtostrf(calc_Temperature_V1, 7, 1, temperature_String_V1);
  // debug display
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print(temperature_String_V1);
  Serial.println(F(" *C"));
  #endif
}

void calculate_Temperature_V2(void) {
  thermistor_Res = SERIESRESISTOR_V2 * (1/((LMREF / bus_Voltage_V) -1));
  steinhart = thermistor_Res / THERMISTORNOMINAL_V2;
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT_V2;                   // 1/B * ln(R/Ro)
  steinhart += (1.0 / (TEMPERATURENOMINAL + 273.15)); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  calc_Temperature_V2 = (float) steinhart - 273.15;                         // convert to C
  dtostrf(calc_Temperature_V2, 7, 1, temperature_String_V2);
  // debug display
  #ifdef SERIAL_DEBUG_ENABLED
  Serial.print(temperature_String_V1);
  Serial.println(F(" *C"));
  #endif
}