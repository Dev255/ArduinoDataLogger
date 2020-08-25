
/*
Neil Devonshire - Dev255
YouTube Dev255
www.dev255.co.uk

Arduino Nano Data logger and transmitter of 10 temperature sensors

    Copyright (C) <2020>  <Neil Devonshire>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
*/

#include <SD.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <OneWire.h>  // Required for temperature sensors
#include <DallasTemperature.h>

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
const int chipSelect = 4;
const int pumpRelay = 3;
const int heaterRelay = 2;

OneWire  ds_a(6);  // Temperature sensor pins
OneWire  ds_b(7);  // Temperature sensor pins
DallasTemperature sensors_a(&ds_a);
DallasTemperature sensors_b(&ds_b);

DeviceAddress panel_in =    { 0x28, 0x9C, 0x25, 0x1A, 0x28, 0x19, 0x01, 0x6A };
DeviceAddress panel_out =   { 0x28, 0xC8, 0xA4, 0x0D, 0x28, 0x19, 0x01, 0x20 };
DeviceAddress panel_cntr =  { 0x28, 0xAE, 0xA3, 0x15, 0x28, 0x19, 0x01, 0x24 };
DeviceAddress dome_in =     { 0x28, 0x2A, 0xB9, 0x1F, 0x28, 0x19, 0x01, 0xC8 };
DeviceAddress dome_out =    { 0x28, 0x21, 0x5C, 0x13, 0x28, 0x19, 0x01, 0x2A };
DeviceAddress dome_cntr =   { 0x28, 0x27, 0x25, 0x19, 0x28, 0x19, 0x01, 0x1D };
DeviceAddress elect_box =   { 0x28, 0xDA, 0x19, 0x15, 0x28, 0x19, 0x01, 0x92 };
DeviceAddress outside_shd = { 0x28, 0xA7, 0x1C, 0x22, 0x28, 0x19, 0x01, 0x5C };
DeviceAddress outside_sun = { 0x28, 0xDF, 0x83, 0x13, 0x28, 0x19, 0x01, 0x7A };
DeviceAddress pool_water =  { 0x28, 0x07, 0x0C, 0x0B, 0x28, 0x19, 0x01, 0x9F };

File poolTemp;

void setup()
{

pinMode (pumpRelay, OUTPUT);
pinMode (heaterRelay, OUTPUT);
digitalWrite (pumpRelay, HIGH);
digitalWrite (heaterRelay, HIGH);

Serial.begin(9600);
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  poolTemp = SD.open("pooltemp.txt", FILE_WRITE);
  if (poolTemp) {
    Serial.print("Writing to pooltemp.txt...");
    poolTemp.println("");
    poolTemp.println("---- NEW DATA ENTRY ----");
    poolTemp.println("");
  // close the file:
    poolTemp.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening pooltemp.txt");
  }
    
sensors_a.begin ();
sensors_b.begin ();

Serial.println ("");
Serial.print (".......On a found ");
Serial.print (sensors_a.getDeviceCount(), DEC);
Serial.println (" DS18B20 devices ..........");
Serial.println ("");
Serial.print (".......On b found ");
Serial.print (sensors_b.getDeviceCount(), DEC);
Serial.println (" DS18B20 devices ..........");
Serial.println ("");
delay(1000);

sensors_a.setResolution (panel_in, 12);
sensors_a.setResolution (panel_out, 12);
sensors_a.setResolution (panel_cntr, 12);
sensors_a.setResolution (dome_in, 12);
sensors_a.setResolution (dome_out, 12);
sensors_a.setResolution (dome_cntr, 12);
sensors_a.setResolution (elect_box, 12);
sensors_a.setResolution (outside_shd, 12);
sensors_a.setResolution (outside_sun, 12);
sensors_a.setResolution (pool_water, 12);

delay(2000);

Mirf.cePin = 9; //Set the CE pin to D9
Mirf.csnPin = 10; //Set the CE pin to D10
Mirf.spi = &MirfHardwareSpi;
Mirf.init(); //initialization nRF24L01

//Set the receiving identifier "Sen01"

Mirf.setRADDR((byte *)"Sen01");

//Set the number of bytes sent and received at a time, here send an integer, write sizeof (unsigned int), actually equal to 2 bytes

Mirf.payload = sizeof(unsigned int);

//Sending channel, can fill 0~128, send and receive must be consistent.

Mirf.channel = 124;

Mirf.config();

//Note that one Arduino writes Sender.ino and the other writes Receiver.ino.
//The identifier here is written to Sender.ino

Serial.println("I'm Sender...");
}

