/* ---------------------------------------------------------------------------
                                cp1_eeprom.h
                 
     Grundfunktionen zum externen EEProm des CP1+ Boards. Aufgrund der fixen
     Verdrahtung des EEProms auf SCL = PB6, SDA = PB7 koennen diese An-
     schluesse nicht geaendert werden. 
     
     PB6 und PB7 sind beim Betrieb eines ATmegas mit externem Quarz die 
     Quarzanschluesse.
     
     Weil PB6 und PB7 innerhalb des Arduino Frameworks nicht deklariert sind
     muss hier der Umweg ueber direkte Zuweisung an Register DDR, PIN und
     PORT erfolgen.
     
     27.01.2021    R. Seelig                      
   --------------------------------------------------------------------------- */

#ifndef in_cp1_eeprom
#define in_cp1_eeprom

#include "Arduino.h"
#include <avr/io.h>
#include <util/delay.h>

// I2C:   SCL = PB6, SDA = PB7

#define mask6          ( 1 << 6 )
#define mask7          ( 1 << 7 )

#define i2c_scl_hi()   ( DDRB &= ~mask6 )
#define i2c_scl_lo()   { DDRB |= mask6; PORTB &= ~mask6; }

#define i2c_sda_hi()   ( DDRB &= ~mask7 )
#define i2c_sda_lo()   { DDRB |= mask7; PORTB &= ~mask7; }
#define i2c_is_sda()   ( PINB & mask7 )

  /*
    Pagesizes EEPROM

     EEPROM  |  bytes per page
    _________|_________________
             |
      24LC16 |     16
      24LC32 |      8
      24LC64 |     32
     24LC128 |     64
     24LC256 |     64
     24LC512 |    128
    24LC1025 |    128


  */

  #define  eep_pagesize    8
  #define  eep_addr        0xa0


  #define short_puls     1            // Einheiten fuer einen langen Taktimpuls
  #define long_puls      1            // Einheiten fuer einen kurzen Taktimpuls
  #define del_wait       1            // Wartezeit fuer garantierten 0 Pegel SCL-Leitung

  #define short_del()     i2c_delay(short_puls)
  #define long_del()      i2c_delay(long_puls)
  #define wait_del()      i2c_delay(del_wait)

class eepr
{
  public:
  
    #define i2c_read_ack()    i2c_read(1)
    #define i2c_read_nack()   i2c_read(0)
      
    eepr(void);
    void i2c_delay(uint16_t anz);
    void i2c_sendstart(void);
    uint8_t i2c_start(uint8_t addr);
    void i2c_stop();
    void i2c_startaddr(uint8_t addr, uint8_t rwflag);
    uint8_t check_eeprom(void);
    void i2c_write_nack(uint8_t data);
    uint8_t i2c_write(uint8_t data);
    uint8_t i2c_write16(uint16_t data);
    uint8_t i2c_read(uint8_t ack);
    
    void write(uint16_t adr, uint8_t value);
    void erase(void);
    void writebuf(uint16_t adr, uint8_t *buf, uint16_t len);
    uint8_t read(uint16_t adr);
    void readbuf(uint16_t adr, uint8_t *buf, uint16_t len);    

  protected:

  private:

};


#endif

