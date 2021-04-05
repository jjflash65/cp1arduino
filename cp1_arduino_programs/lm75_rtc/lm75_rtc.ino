/* -------------------------------------------------------
                         lm75_rtc.ino

     Liest LM75 Temperatursensor aus und zeigt diese
     auf der 7-Segmentanzeige an.

     05.04.2021  R. Seelig
   ------------------------------------------------------ */

#include "cp1_tm1637.h"   
#include "cp1_i2c.h"
#include "cp1_rtc.h"


swi2c           i2c(P2_0, P2_1);     // Objekt Software-I2C : i2c(sda, scl)


#define lm75_addr           0x90
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

/* --------------------------------------------------
                       lm75_read

     Liest einen eventuell angeschlossenen
     LM75 - Sensor aus und gibt die Temperatur multi-
     pliziert mit 10 als Argument zurueck.

     Ist kein LM75 am I2C Bus angeschlossen, wird als
     Wert -127 zurueck gegeben.
   -------------------------------------------------- */
int lm75_read(void)
{
  char       ack;
  char       t1;
  uint8_t    t2;
  int        lm75temp;

  ack= i2c.start(lm75_addr);                // LM75 Basisadresse
  if (ack)
  {

    i2c.write(0x00);                        // LM75 Registerselect: Temp. auslesen
    i2c.write(0x00);

    i2c.stop();
    _delay_us(200);
    i2c.sendstart();

    i2c.write(lm75_addr | 1);               // LM75 zum Lesen anwaehlen
    delay(1);                               // Reaktionszeit LM75
    t1= 0;
    t1= i2c.read_ack();                     // hoeherwertigen 8 Bit
    delay(1);
    t2= i2c.read_nack();                    // niederwertiges Bit (repraesentiert 0.5 Grad)
    i2c.stop();

  }
  else
  {
    i2c.stop();
    return -127;                            // Abbruch, Chip nicht gefunden
  }

  lm75temp= t1;
  lm75temp = lm75temp*10;
  if (t2 & 0x80) lm75temp += 5;             // wenn niederwertiges Bit gesetzt, sind das 0.5 Grad
  return lm75temp;
}


/* --------------------------------------------------
                       showtemp

     zeigt Temperatur
   -------------------------------------------------- */
void showtemp(int temp)
{
  uint16_t z;

  tm16.setbmp(0, 0);
  tm16.setbmp(1, 0);

  if (temp< 0)
  {
    temp= temp * (-1);
    tm16.setbmp(1,0x40);    // Minuszeichen
  }
  z= temp / 100;
  tm16.setzif(2, z);
  temp-= (z*100);
  z= temp / 10;
  tm16.setzif_dp(3,z);
  temp -= (z*10);
  tm16.setzif(4, temp % 10);
  tm16.setbmp(5, 0x63);
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
  static uint8_t sekcx = 0;
  int i;


  delay(250);
  rtc.readdate();
  
  if (date.sek != oldsek)
  {
    oldsek= date.sek;

    if (sekcx < 8)
    {
      showtime();    
      rtc.readdate();
    }  
    else
    {
      i= lm75_read();
      showtemp(i);        
    }
      
    sekcx++;
    sekcx = sekcx % 16;    
  }  

  // Shift 8 aktiviert "Uhr stellen"
  if (tm16.readshiftkeys(1,1)== 0x88)     // Shift - 8 = Input
  {
    stellen();
    sekcx= 0;
  }
      
}
