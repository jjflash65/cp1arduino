/* ---------------------------------------------------------------------------
                                cp1_tm1637.h
                 
     Grundfunktionen des TM1637 7-Segment- und Keyboardcontrollers auf 
     dem CP1+ Board
     
     27.01.2021    R. Seelig                      
   --------------------------------------------------------------------------- */

#ifndef in_cp1tm1637
#define in_cp1tm1637

#include "Arduino.h"
#include <avr/pgmspace.h>


class tm1637
{
  #define scl_init()    pinMode(tm16_scl, INPUT)
  #define sda_init()    pinMode(tm16_sda, INPUT)
  #define shift_init()  { pinMode(shift_key, INPUT); digitalWrite(shift_key, HIGH); }
  #define bb_scl_hi()   scl_init()  
  #define bb_scl_lo()   { pinMode(tm16_scl, OUTPUT);  digitalWrite(tm16_scl, LOW); }  
  #define bb_sda_hi()   sda_init()  
  #define bb_sda_lo()   { pinMode(tm16_sda, OUTPUT);  digitalWrite(tm16_sda, LOW); }  
  #define bb_is_sda()   digitalRead(tm16_sda)
  #define is_shift()    !digitalRead(shift_key)

  #define puls_us        0
  #define puls_len()     _delay_us(puls_us)  

public:
  tm1637(uint8_t scl, uint8_t sda, uint8_t shift);
//  tm1637();
  void clear();  
  void setbright(uint8_t value);
  void setbmp(uint8_t pos, uint8_t value);
  void setascii(uint8_t pos, uint8_t ch);
  void setzif(uint8_t pos, uint8_t value);
  void setzif_dp(uint8_t pos, uint8_t value);  
  void setdez(int32_t value, uint8_t komma, uint8_t leading); 
  void sethex(uint32_t value, uint8_t digits);
  void setseg(uint8_t digit, uint8_t seg);
  void clrseg(uint8_t digit, uint8_t seg);
  uint8_t readkey(void);  
  uint8_t readshiftkeys(uint8_t wait_unpress, uint8_t wait_shiftunpress);
  uint32_t input(uint8_t bmp, uint8_t *endkey);  
  void puts(uint8_t pos, char *s);
  void puts_rom(uint8_t pos, const PROGMEM uint8_t *s);

protected:  

private:
  uint8_t   hellig     = 3;                 // beinhaltet Wert fuer die Helligkeit (erlaubt: 0x00 .. 0x07);
  uint8_t   segbuf     = 0;
  uint8_t   tm16_scl;
  uint8_t   tm16_sda;
  uint8_t   shift_key;

  void start ();
  void stop ();    
  void write (uint8_t value);
  uint8_t read(uint8_t ack);

  void selectpos(char nr);  
};


#endif
