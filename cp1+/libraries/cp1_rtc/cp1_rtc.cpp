/* -------------------------------------------------------
                        rtc_demo.cpp

     Liest DS1307 / DS3231 (Realtime-Clock-Chip) aus und 
     zeigt diese auf der seriellen  Schnittstelle an

     26.03.2021  R. Seelig
   ------------------------------------------------------ */
   
#include "cp1_rtc.h"


struct my_datum date;  

/* --------------------------------------------------
                       Konstruktor
   -------------------------------------------------- */  
realtimeclock::realtimeclock()
{  
  // 1. Januar 2000, 01.00:00
  date.std= 1;
  date.min= 0;
  date.sek= 0;
  date.tag= 1;
  date.monat= 1;
  date.jahr= 0; 
}


/* --------------------------------------------------
     realtimeclock::read

     liest einen einzelnen Wert aus dem RTC-Chip

     Uebergabe:
         addr : Registeradresse des DS1307 der
                gelesen werden soll
   -------------------------------------------------- */
uint8_t realtimeclock::read(uint8_t addr)
{
  uint8_t value;

  i2c.sendstart();
  i2c.write(rtc_addr);
  i2c.write(addr);
  i2c.stop();
  i2c.sendstart();
  i2c.write(rtc_addr | 1);
  value= i2c.read_nack();
  i2c.stop();

  return value;
}

/* --------------------------------------------------
     realtimeclock::write

     schreibt einen einzelnen Wert aus dem RTC-Chip

     Uebergabe:
         addr : Registeradresse des DS1307 der
                geschrieben werden soll
   -------------------------------------------------- */
void realtimeclock::write(uint8_t addr, uint8_t value)
{
  i2c.sendstart();
  i2c.write(rtc_addr);
  i2c.write(addr);
  i2c.write(value);
  i2c.stop();
}

/* --------------------------------------------------
      realtimeclock::bcd2dez

      wandelt eine BCD Zahl (NICHT hex)  in einen
      dezimalen Wert um

      Bsp: value = 0x34
      Rueckgabe    34
   -------------------------------------------------- */
uint8_t realtimeclock::bcd2dez(uint8_t value)
{
  uint8_t hiz,c;

  hiz= value / 16;
  c= (hiz*10)+(value & 0x0f);
  return c;
}

/* --------------------------------------------------
      realtimeclock::dez2bcd

      wandelt eine dezimale Zahl in eine BCD
      Bsp: value = 45
      Rueckgabe    0x45
   -------------------------------------------------- */
uint8_t realtimeclock::dez2bcd(uint8_t value)
{
  uint8_t hiz,loz,c;

  hiz= value / 10;
  loz= (value -(hiz*10));
  c= (hiz << 4) | loz;
  return c;
}


/* --------------------------------------------------
      realtimeclock::getwtag

      Berechnet zu einem bestimmten Datum den
      Wochentag (nach Carl Friedrich Gauss). Die
      Funktion wertet die globale Struktur date
      aus. Ein Wochentag beginnt mit 0 (0 entspricht
      Sonntag)

      Rueckgabe:
           Tag der Woche

      Bsp.:      11.04.2017   ( das ist ein Dienstag )
      Rueckgabe: 2
   -------------------------------------------------- */
uint8_t realtimeclock::getwtag(void)
{
  int tag, monat, jahr;
  int w_tag;

  tag=  bcd2dez(date.tag);
  monat= bcd2dez(date.monat);
  jahr= bcd2dez(date.jahr)+2000;

  if (monat < 3)
  {
     monat = monat + 12;
     jahr--;
  }
  w_tag = (tag+2*monat + (3*monat+3)/5 + jahr + jahr/4 - jahr/100 + jahr/400 + 1) % 7 ;
  return w_tag;
}

/* --------------------------------------------------
      realtimeclock::readdate

      liest den DS1307 / DS3231 Baustein in die
      globale Struktur date ein.
   -------------------------------------------------- */
void realtimeclock::readdate(void)
{
  date.sek= bcd2dez(read(0) & 0x7f);
  date.min= bcd2dez(read(1) & 0x7f);
  date.std= bcd2dez(read(2) & 0x3f);
  date.tag= bcd2dez(read(4) & 0x3f);
  date.monat= bcd2dez(read(5) & 0x1f);
  date.jahr= bcd2dez(read(6));
  date.dow= bcd2dez(getwtag());
}

/* --------------------------------------------------
     realtimeclock::writedate

     schreibt die in der Struktur enthaltenen Daten
     in den RTC-Chip
   -------------------------------------------------- */
void realtimeclock::writedate(void)
{
           
  write(2, dez2bcd(date.std));
  write(1, dez2bcd(date.min));
  write(0, dez2bcd(date.sek));
  write(4, dez2bcd(date.tag));
  write(5, dez2bcd(date.monat));
  write(6, dez2bcd(date.jahr));     
}
