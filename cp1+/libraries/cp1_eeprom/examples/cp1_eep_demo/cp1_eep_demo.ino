/* ---------------------------------------------------------------------------
                                cp1_eep_demo.ino

     Grundfunktionen fuer das Lesen eines 24LCxxx EEProms mit Bitbanging-
     I2C Interface

     27.01.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include <avr/io.h>
#include "cp1_tm1637.h"
#include "cp1_eeprom.h"


// Belegung CP1+ Board :
//    SCL = A5
//    SDA = A4
//    Shift-Taste = D5

tm1637  tm16(A5, A4, 5);
eepr    eep;

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup()
{
  tm16.clear();
  tm16.setbright(2);
}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop()
{
  uint8_t ack;
  uint16_t addr;
  uint8_t data;
  uint8_t key;
  
  ack= eep.check_eeprom();

  // Check ob EEProm vorhanden ist
  if (!ack)                     
  {
    tm16.puts(0,"no I2C");  
    while(1);
  }
  
  while(1)
  {
    addr= tm16.input(0x77, &key);       // zu beschreibende Adresse einlesen,   0x77 = A 
    if (addr != 0)                      // Adresse 0 in dieser Demo nicht beschreiben
    {
      data= tm16.input(0x5e, &key);     // zu schreibendes Datum einlesen,    0x5e = d
      eep.write(addr, data);
    }  
    addr= tm16.input(0x50, &key);     // zu schreibendes Datum einlesen,    0x50 = r    
    data= eep.read(addr);
    tm16.clear();
    tm16.setdez(data, 0, 0);
    delay(2000);
  }
}

