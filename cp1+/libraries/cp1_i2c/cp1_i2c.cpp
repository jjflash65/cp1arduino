/* ---------------------------------------------------------------------------
                                cp1_i2c.cpp

     Rudimentaere I2C Funktionen mittels Bitbanging (Software) realisiert

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include "cp1_i2c.h"

/* -------------------------------------------------------
                           swi2c::swi2c

    setzt die Pins die fuer den I2C Bus verwendet werden
    auf High
   ------------------------------------------------------- */
swi2c::swi2c(uint8_t da, uint8_t cl)
{
  sda= da; scl= cl;
  i2c_sda_hi();
  i2c_scl_hi();
}

/* -------------------------------------------------------
                         swi2c::setpins

     vergibt neue Zuordnung zu SDA und SCL
   ------------------------------------------------------- */
void swi2c::setpins(uint8_t da, uint8_t cl)
{
  sda= da; scl= cl;
  i2c_sda_hi();
  i2c_scl_hi();
}


/* ---------------------------------------------------------
                        swi2c::delay

       an die "Reaktionszeiten" des ATmega mit 8MHz ange-
       passte Warteschleife
   --------------------------------------------------------- */
void swi2c::delay(uint16_t anz)
{
  volatile uint16_t count;

  for (count= 0; count< anz; count++)
  {
    _delay_us(1);
  }
}

/* -------------------------------------------------------
                    swi2c::sendstart

    erzeugt die Startbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void swi2c::sendstart()
{
  i2c_scl_hi();
  long_del();

  i2c_sda_lo();
  long_del();
}

/* -------------------------------------------------------
                     swi2c::start

    erzeugt die Startbedingung und sendet anschliessend
    die Deviceadresse
   ------------------------------------------------------- */
uint8_t swi2c::start(uint8_t addr)
{
  uint8_t ack;

  sendstart();
  ack= write(addr);
  return ack;
}

/* -------------------------------------------------------
                   swi2c::startaddr

   startet den I2C-Bus und sendet Bauteileadresse.
   rwflag bestimmt, ob das Device beschrieben oder
   gelesen werden soll
  -------------------------------------------------- */
void swi2c::startaddr(uint8_t addr, uint8_t rwflag)
{
  addr = (addr << 1) | rwflag;

  sendstart();
  write(addr);
}

/* -------------------------------------------------------
                    swi2c:stop

    erzeugt die Stopbedingung auf dem I2C Bus
   ------------------------------------------------------- */
void swi2c::stop(void)
{
   i2c_sda_lo();
   long_del();
   i2c_scl_hi();
   short_del();
   i2c_sda_hi();
   long_del();
}

/* -------------------------------------------------------
                  swi2c::write_nack

   schreibt einen Wert auf dem I2C Bus OHNE ein Ack-
   nowledge einzulesen
  ------------------------------------------------------- */
void swi2c::write_nack(uint8_t data)
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
                 swi2c::write

   schreibt einen Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t swi2c::write(uint8_t data)
{
   uint8_t ack;

   write_nack(data);

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
                  swi2c::write16

   schreibt einen 16-Bit Wert auf dem I2C Bus.

   Rueckgabe:
               > 0 wenn Slave ein Acknowledge gegeben hat
               == 0 wenn kein Acknowledge vom Slave
   ------------------------------------------------------- */
uint8_t swi2c::write16(uint16_t data)
{
  uint8_t ack;
  ack= write(data >> 8);
  if (!(ack)) return 0;
  ack= write(data & 0xff);

  return ack;
}

/* -------------------------------------------------------
                 swi2c::read

   liest ein Byte vom I2c Bus.

   Uebergabe:
               1 : nach dem Lesen wird dem Slave ein
                   Acknowledge gesendet
               0 : es wird kein Acknowledge gesendet

   Rueckgabe:
               gelesenes Byte
   ------------------------------------------------------- */
uint8_t swi2c::read(uint8_t ack)
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
                 swi2c::read_ack

   liest ein Byte vom I2c Bus und gibt nach dem Lesen
   ein Acknowledge zurueck
   ------------------------------------------------------- */
uint8_t swi2c::read_ack(void)
{
  return read(1);
}

/* -------------------------------------------------------
                 swi2c::read_nack

   liest ein Byte vom I2c Bus und gibt nach dem Lesen
   jedoch KEIN Acknowledge zurueck
   ------------------------------------------------------- */
uint8_t swi2c::read_nack(void)
{
  return read(0);
}
