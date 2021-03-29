/* ---------------------------------------------------------------------------
                                cp1_rda5807.cpp

     rudimentaere Funktionen zum UKW - Empfaengerchip RDA5807

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include "Arduino.h"
#include "cp1_rda5807.h"


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
  
/* -------------------------------------------------------
                       rda5807::rda5807
                         Konstruktor

    setzt die Pins die fuer den _I2C_ Bus verwendet werden
   ------------------------------------------------------- */
rda5807::rda5807(uint8_t da, uint8_t cl)
{
  sda= da; scl= cl;
  i2c_sda_hi();
  i2c_scl_hi();
}

/* ---------------------------------------------------------
                        rda5807::i2c_delay

       an die "Reaktionszeiten" des ATmega mit 8MHz ange-
       passte Warteschleife
   --------------------------------------------------------- */
void rda5807::i2c_delay(uint16_t anz)
{
  volatile uint16_t count;

  for (count= 0; count< anz; count++)
  {
    _delay_us(1);
  }
}

/* -------------------------------------------------------
                    rda5807::i2c_sendstart

    erzeugt die Startbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void rda5807::i2c_sendstart()
{
  i2c_scl_hi();
  long_del();

  i2c_sda_lo();
  long_del();
}

/* -------------------------------------------------------
                     rda5807::i2c_start

    erzeugt die Startbedingung und sendet anschliessend
    die Deviceadresse
   ------------------------------------------------------- */
uint8_t rda5807::i2c_start(uint8_t addr)
{
  uint8_t ack;

  i2c_sendstart();
  ack= i2c_write(addr);
  return ack;
}

/* -------------------------------------------------------
                    swi2c:stop

    erzeugt die Stopbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void rda5807::i2c_stop(void)
{
   i2c_sda_lo();
   long_del();
   i2c_scl_hi();
   short_del();
   i2c_sda_hi();
   long_del();
}

/* -------------------------------------------------------
                  rda5807::i2c_write_nack

   schreibt einen Wert auf dem I2C Bus OHNE ein Ack-
   nowledge einzulesen
  ------------------------------------------------------- */
