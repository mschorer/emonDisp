//
// receive and display Open Energy Monitor data
//

#include <Time.h>
#include <Wire.h>
#include <DigisparkOLED.h>
#include <font6x8.h>
#include <font8x16.h>
#include "digistump_128x64c1.h"

#include "oem_types.h"

#include <SPI.h>
//#include "nRF24L01.h"
#include "RF24.h"
//#include "printf.h"

//RF24 radio(7,10);  // due
//RF24 radio(7,8);   // uno
RF24 radio(7,12);   // digispark pro

const int LED = 1;

sStatus oem;
const unsigned long DEFAULT_TIME = 1357041600;
const uint8_t pipes[][6] = { "1emon", "2emon", "3emon", "4emon", "5emon", "6emon", };

unsigned long tx_id=0;
int dots = 40;

void setup() {
  oled.begin();
  oled.clear();
  
  oled.draw( 0, 0, 128, 64, digistumplogo);
  
  oled.setCursor(0, 7);

  oled.print("emon A #");
  oled.print(sizeof( oem_power));
  oled.print("/");
  oled.print(sizeof( oem_energy));
  
  oem.power.timestamp = 0; 
  oem.power.voltage = 0;
  oem.power.realPower_CT1 = 0;
  oem.power.realPower_CT2 = 0;
  oem.power.realPower_CT3 = 0;
  oem.power.realPower_CT4 = 0; 

  oem.energy.timestamp = 0; 
  oem.energy.duration = 0; 
  oem.energy.wh_CT1 = 0;
  oem.energy.wh_CT2 = 0;
  oem.energy.wh_CT3 = 0;
  oem.energy.wh_CT4 = 0;
  
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, HIGH);
  radio.begin();
  radio.setChannel(32);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.enableAckPayload();
//  radio.enableDynamicPayloads();
//  radio.setPayloadSize( sizeof( rx_packet));
  radio.setRetries( 2, 10);                   // Optionally, increase the delay between retries & # of retries
  radio.setCRCLength(RF24_CRC_16); 
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
  
  radio.startListening();
  radio.printDetails();
  
  radio.powerUp(); 
  delay( 2500);
  oled.clear();
    
  updatePower();
  updateEnergy();
  digitalWrite(LED, LOW);
}

void loop() {
  byte pipe;
//  byte ack = 0x81;
  int pulse = 400;
  static oem_packet rx_buffer;
  uint8_t psz = 0;

  digitalWrite(LED, HIGH);

  if ( radio.available( &pipe) || radio.isAckPayloadAvailable()){       
    psz = radio.getDynamicPayloadSize();
    radio.read( &rx_buffer, psz);
    
    switch( rx_buffer.packet_type) {
      case OEM_TIMESTAMP:
/*
        if ( rx_buffer.timestamp.timestamp > DEFAULT_TIME) {
          oem.
          setTime( rx_buffer.timestamp.timestamp);
          oem.ts.timestamp = rx_buffer.timestamp.timestamp;

          Serial.print( "rx time[");
          Serial.print(hour());
          printDigits(minute());
          printDigits(second());
          Serial.print(" ");
          Serial.print(day());
          Serial.print(" ");
          Serial.print(month());
          Serial.print(" ");
          Serial.print(year()); 
          Serial.println( "]");
        }
*/
      break;

      case OEM_POWER:
        memcpy( &( oem.power), &rx_buffer.power, sizeof( oem_power));
        updatePower();
      break;
      
      case OEM_ENERGY:
        memcpy( &( oem.energy), &rx_buffer.energy, sizeof( oem_energy));
        updateEnergy();
      break;
      
      default:
        Serial.print( "rx[");
        Serial.print( rx_buffer.packet_type);
        Serial.print( " #");
        Serial.print( psz);
        Serial.println( "]");
    }

    byte ack = OEM_NOP;
    radio.writeAckPayload( pipe, &ack, 1);   
  } else {
    delay( 100);
  }
  digitalWrite(LED, LOW);
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

float conv( int val) {
  return val;  //((float) val) / 256.0;
}

void updateEnergy() {
    char s[20];
    oled.setFont(FONT6X8);

    oled.setCursor(72, 0);  
    sprintf( s, "#%08ld", oem.energy.timestamp+oem.energy.duration);
    oled.print( s);

    oled.setCursor(60, 4);  
    sprintf( s, "%8ld Wh", oem.energy.wh_CT1);
    oled.print( s);     

    oled.setCursor(60,5);  
    sprintf( s, "%8ld Wh", oem.energy.wh_CT2);
    oled.print( s);     

    oled.setCursor(60, 6);  
    sprintf( s, "%8ld Wh", oem.energy.wh_CT3);
    oled.print( s);     

    oled.setCursor(60, 7);  
    sprintf( s, "%8ld Wh", oem.energy.wh_CT4);
    oled.print( s);     

    oled.setFont(FONT8X16);
    oled.setCursor(54, 1);  
    sprintf( s, "%6ldkWh", ( oem.energy.wh_CT1+oem.energy.wh_CT2+oem.energy.wh_CT3+oem.energy.wh_CT4) / 1000);
    oled.print(s);     
}

void updatePower() {
    char s[20];
    oled.setFont(FONT6X8);
/*    
    oled.setCursor(0, 0);  
    sprintf( s, "#%06d", oem.power.timestamp);
    oled.print( s);
*/
    oled.setCursor( 22, 0);  
    sprintf( s, "%3dV", oem.power.voltage);
    oled.print( s);

    oled.setCursor(6, 4);  
    sprintf( s, "%5d W", oem.power.realPower_CT1);
    oled.print( s);     

    oled.setCursor(6, 5);  
    sprintf( s, "%5d W", oem.power.realPower_CT2);
    oled.print( s);     

    oled.setCursor(6, 6);  
    sprintf( s, "%5d W", oem.power.realPower_CT3);
    oled.print( s);     

    oled.setCursor(6, 7);  
    sprintf( s, "%5d W", oem.power.realPower_CT4);
    oled.print( s);     

    oled.setFont(FONT8X16);
    oled.setCursor(0, 1);  
    sprintf( s, "%5uW", oem.power.realPower_CT1+oem.power.realPower_CT2+oem.power.realPower_CT3+oem.power.realPower_CT4);
    oled.print(s);     
}
