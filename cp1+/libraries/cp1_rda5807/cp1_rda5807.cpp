/* ---------------------------------------------------------------------------
                                cp1_rda5807.cpp

     rudimentaere Funktionen zum UKW - Empfaengerchip RDA5807

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include "Arduino.h"
#include "cp1_rda5807.h"

 
/* -------------------------------------------------------
                       rda5807::rda5807
                         Konstruktor

    setzt die Pins die fuer den I2C-Bus verwendet werden
   ------------------------------------------------------- */
rda5807::rda5807()
{
}

/* --------------------------------------------------
      rda5807::writereg

   einzelnes Register des RDA5807 schreiben
   -------------------------------------------------- */
void rda5807::writereg(uint8_t ind)
{
  i2c.start(rda5807_adrr);
  i2c.write(ind);
  i2c.write16(reg[ind]);
  i2c.stop();
}

/* --------------------------------------------------
      rda5807::write

   alle Register es RDA5807 schreiben
   -------------------------------------------------- */
void rda5807::write(void)
{
  i2c.start(rda5807_adrs);
  for (cx= 2; cx< 7; cx++)
  {
    i2c.write16(reg[cx]);
  }
  i2c.stop();
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

  i2c.start(rda5807_adrs);
  i2c.write16(0xD009);
  i2c.write16(reg[3]);
  i2c.stop();

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
  i2c.start(rda5807_adrs | 1);
  for (cx= 0; cx < 3; cx++)
  {
    b= i2c.read_ack();
    delay(5);
    if (cx == 2)
    {
      i2c.read_nack();
      i2c.stop();
      return b;
    }
  }
  b= i2c.read_nack();

  i2c.stop();
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
