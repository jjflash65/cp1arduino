/* ---------------------------------------------------------------------------
                                cp1_eeprom.cpp
                 
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
   
#include "cp1_eeprom.h"

/* #################################################################
     Funktionen fuer I2C - Bus (Softwareimplementierung)
   ################################################################# */

/* -------------------------------------------------------
                           eepr::eepr

    setzt die Pins die fuer den I2C Bus verwendet werden
    auf High
   ------------------------------------------------------- */
eepr::eepr(void)
{
  i2c_sda_hi();
  i2c_scl_hi();
}

/* ---------------------------------------------------------
                        eepr::i2c_delay

       an die "Reaktionszeiten" des ATmega mit 8MHz ange-
       passte Warteschleife
   --------------------------------------------------------- */
void eepr::i2c_delay(uint16_t anz)
{
  volatile uint16_t count;

  for (count= 0; count< anz; count++)
  {
    _delay_us(1);
  }
}

/* -------------------------------------------------------
                    eepr::i2c_sendstart

    erzeugt die Startbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void eepr::i2c_sendstart()
{
  i2c_scl_hi();
  long_del();

  i2c_sda_lo();
  long_del();
}

/* -------------------------------------------------------
                     eepr::i2c_start

    erzeugt die Startbedingung und sendet anschliessend
    die Deviceadresse
   ------------------------------------------------------- */
uint8_t eepr::i2c_start(uint8_t addr)
{
  uint8_t ack;

  i2c_sendstart();
  ack= i2c_write(addr);
  return ack;
}

/* -------------------------------------------------------
                   eepr::i2c_startaddr

   startet den I2C-Bus und sendet Bauteileadresse.
   rwflag bestimmt, ob das Device beschrieben oder
   gelesen werden soll
  -------------------------------------------------- */
void eepr::i2c_startaddr(uint8_t addr, uint8_t rwflag)
{
  addr = (addr << 1) | rwflag;

  i2c_sendstart();
  i2c_write(addr);
}

/* -------------------------------------------------------
                    eepr:i2c_stop

    erzeugt die Stopbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void eepr::i2c_stop(void)
{
   i2c_sda_lo();
   long_del();
   i2c_scl_hi();
   short_del();
   i2c_sda_hi();
   long_del();
}

/* -------------------------------------------------------
                    eepr::check_eeprom

    testet, ob das I2C EEProm angeschlossen ist
    
    Rueckgabe:
    
        0 : EEProm antwortet nicht
        1 : EEProm vorhanden
   ------------------------------------------------------- */
uint8_t eepr::check_eeprom(void)
{
  uint8_t ack;
  ack= i2c_start(eep_addr);
  i2c_stop();
  return ack;
}

/* -------------------------------------------------------
                  eepr::i2c_write_nack

   schreibt einen Wert auf dem I2C Bus OHNE ein Ack-
   nowledge einzulesen
  ------------------------------------------------------- */
void eepr::i2c_write_nack(uint8_t data)
{
  uint8_t i;

  for(i=0;i<8;i++)
  {
    i2c_scl_lo();
    short_del();

    if(data & 0x80) i2c_sda_hi();
               else i2c_sda_lo();

    short_del();

    i2c_scl_hi();
    short_del();
    wait_del();

    data= data<<1;
  }
  i2c_scl_lo();
  short_del();

}


/* -------------------------------------------------------
                 eepr::i2c_write

   schreibt einen Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t eepr::i2c_write(uint8_t data)
{
   uint8_t ack;

   i2c_write_nack(data);

  //  9. Taktimpuls (Ack)

  i2c_sda_hi();
  long_del();

  i2c_scl_hi();
  long_del();

  i2c_sda_hi();
  long_del();

  if (i2c_is_sda()) ack= 0; else ack= 1;

  i2c_scl_lo();
  long_del();

  return ack;
}

/* -------------------------------------------------------
                  eepr::i2c_write16

   schreibt einen 16-Bit Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t eepr::i2c_write16(uint16_t data)
{
  uint8_t ack;
  ack= i2c_write(data >> 8);
  if (!(ack)) return 0;
  ack= i2c_write(data & 0xff);

  return ack;
}

/* -------------------------------------------------------
                 eepr::i2c_read

   liest ein Byte vom I2c Bus.

   Uebergabe:
               1 : nach dem Lesen wird dem Slave ein
                   Acknowledge gesendet
               0 : es wird kein Acknowledge gesendet

   Rueckgabe:
               gelesenes Byte
   ------------------------------------------------------- */
