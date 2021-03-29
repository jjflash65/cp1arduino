/* ---------------------------------------------------------------------------
                                cp1_radio.ino

     UKW-Empfaenger Demoprogramm mit RDA5807 Chip

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include "Arduino.h"

#include "cp1_tm1637.h"
#include "cp1_rda5807.h"


// Belegung CP1+ Board :
//    SCL = A5
//    SDA = A4
//    Shift-Taste = D5
  tm1637  tm16(A5, A4, 5);

// I2C Anschluss
//    SDA = P2_1
//    SCL = P2_0
  rda5807 ukw(P2_1, P2_0);

/*  ---------------------------------------------------------
                               setup
    --------------------------------------------------------- */
void setup()
{
  delay(300);
  ukw.reset();
  delay(100);
  ukw.poweron();
  delay(100);
  ukw.setmono();
  ukw.setfreq(1018);
  ukw.setvol(2);

  tm16.clear();
  tm16.setbright(2); 
  
  tm16.setdez(ukw.aktfreq, 1,0);  

}

/*  ---------------------------------------------------------
                              loop
    --------------------------------------------------------- */
void loop()
{
  uint8_t key;
  uint16_t aktfreq;
  uint16_t aktvol;

  key= tm16.readshiftkeys(1,0);
  switch (key)
  {
    // Sendersuchlauf abnehmende Frequenz
    case 0 :
    {
      tm16.clear();
      tm16.setzif(0,0x0d);
      ukw.scandown();
      tm16.setdez(ukw.aktfreq, 1,0); 
      break;
    }
    // Sendersuchlauf zunehmende Frequenz
    case 1 :
    {
      tm16.clear();
      tm16.setbmp(0,0x1c);
      ukw.scanup();
      tm16.setdez(ukw.aktfreq, 1,0); 
      break;
    }
    // manuelle Frequenzeingabe
    case 2 : 
    {
      ukw.aktfreq= tm16.input(0x71, &key);
      ukw.setfreq(ukw.aktfreq);
      tm16.setdez(ukw.aktfreq, 1,0); 
      break;
    }
    // manuelle Lautstaerkeeingabe
    case 3 : 
    {
      ukw.aktvol= tm16.input(0x38, &key);
      ukw.setvol(ukw.aktvol);
      tm16.setdez(ukw.aktfreq, 1,0); 
      break;
    }
    // Anzeige Lautstaerke
    case 4 : 
    {
      tm16.setdez(ukw.aktvol, 0,0); 
      delay(1000);
      tm16.setdez(ukw.aktfreq, 1,0); 
      break;
    }
    default : break;
  }
}
