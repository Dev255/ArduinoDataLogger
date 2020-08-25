
/*
Neil Devonshire - Dev255
YouTube Dev255
www.dev255.co.uk

Arduino Nano Data Receiver and LCD display of 10 temperature sensors

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

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <LiquidCrystal.h>

//Define a variable adata to store the final result.

// lcd Pin setup
const int rs = 2;
const int rw = 3;
const int e = 4;
const int db4 = 5;
const int db5 = 6;
const int db6 = 7;
const int db7 = 8;
LiquidCrystal lcd(rs, rw, e, db4, db5, db6, db7);
byte degree[8] = {
  B01111,
  B01001,
  B01111,
  B00000,
  B00000,
  B00000,
  B00000,
  };

unsigned int adata = 0;
float temperature = 0;
bool receiveFlag = false;
unsigned long redundantTime = 0;

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
float dome_difference = 0;
float panel_difference = 0;
int signalHeartBeat = 0;
long dome_Watts = 0;
long panel_Watts = 0;
bool pumpOn = false;

void setup()
{
Serial.begin(9600);

  // Set the lcd pins mode
  pinMode(rs, OUTPUT);
  pinMode(rw, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(db4, OUTPUT);
  pinMode(db5, OUTPUT);
  pinMode(db6, OUTPUT);
  pinMode(db7, OUTPUT);
  lcd.createChar(0, degree);

//---------Initial part, can't be modified at any time---------
Mirf.cePin = 9; //Set CE Pin to D9
Mirf.csnPin = 10; //Set CE Pin to D10
Mirf.spi = &MirfHardwareSpi;
Mirf.init(); //initialization nRF24L01

//---------Configuration part, can be modified it at any time---------
//Set the receiving identifier "Rev01"
Mirf.setRADDR((byte *)"Rec01");
//Set the number of bytes sent and received at a time, here sent an integer.
//Write sizeof(unsigned int), which is actually equal to 2 bytes.
Mirf.payload = sizeof(unsigned int);
// Sending channel, can fill 0~128, send and receive must be consistent.
Mirf.channel = 124;
Mirf.config();

//Note that one Arduino writes Sender.ino and the other writes Receiver.ino.
//To identify the program written in Receiver.ino.

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("  Hello World   ");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("    Receiver    ");
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("    Turn on     ");
  lcd.setCursor(0, 1);
  lcd.print("  Transmitter   ");
  
  if(!Mirf.dataReady()){
    while(!Mirf.dataReady()){
      lcd.setCursor(15,0);
      lcd.print("|");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("}");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print(")");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("}");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("|");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("{");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("(");
      delay(200);
      lcd.setCursor(15,0);
      lcd.print("{");
      delay(200);
    }
  }
  lcd.clear();
}


void loop(){

  if(Mirf.dataReady()){
    receiveFlag = true;
    RcvData();
    redundantTime = millis();
    Serial.println(redundantTime/1000);
    lcd.setCursor(0, 0);
    lcd.print("P=");
    lcd.setCursor(7, 0);
    lcd.print(" O=");
    lcd.setCursor(0, 1);
    lcd.print("D=");
    lcd.setCursor(7, 1);
    lcd.print(" H=");
    dome_difference = dome_out_TempC - dome_in_TempC;
    panel_difference = panel_out_TempC - panel_in_TempC;
    dome_Watts = dome_difference * 637;
    panel_Watts = panel_difference * 287.586;
    if (dome_Watts < 0 || pumpOn == false){
      lcd.setCursor(2, 1);
      lcd.print("OFF ");  
    }else{
      lcd.setCursor(2, 1);
      lcd.print(dome_Watts);
      lcd.print("W  ");      
    }
    if (panel_Watts < 0 || pumpOn == false){
      lcd.setCursor(10, 1);
      lcd.print("OFF "); 
    }else{
      lcd.setCursor(10, 1);
      lcd.print(panel_Watts);
      lcd.print("W ");      
    }

  }
  if(redundantTime+3000 < millis()){
    lcd.setCursor(0, 0);
    lcd.print("  Transmission  ");
    lcd.setCursor(0, 1);
    lcd.print("      Lost      ");
  }
}

void RcvData(void){
  byte data[Mirf.payload];
  while(receiveFlag == true){
    if(Mirf.dataReady()){
      Mirf.getData(data); //Receive data to data array.
      adata = (unsigned int)((data[1] << 8) | data[0]);
      switch (adata){
        case 1:
          while(adata == 1){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              panel_in_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Panel In Temp =      ");
                Serial.println("No Probe Found");
                lcd.setCursor(0, 0);
                lcd.print("NP ");
              }else{
                Serial.print("Panel In Temp =      ");
                Serial.print(panel_in_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(0, 0);
                //lcd.print(panel_in_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 2:
          while(adata == 2){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              panel_out_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Panel Out Temp =     ");
                Serial.println("No Probe Found");
                lcd.setCursor(3, 0);
                lcd.print("NP ");
              }else{
                Serial.print("Panel Out Temp =     ");
                Serial.print(panel_out_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(3, 0);
                //lcd.print(panel_out_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 3:
          while(adata == 3){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              panel_cntr_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Panel Center Temp =  ");
                Serial.println("No Probe Found");
                lcd.setCursor(6, 0);
                lcd.print("NP ");
              }else{
                Serial.print("Panel Center Temp =  ");
                Serial.print(panel_cntr_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(6, 0);
                //lcd.print(panel_cntr_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 4:
          while(adata == 4){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              dome_in_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Dome In Temp =       ");
                Serial.println("No Probe Found");
                lcd.setCursor(9, 0);
                lcd.print("NP ");
              }else{
                Serial.print("Dome In Temp =       ");
                Serial.print(dome_in_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(9, 0);
                //lcd.print(dome_in_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 5:
          while(adata == 5){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              dome_out_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Dome Out Temp =      ");
                Serial.println("No Probe Found");
                lcd.setCursor(12, 0);
                lcd.print("NP ");
              }else{
                Serial.print("Dome Out Temp =      ");
                Serial.print(dome_out_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(12, 0);
                //lcd.print(dome_out_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 6:
          while(adata == 6){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              dome_cntr_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Dome Center Temp =   ");
                Serial.println("No Probe Found");
                lcd.setCursor(0, 1);
                lcd.print("NP ");
              }else{
                Serial.print("Dome Center Temp =   ");
                Serial.print(dome_cntr_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(0, 1);
                //lcd.print(dome_cntr_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 7:
          while(adata == 7){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              elect_box_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Electric Box Temp =  ");
                Serial.println("No Probe Found");
                lcd.setCursor(3, 1);
                lcd.print("NP ");
              }else{
                Serial.print("Electric Box Temp =  ");
                Serial.print(elect_box_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(3, 1);
                //lcd.print(elect_box_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 8:
          while(adata == 8){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              outside_shd_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Outside Shade Temp = ");
                Serial.println("No Probe Found");
                lcd.setCursor(6, 1);
                lcd.print("NP ");
              }else{
                Serial.print("Outside Shade Temp = ");
                Serial.print(outside_shd_TempC, 3);
                Serial.println("ºC");
                lcd.setCursor(10, 0);
                lcd.print(outside_shd_TempC, 1);
                lcd.print(" ");
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 9:
          while(adata == 9){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              outside_sun_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Outside Sun Temp =   ");
                Serial.println("No Probe Found");
                lcd.setCursor(9, 1);
                lcd.print("NP ");
              }else{
                Serial.print("Outside Sun Temp =   ");
                Serial.print(outside_sun_TempC, 3);
                Serial.println("ºC");
                //lcd.setCursor(9, 1);
                //lcd.print(outside_sun_TempC, 0);
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 10:
          while(adata == 10){
            if(Mirf.dataReady()){
              Mirf.getData(data); //Receive data to data array.
              adata = (unsigned int)((data[1] << 8) | data[0]);
              pool_water_TempC = (float) adata/128;
              if (adata == 58496){
                Serial.print("Pool Water Temp =    ");
                Serial.println("No Probe Found");
                lcd.setCursor(12, 1);
                lcd.print("NP ");
              }else{
                Serial.print("Pool Water Temp =    ");
                Serial.print(pool_water_TempC, 3);
                Serial.println("ºC");
                lcd.setCursor(2, 0);
                lcd.print(pool_water_TempC, 2);
                lcd.print(" ");
                //lcd.print((char)B11011111);
                //lcd.print("C");
              }
            }
          }
          break;
        case 11:
          Serial.println("Pool Pumps = ON");
          lcd.setCursor(15, 0);
          lcd.print("P");
          pumpOn = true;
          break;
        case 12:
          Serial.println("Pool Pumps = OFF");
          lcd.setCursor(15, 0);
          lcd.print("_");
          pumpOn = false;
          break;
        case 13:
          Serial.println("Electric Box Heater = ON");
          lcd.setCursor(15, 1);
          lcd.print("H");
          break;
        case 14:
          Serial.println("Electric Box Heater = OFF");
          lcd.setCursor(15, 1);
          lcd.print("_");
          break;
        case 65534:
          receiveFlag = false;
          if (signalHeartBeat == 0){
            lcd.setCursor(15, 1);
            lcd.print((char)B01011100);
            signalHeartBeat = 1;
          }else{
            signalHeartBeat = 0;
          }
          break;
        defult:
          break;
      }
    }
  }
}