unsigned int adata = 0;
int panel_in_Temp = 0;
int panel_out_Temp = 0;
int panel_cntr_Temp = 0;
int dome_in_Temp = 0;
int dome_out_Temp = 0;
int dome_cntr_Temp = 0;
int elect_box_Temp = 0;
int outside_shd_Temp = 0;
int outside_sun_Temp = 0;
int pool_water_Temp = 0;
float panel_in_TempC = 0;
float panel_out_TempC = 0;
float panel_cntr_TempC = 0;
float dome_in_TempC = 0;
float dome_out_TempC = 0;
float dome_cntr_TempC = 0;
float elect_box_TempC = 0;
float outside_shd_TempC = 0;
float outside_sun_TempC = 0;
float pool_water_TempC = 0;

int pumpRelayVal = 0;
int heaterRelayVal = 0;

unsigned long timeStamp = 0;
unsigned long logTime = 0;

void loop(){
  
  timeStamp = millis();
  if (logTime+14000 < timeStamp){
    while(logTime+15000 > timeStamp){
      timeStamp = millis();    
      }
    logTime = timeStamp;
    LogData();
  }
  Serial.print("Seconds: ");
  Serial.println(timeStamp/1000);
  
sensors_a.requestTemperatures ();
sensors_b.requestTemperatures ();

adata = 65535;  // Max 65535 (16 bit)
TransmitData();
panel_in_Temp = sensors_a.getTemp (panel_in);
adata = 1;
TransmitData();
adata = panel_in_Temp;  // Max 65535 (16 bit)
TransmitData();
panel_in_TempC = (float) panel_in_Temp/128;
//Serial.print ("Panel In = "); Serial.print (panel_in_TempC, 3); Serial.println ("ºC");
panel_out_Temp = sensors_b.getTemp (panel_out);
adata = 2;
TransmitData();
adata = panel_out_Temp;  // Max 65535 (16 bit)
TransmitData();
panel_out_TempC = (float) panel_out_Temp/128;
//Serial.print ("Panel Out = "); Serial.print (panel_out_TempC, 3); Serial.println ("ºC");
panel_cntr_Temp = sensors_b.getTemp (panel_cntr);
adata = 3;
TransmitData();
adata = panel_cntr_Temp;  // Max 65535 (16 bit)
TransmitData();
panel_cntr_TempC = (float) panel_cntr_Temp/128;
//Serial.print ("Panel Center = "); Serial.print (panel_cntr_TempC, 3); Serial.println ("ºC");
dome_in_Temp = sensors_a.getTemp (dome_in);
adata = 4;
TransmitData();
adata = dome_in_Temp;  // Max 65535 (16 bit)
TransmitData();
dome_in_TempC = (float) dome_in_Temp/128;
//Serial.print ("Dome In = "); Serial.print (dome_in_TempC, 3); Serial.println ("ºC");
dome_out_Temp = sensors_b.getTemp (dome_out);
adata = 5;
TransmitData();
adata = dome_out_Temp;  // Max 65535 (16 bit)
TransmitData();
dome_out_TempC = (float) dome_out_Temp/128;
//Serial.print ("Dome Out = "); Serial.print (dome_out_TempC, 3); Serial.println ("ºC");
dome_cntr_Temp = sensors_a.getTemp (dome_cntr);
adata = 6;
TransmitData();
adata = dome_cntr_Temp;  // Max 65535 (16 bit)
TransmitData();
dome_cntr_TempC = (float) dome_cntr_Temp/128;
//Serial.print ("Dome Center = "); Serial.print (dome_cntr_TempC, 3); Serial.println ("ºC");
elect_box_Temp = sensors_b.getTemp (elect_box);
adata = 7;
TransmitData();
adata = elect_box_Temp;  // Max 65535 (16 bit)
TransmitData();
elect_box_TempC = (float) elect_box_Temp/128;
//Serial.print ("Electric Box = "); Serial.print (elect_box_TempC, 3); Serial.println ("ºC");
outside_shd_Temp = sensors_a.getTemp (outside_shd);
adata = 8;
TransmitData();
adata = outside_shd_Temp;  // Max 65535 (16 bit)
TransmitData();
outside_shd_TempC = (float) outside_shd_Temp/128;
//Serial.print ("Outside Shade = "); Serial.print (outside_shd_TempC, 3); Serial.println ("ºC");
outside_sun_Temp = sensors_a.getTemp (outside_sun);
adata = 9;
TransmitData();
adata = outside_sun_Temp;  // Max 65535 (16 bit)
TransmitData();
outside_sun_TempC = (float) outside_sun_Temp/128;
//Serial.print ("Outside Sun = "); Serial.print (outside_sun_TempC, 3); Serial.println ("ºC");
pool_water_Temp = sensors_a.getTemp (pool_water);
adata = 10;
TransmitData();
adata = pool_water_Temp;  // Max 65535 (16 bit)
TransmitData();
pool_water_TempC = (float) pool_water_Temp/128;
//Serial.print ("Pool Water = "); Serial.print (pool_water_TempC, 3); Serial.println ("ºC");
pumpRelayVal = digitalRead(pumpRelay);
if (pumpRelayVal == LOW){
  adata = 11;  // Max 65535 (16 bit)
  TransmitData();
}
pumpRelayVal = digitalRead(pumpRelay);
if (pumpRelayVal == HIGH){
  adata = 12;  // Max 65535 (16 bit)
  TransmitData();
}
heaterRelayVal = digitalRead(heaterRelay);
if (heaterRelayVal == LOW){
  adata = 13;  // Max 65535 (16 bit)
  TransmitData();
}
heaterRelayVal = digitalRead(heaterRelay);
if (heaterRelayVal == HIGH){
  adata = 14;  // Max 65535 (16 bit)
  TransmitData();
}
adata = 65534;  // Max 65535 (16 bit)
TransmitData();
Serial.println();
Serial.println();

  if (panel_cntr_TempC > pool_water_TempC+5){
    digitalWrite(pumpRelay, LOW);
  }else{
    if (panel_cntr_TempC < pool_water_TempC+2){
      digitalWrite(pumpRelay, HIGH);
    }
  }
  if (elect_box_TempC < 5){
    digitalWrite(heaterRelay, LOW);
  }else{
    if (elect_box_TempC > 7){
      digitalWrite(heaterRelay, HIGH);
    }
  }
}

