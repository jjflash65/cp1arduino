/* -----------------------------------------------------
                        sw_i2c.h

    Header fuer Anbinden eines EEPROMs mit 
    Software-I2C Bus (Bit-banging)

    MCU   : ATmega328p
    Takt  : 8 MHz

    09.01.2021   R. Seelig
  ------------------------------------------------------ */

#ifndef in_cp1_eeprom_i2c
  #define in_cp1_eeprom_i2c

  #include <util/delay.h>
  #include <avr/io.h>

  // Dataanschluss
  #define i2c_sdaport    B
  #define i2c_sdabitnr   7

  // Clockanschluss
  #define i2c_sclport    B
  #define i2c_sclbitnr   6
  
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

  // --------------------------------------------------------------------
  //                         Prototypen
  // --------------------------------------------------------------------
  
  void i2c_delay(uint16_t anz);
  void i2c_master_init(void);
  void i2c_sendstart(void);
  uint8_t i2c_start(uint8_t addr);
  void i2c_stop();
  void i2c_startaddr(uint8_t addr, uint8_t rwflag);
  void i2c_write_nack(uint8_t data);
  uint8_t i2c_write(uint8_t data);
  uint8_t i2c_write16(uint16_t data);
  uint8_t i2c_read(uint8_t ack);

  #define i2c_read_ack()    i2c_read(1)
  #define i2c_read_nack()   i2c_read(0)
  
  void eep_write(uint16_t adr, uint8_t value);
  void eep_erase(void);
  void eep_writebuf(uint16_t adr, uint8_t *buf, uint16_t len);
  uint8_t eep_read(uint16_t adr);
  void eep_readbuf(uint16_t adr, uint8_t *buf, uint16_t len);


  // ----------------------------------------------------------------
  // Praeprozessormacros um 2 Stringtexte zur weiteren Verwendung
  // innerhalb des Praeprozessors  zu verknuepfen
  //
  // Bsp.:
  //        #define ionr      A
  //        #define ioport    conc2(PORT, ionr)
  //
  //        ioport wird nun als "PORTA" behandelt
  #define conc2(a,b)        CONC2EXP(a, b)
  #define CONC2EXP(a,b)     a ## b
  // ----------------------------------------------------------------

  // ----------------------------------------------------------------
  //   Makros zum Initialiseren der verwendeten Pins als Ausgaenge
  //   sowie zum Setzen / Loeschen dieser Pins (Bitbanging)
  //
  //   liest sich sehr "verwirrend", macht aber das Konfigurieren
  //   der beteiligten Pins (siehe Anfang dieser Datei) sehr ein-
  //   fach. Diese Zeilen hier sollten nicht veraendert werden
  // ----------------------------------------------------------------
  #define sdaport           conc2(PORT,i2c_sdaport)
  #define sdaddr            conc2(DDR,i2c_sdaport)
  #define sdapin            conc2(PIN,i2c_sdaport)

  #define sclport           conc2(PORT,i2c_sclport)
  #define sclddr            conc2(DDR,i2c_sclport)

  #define i2c_sda_hi()      sdaddr &= ~(1 << i2c_sdabitnr)
  #define i2c_sda_lo()      { sdaddr |= (1 << i2c_sdabitnr); sdaport &= ~(1 << i2c_sdabitnr); }
  #define i2c_is_sda()      ((sdapin & (1 << i2c_sdabitnr)) >> i2c_sdabitnr)

  #define i2c_scl_hi()      sclddr &= ~(1 << i2c_sclbitnr)
  #define i2c_scl_lo()      { sclddr |= (1 << i2c_sclbitnr); sclport &= ~(1 << i2c_sclbitnr); }


#endif

  
