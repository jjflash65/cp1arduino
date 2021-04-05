/* -------------------------------------------------------
                         cp1_rtc.h

     Libraryheader fuer DS1307 / DS3231 Realtime-
     Clock-Chip

     26.03.2021  R. Seelig
   ------------------------------------------------------ */


#include <avr/io.h>
#include <avr/interrupt.h>
  
#include "Arduino.h"   
#include "cp1_i2c.h"

// Software - I2C : i2c(sda, scl)
extern swi2c i2c;

extern struct my_datum date;  

#define rtc_addr            0xd0              // 8-Bit I2C Adresse: R/W Flag ist Bestandteil der Adresse !

// Software - I2C : i2c(sda, scl)
extern swi2c i2c;

struct my_datum                               // Datum- und Uhrzeitsstruktur
{
  uint8_t jahr;
  uint8_t monat;
  uint8_t tag;
  uint8_t dow;
  uint8_t std;
  uint8_t min;
  uint8_t sek;
};

class realtimeclock
{
  public: 
    
    realtimeclock();
    uint8_t read(uint8_t addr);
    void write(uint8_t addr, uint8_t value);
    uint8_t getwtag(void);
    void readdate(void);
    void writedate(void);
  
  protected:
  
  private:
    uint8_t bcd2dez(uint8_t value);
    uint8_t dez2bcd(uint8_t value);
};