void TransmitData(){
  byte data[Mirf.payload];
  data[0] = adata & 0xFF; //low eight bits to data[0]，
  data[1] = adata >> 8; //high eight bits to data[1]。
  Mirf.setTADDR((byte *)"Rec01");
  Mirf.send(data);
  while(Mirf.isSending()) {}
  Serial.println (adata);
  delay(10);
}

void LogData (){
  poolTemp = SD.open("pooltemp.txt", FILE_WRITE);
 
  if (poolTemp) {
    Serial.print("Writing to pooltemp.txt...");
    poolTemp.println();
    poolTemp.print(timeStamp/1000);
    poolTemp.print(",");
    poolTemp.print(panel_in_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(panel_out_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(panel_cntr_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(dome_in_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(dome_out_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(dome_cntr_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(elect_box_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(outside_shd_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(outside_sun_TempC, 3);
    poolTemp.print(",");
    poolTemp.print(pool_water_TempC, 3);
    poolTemp.print(",");
    pumpRelayVal = digitalRead(pumpRelay);
    if (pumpRelayVal == LOW){
       poolTemp.print("Pmp_ON,");
    }else{
       poolTemp.print("Pmp_OFF,");
    }
    heaterRelayVal = digitalRead(heaterRelay);
    if (heaterRelayVal == LOW){
       poolTemp.print("Htr_ON,");
    }else{
       poolTemp.print("Htr_OFF,");
    }
  // close the file:
    poolTemp.close();
    Serial.println("done.");
  } else {
    Serial.println("error opening pooltemp.txt");
  }
}