uint8_t eepr::i2c_read(uint8_t ack)
{
  uint8_t data= 0x00;
  uint8_t i;

  i2c_sda_hi();

  for(i=0;i<8;i++)
  {
    i2c_scl_lo();
    short_del();
    i2c_scl_hi();

    short_del();
    wait_del();

    if(i2c_is_sda()) data|= (0x80>>i);
  }

  i2c_scl_lo();

  i2c_sda_hi();

  long_del();

  if (ack)
  {
    i2c_sda_lo();
    long_del();
  }

  i2c_scl_hi();
  long_del();

  i2c_scl_lo();
  long_del();

  i2c_sda_hi();

  return data;
}


/* #################################################################
     Funktionen 24LCxx EEProm
   ################################################################# */

/* --------------------------------------------------
                   eepr::write

     schreibt einen 8-Bit Wert value an die
     Adresse adr
   -------------------------------------------------- */
void eepr::write(uint16_t adr, uint8_t value)
{
  i2c_start(eep_addr);
  i2c_write16(adr);
  i2c_write(value);
  i2c_stop();
  delay(4);
}

/* --------------------------------------------------
                    eepr::erase

     loescht den gesamten Inhalt des EEPROMS
   -------------------------------------------------- */
void eepr::erase(void)
{
  uint16_t cnt;
  uint16_t adr, len;

  adr= 0; len= 0x7fff;

  i2c_start(eep_addr);
  i2c_write16(adr);

  cnt= 0;
  do
  {
    i2c_write(0xff);
    cnt++;
    adr++;

    if ((adr % eep_pagesize == 0))                 // Pagegrenze des EEProms
    {
      i2c_stop();
      delay(4);                                    // Wartezeit bis Page geschrieben ist
      i2c_start(eep_addr);                         // neue Page oeffnen
      i2c_write16(adr);
    }
  } while (cnt< len);
  i2c_stop();
  delay(4);
}

/* --------------------------------------------------
                  eepr::writebuf

     schreibt mehrere Datenbytes in das EEProm

     Uebergabe:
         adr    : Adresse, ab der die Bytes im
                  EEProm gespeichert werden
         *buf   : Zeiger auf die Datenbytes, die
                  gespeichert werden sollen
         len    : Anzahl zu speichernder Bytes
   -------------------------------------------------- */
void eepr::writebuf(uint16_t adr, uint8_t *buf, uint16_t len)
{
  uint16_t cnt;

  i2c_start(eep_addr);
  i2c_write16(adr);

  cnt= 0;
  do
  {
    i2c_write(*buf);
    buf++;
    cnt++;
    adr++;

    if ((adr % eep_pagesize == 0))                 // Pagegrenze des EEProms
    {
      i2c_stop();
      delay(4);                                    // Wartezeit bis Page geschrieben ist
      i2c_start(eep_addr);                         // neue Page oeffnen
      i2c_write16(adr);
    }
  } while (cnt< len);
  i2c_stop();
  delay(4);
}

/* --------------------------------------------------
                     eepr::read

     liest ein einzelnes Byte aus dem EEProm an
     der Adresse adr aus
   -------------------------------------------------- */
uint8_t eepr::read(uint16_t adr)
{
  uint8_t value;

  delay(1);
  i2c_start(eep_addr);                             // sicherheitshalber
  i2c_stop();                                      // I2C Bus garantiert frei

  i2c_start(eep_addr);
  i2c_write16(adr);
  delay(5);

  i2c_start(eep_addr+1);

  value= i2c_read_nack();
  i2c_stop();
  return value;
}

/* --------------------------------------------------
                   eep::readbuf

     liest mehrere Bytes aus dem EEProm in einen
     Pufferspeicher ein.

     Uebergabe:
         adr     : Adresse, ab der im EEProm
                   gelesen wird.
         *buf    : Zeiger auf einen Speicherbereich
                   in den die Daten aus dem EEPROM
                   kopiert werden.
         len     : Anzahl der zu lesenden Bytes
   -------------------------------------------------- */
void eepr::readbuf(uint16_t adr, uint8_t *buf, uint16_t len)
{
  uint16_t cnt;

  i2c_start(eep_addr);                             // sicherheitshalber
  i2c_stop();                                      // I2C Bus garantiert frei

  i2c_start(eep_addr);
  i2c_write16(adr);
  i2c_start(eep_addr+1);

  cnt= 0;
  do
  {
    if (len== 1)
    {
      *buf= i2c_read_nack();
      i2c_stop();
      return;
    }
    *buf= i2c_read_ack();
    buf++;
    cnt++;
    adr++;
  } while (cnt < len-1);
  i2c_read_nack();                                  // noch einmal lesen um kein ack zu schicken
  i2c_stop();                                       // und somit Adresszaehler EEPROM freigeben
  _delay_ms(1);
}