void rda5807::i2c_write_nack(uint8_t data)
{
  uint8_t i;

  for(i=0;i<8;i++)
  {
    i2c_scl_lo();
    short_del();

    if(data & 0x80)
    {
      i2c_sda_hi();
    }
    else
    {
      i2c_sda_lo();
    }

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
                 rda5807::i2c_write

   schreibt einen Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t rda5807::i2c_write(uint8_t data)
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
                  rda5807::i2c_write16

   schreibt einen 16-Bit Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t rda5807::i2c_write16(uint16_t data)
{
  uint8_t ack;
  ack= i2c_write(data >> 8);
  if (!(ack)) return 0;
  ack= i2c_write(data & 0xff);

  return ack;
}

/* -------------------------------------------------------
                 rda5807::i2c_read

   liest ein Byte vom I2c Bus.

   Uebergabe:
               1 : nach dem Lesen wird dem Slave ein
                   Acknowledge gesendet
               0 : es wird kein Acknowledge gesendet

   Rueckgabe:
               gelesenes Byte
   ------------------------------------------------------- */
uint8_t rda5807::i2c_read(uint8_t ack)
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

/* -------------------------------------------------------
                 swi2c_ack::read

   liest ein Byte vom I2c Bus und gibt nach dem Lesen
   ein Acknowledge zurueck
   ------------------------------------------------------- */
uint8_t rda5807::i2c_read_ack(void)
{
  return i2c_read(1);
}

/* -------------------------------------------------------
                 swi2c_nack::read

   liest ein Byte vom I2c Bus und gibt nach dem Lesen
   jedoch KEIN Acknowledge zurueck
   ------------------------------------------------------- */
uint8_t rda5807::i2c_read_nack(void)
{
  return i2c_read(1);
}

// -------------------------------------------------------------------------------------
//                               Ende I2C-Methoden
// -------------------------------------------------------------------------------------



/* --------------------------------------------------
      rda5807::writereg

   einzelnes Register des RDA5807 schreiben
   -------------------------------------------------- */
void rda5807::writereg(uint8_t ind)
{
  i2c_start(rda5807_adrr);
  i2c_write(ind);
  i2c_write16(reg[ind]);
  i2c_stop();
}

/* --------------------------------------------------
      rda5807::write

   alle Register es RDA5807 schreiben
   -------------------------------------------------- */
void rda5807::write(void)
{
  i2c_start(rda5807_adrs);
  for (cx= 2; cx< 7; cx++)
  {
    i2c_write16(reg[cx]);
  }
  i2c_stop();
}

/* --------------------------------------------------
      rda5807::reset
   -------------------------------------------------- */
void rda5807::reset(void)
{
  for (cx= 0; cx< 7; cx++)
  {
    reg[cx]= regdef[cx];
  }
  reg[2]= reg[2] | 0x0002;    // Enable SoftReset
  write();
  reg[2]= reg[2] & 0xFFFB;    // Disable SoftReset
}

/* --------------------------------------------------
      rda5807::poweron
   -------------------------------------------------- */
void rda5807::poweron(void)
{
  reg[3]= reg[3] | 0x010;   // Enable Tuning
  reg[2]= reg[2] | 0x001;   // Enable PowerOn

  write();

  reg[3]= reg[3] & 0xFFEF;  // Disable Tuning
}

/* --------------------------------------------------
      rda5807::setfreq

      setzt angegebene Frequenz * 0.1 MHz

      Bsp.:
         rda5807::setfreq(1018);    // setzt 101.8 MHz
                                    // die neue Welle
   -------------------------------------------------- */
int rda5807::setfreq(uint16_t channel)
{

  aktfreq= channel;

  channel -= fbandmin;
  channel&= 0x03FF;
  reg[3]= (channel << 6) + 0x10;  // Channel + TUNE-Bit + Band=00(87-108) + Space=00(100kHz)

  i2c_start(rda5807_adrs);
  i2c_write16(0xD009);
  i2c_write16(reg[3]);
  i2c_stop();

  delay(100);
  return 0;
}

/* --------------------------------------------------
      rda5807::setvol
   -------------------------------------------------- */
void rda5807::setvol(uint8_t vol)
{
  aktvol= vol;
  reg[5]=(reg[5] & 0xFFF0) | vol;
  writereg(5);
}

/* --------------------------------------------------
      rda5807::setmono
   -------------------------------------------------- */
void rda5807::setmono(void)
{
  reg[2]=(reg[2] | 0x2000);
  writereg(2);
}

/* --------------------------------------------------
      rda5807::setstero
   -------------------------------------------------- */
void rda5807::setstereo(void)
{
  reg[2]=(reg[2] & 0xdfff);
  writereg(2);
}

/* --------------------------------------------------
                   rda5807::getsig

     liefert Empfangsstaerke des eingestellten
     Senders zurueck
   -------------------------------------------------- */
uint8_t rda5807::getsig(void)
{
  uint8_t b;

  delay(100);
  i2c_start(rda5807_adrs | 1);
  for (cx= 0; cx < 3; cx++)
  {
    b= i2c_read_ack();
    delay(5);
    if (cx == 2)
    {
      i2c_read_nack();
      i2c_stop();
      return b;
    }
  }
  b= i2c_read_nack();

  i2c_stop();
  return b;
}

/* --------------------------------------------------
                   rda5807::scandown

     automatischer Sendersuchlauf, Frequenz
     dekrementieren
   -------------------------------------------------- */
void rda5807::scandown(void)
{
  tmpvol= aktvol;
  setvol(0);

  if (aktfreq== fbandmin) { aktfreq= fbandmax; }
  do
  {
    aktfreq--;
    setfreq(aktfreq);
    cx= getsig();
  }while ((cx < sigschwelle) && (aktfreq > fbandmin));

  aktvol= tmpvol;
  setvol(aktvol);
}

/* --------------------------------------------------
                   rda5807::scanup

     automatischer Sendersuchlauf, Frequenz
     inkrementieren
   -------------------------------------------------- */
void rda5807::scanup(void)
{
  uint8_t siglev;

  tmpvol= aktvol;
  setvol(0);

  if (aktfreq== fbandmax) { aktfreq= fbandmin; }
  do
  {
    aktfreq++;
    setfreq(aktfreq);
    siglev= getsig();
  }while ((siglev < sigschwelle) && (aktfreq < fbandmax));

  aktvol= tmpvol;
  setvol(aktvol);

}
