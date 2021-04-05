/* -------------------------------------------------------
                         rtc_demo.c

     Liest DS1307 / DS3231 (Realtime-Clock-Chip) aus und 
     zeigt diese auf der seriellen  Schnittstelle an

     26.03.2021  R. Seelig
   ------------------------------------------------------ */

#include "cp1_tm1637.h"   
#include "cp1_i2c.h"
#include "cp1_rtc.h"

swi2c           i2c(P2_0, P2_1);     // Objekt Software-I2C : i2c(sda, scl)
realtimeclock   rtc;                 // Objekt rtc

// tm1637 (Tastatur und 7-Segmentanzeige Chip) :
//    SCL = A5
//    SDA = A4
//    Shift-Taste = D5

tm1637          tm16(A5, A4, 5);     // Objekt Tasten- und Segmentanzeigentreiber

/* --------------------------------------------------
     stellen

     die Uhr benutzerabgefragt stellen
   -------------------------------------------------- */
void stellen(void)
{
  uint8_t key;
  
  date.std= tm16.input(0x01, &key);    // Stunden einlesen
  date.min= tm16.input(0x40, &key);    // Minuten einlesen
  date.sek= tm16.input(0x08, &key);    // Sekunden einlesen
  rtc.writedate();

}

/* --------------------------------------------------
     showtime

     zeigt die Uhrzeit an
   -------------------------------------------------- */
void showtime(void)
{
  tm16.setzif(0,date.std /10);
  tm16.setzif_dp(1,date.std % 10);
  
  tm16.setzif(2,date.min /10);
  tm16.setzif_dp(3,date.min % 10);
  
  tm16.setzif(4,date.sek /10);
  tm16.setzif(5,date.sek % 10);
}

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{ 
  uint8_t ch;

  tm16.clear();
  tm16.setbright(2);

  rtc.readdate();
  showtime(); 

}

  
/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  static uint8_t oldsek = 0;
  static uint8_t cx = 0;
    
  delay(250);
  rtc.readdate();
  
  if (date.sek != oldsek)
  {
    showtime();    
    rtc.readdate();
    oldsek= date.sek;    
  }  

  // Shift 8 aktiviert "Uhr stellen"
  if (tm16.readshiftkeys(1,1)== 0x88)     // Shift - 8 = Input
  {
    stellen();
  }    
}
