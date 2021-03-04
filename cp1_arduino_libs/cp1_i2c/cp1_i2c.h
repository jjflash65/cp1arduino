/* ---------------------------------------------------------------------------
                                cp1_i2c.h

     Rudimentaere I2C Funktionen mittels Bitbanging (Software) realisiert

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */
   
#ifndef in_cp1_i2c
  #define in_cp1_i2c

  #include <string.h>
  
  #include "Arduino.h"
  #include <avr/io.h>
  #include <util/delay.h>
  
  class swi2c
  {
    #define i2c_scl_hi()   pinMode(scl, INPUT)
    #define i2c_scl_lo()   { pinMode(scl, OUTPUT); digitalWrite(scl, LOW); }
  
    #define i2c_sda_hi()   pinMode(sda, INPUT)
    #define i2c_sda_lo()   { pinMode(sda, OUTPUT); digitalWrite(sda, LOW); }
    #define i2c_is_sda()   digitalRead(sda)
  
    #define short_puls     1            // Einheiten fuer einen langen Taktimpuls
    #define long_puls      1            // Einheiten fuer einen kurzen Taktimpuls
    #define del_wait       1            // Wartezeit fuer garantierten 0 Pegel SCL-Leitung
  
    #define short_del()     delay(short_puls)
    #define long_del()      delay(long_puls)
    #define wait_del()      delay(del_wait)
  
    public:
  
      uint8_t sda, scl;
  
      swi2c(uint8_t da, uint8_t cl);
      void setpins(uint8_t da, uint8_t cl);
      void delay(uint16_t anz);
      void sendstart(void);
      uint8_t start(uint8_t addr);
      void stop();
      void startaddr(uint8_t addr, uint8_t rwflag);
      void write_nack(uint8_t data);
      uint8_t write(uint8_t data);
      uint8_t write16(uint16_t data);
      uint8_t read(uint8_t ack);
      uint8_t read_ack();
      uint8_t read_nack();
  
    protected:
  
    private:
  
  };
  
#endif
