/*  ---------------------------------------------------------
                      cp1_tm1637_demo.ino

      Demoprogramm fuer TM1637 auf dem CP1+ Board.

      27.01.2021    R. Seelig                             
    --------------------------------------------------------- */

#include "cp1_tm1637.h"

#define LED  P2_7

// Belegung CP1+ Board :
//    SCL = A5
//    SDA = A4
//    Shift-Taste = D5

tm1637  tm16(A5, A4, 5);

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{
  pinMode(LED, OUTPUT);
  tm16.clear();
  tm16.setbright(2);
}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  uint8_t key;
  uint32_t counter;
  uint16_t my_speed = 100;
  uint8_t i;

  // blinkendes Hallo
  for (i= 0; i< 5; i++)
  {    
    tm16.puts(0, "HALLO");
    delay(500);    
    tm16.clear();
    delay(500);    
  }  

  // Countdown-Zaehler
  // Erste Eingabe  = Startwert des Zaehlers
  // Zweite Eingabe = Zaehlgeschwindigkeit
 
  counter= tm16.input(0x54, &key);
  my_speed= tm16.input(0x6d, &key);
  
  // Countdown
  while (counter)
  {
    tm16.setdez(counter, 0,0); 
    tm16.setseg(0, counter % 6);
    digitalWrite(LED, HIGH);  
    delay(my_speed);                     
    digitalWrite(LED, LOW);    
    delay(my_speed); 
    tm16.clrseg(0, counter % 6);
    counter--;
    
    // mit Shift-STP kann der Countdown beendet werden
    if (tm16.readshiftkeys(0,0) == 0x82) counter= 0;
  }
  tm16.setdez(counter, 0,0); 
  tm16.setseg(0, counter % 6);  
  delay(2000);  
  tm16.clear();
}
