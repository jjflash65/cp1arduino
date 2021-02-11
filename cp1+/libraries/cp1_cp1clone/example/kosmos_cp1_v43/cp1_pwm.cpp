/* ----------------------------------------------------------
                          cp1_pwm.cpp

     initialisiert Timer fuer die Verwendung als
     PWM-Generatoren.

     Aktivierung der einzelnen PWM in pwm.h.

         PWM Timer0 PWM Pin = PD5
         PWM Timer1 PWM Pin = PB1
         PWM Timer2 PWM Pin = PD3

     MCU     : ATmegaxx8
     F_CPU   :

     06.01.2021    R. Seelig
   ---------------------------------------------------------- */

#include "cp1_pwm.h"


// Reziprokenwert = 1e+12 damit Berechnungen mit Float
// entfallen koennen
#define rez_val  1000000000

/* -------------------------------------------------
                      pwmt0_init

     initialisiert eine PWM auf Pin PD5 (OC0B).

     Uebergabe:

       prescale : Taktvorteiler Timer (siehe
                  Kommentare) t_s= t_F_CPU / Teiler
                  Gueltige Werte 0..5

       tg       : Gesamtzeit Periode, t= tg * t_s
       tp       : Pausezeit, t= tp * t_s
   ------------------------------------------------- */
void pwmt0_init(uint8_t prescale, uint8_t tg, uint8_t tp)
{
  PD5_output_init();         // PD5 is "Ausgabepin" fuer Output Compare 0 B (OC0B)

  // WGM00:WGM01, WGM02 in TCCR0B  = 1:1:1 ---> FAST PWM
  // COM0B1:COM2B0                 = 1:0   ---> setzt OC2B zu Beginn der Periode, setzt zurueck
  //                                            bei erreichen des Comparewerts (in OCR0B)
  TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);

  // ----------------------------------------------
  // Bits TCCR0B: CS02:CS01:C020 = Bit2:Bit1:Bit0
  // ----------------------------------------------
  //  CS02   CS01  CS00
  //   0      0     0   ---> keine Taktquelle, Timer gestoppt
  //   0      0     1   ---> F_CPU / 1
  //   0      1     0   ---> F_CPU / 8
  //   0      1     1   ---> F_CPU / 64
  //   1      0     0   ---> F_CPU / 256
  //   1      0     1   ---> F_CPU / 1024

  TCCR0B = (1 << WGM02) | prescale;


  //   Bsp.:  F_CPU = 16 MHz, prescale = 7 (Takt / 1024)
  //          OCR2A = 80 (Ende der Periode bei Erreichen entspricht Gesamtdauer)
  //          OCR2B = 10 nach 10 Takten Wechsel von 1 auf 0

  //  Timertakt     = 16 MHz / 1024 = 15.625 KHz = 0,064 ms
  //  Periodendauer = 80 * (1 / 15.625 KHz) = 5.12 ms => 195.3125 Hz
  //  Pulsdauer     = 10 * 0.064 ms = 0.64 ms

  OCR0A = tg;
  OCR0B = tp;
}

/* -------------------------------------------------
                      pwmt0_ocraget

     ermittelt aus gegebenen Prescaler (csbits)
     und gewuenschter Frequenz den Comparewert fuer
     OCR2A

     Gewuenschte Zielfrequenz wird im Zeiger
     *destfreq uebergeben. In der Rueckgabe ent-
     haelt *destfreq die tatsaechliche Frequenz
     fuer eine Einstellung mit OCR0A

     Uebergabe:
       csbits    : Prescaler 1..5 (siehe pwmt1_init)
       *destfreq : gewuenschte Zielfrequenz

     Rueckgabe:
       Funktionswert : Abeichung zur gewuenschten
                       Frequenz in Hz
       *destfreq : erzielte Frequenz bei Einstellung
                   mit OCR20 = icr
       *ocr0     : Zeiger auf Comparewert

   ------------------------------------------------- */
uint16_t pwmt0_ocraget(uint8_t csbits, uint16_t *destfreq, uint16_t *ocr0)
{
  uint32_t tf, tclk, tk;
  uint16_t divisor;
  uint16_t f, fdif;;

  f= *destfreq;
  tk= *ocr0;

  switch (csbits)
  {
    case 1 : divisor= 1; break;
    case 2 : divisor= 8; break;
    case 3 : divisor= 64; break;
    case 4 : divisor= 256; break;
    case 5 : divisor= 1024; break;
    default : break;
  }
  tf= F_CPU / divisor;
  tclk= rez_val / tf;                                  // Periodendauer des Timer1
  tk= ((( rez_val / f) / tclk) - 1);                   // Anzahl Cyclen fuer gewuenschte Frequenz
  if ((tk> 0xff) || (tk== 0)) { return 0xffff; }       // wenn Wert nicht einstellbar ist
  f= (rez_val / (tclk * (tk + 1)));                    // Frequenz die mit tk (spaeter dann OCR0A)
                                                       // eingestellt wird
  fdif= abs(*destfreq - f);                            // Abweichung gewuenschte zu einstellbarer
                                                       // Frequenz
  *destfreq= f;
  *ocr0= (uint16_t)tk;
  return fdif;
}

/* -------------------------------------------------
                     pwmt0_setfreq

     setzt die nahest einstellbare Frequenz auf PB1
     ein.

     Uebergabe:
       freq : gewuenschte Frequenz
       duty : Tastgrad (Pusdauer : Perode) in %
   ------------------------------------------------- */
uint16_t pwmt0_setfreq(uint16_t freq, uint16_t duty)
{
  uint16_t destfreq, f;
  uint16_t ocr0;
  uint16_t fdif, fdif2;

  uint8_t  sel_csbits;
  uint16_t sel_ocr0;
  uint8_t  i;
  uint32_t d2;

  fdif= 0xfffe;

  // alle Prescaler nach der geringsten Abweichung scanen
  for (i= 1; i< 6; i++)
  {
    destfreq= freq;
    fdif2= pwmt0_ocraget(i, &destfreq, &ocr0);
    if (fdif2< fdif)
    {
      fdif= fdif2;
      f= destfreq;                // eingestellte Frequenz speichern
      sel_csbits= i;              // dto. Prescaler
      sel_ocr0= ocr0;             // dto. OCR0A
    }
  }

  d2= sel_ocr0;
  d2= (d2 * duty) / 100;
  pwmt0_init(sel_csbits, sel_ocr0, (uint16_t) d2);
  return f;
}



/* -------------------------------------------------
                      pwmt1_init

     initialisiert eine PWM auf Pin PB1 (OC1A).

     Uebergabe:

       prescale : Taktvorteiler Timer (siehe
                  Kommentare) t_s= t_F_CPU / Teiler
                  Gueltige Werte 0..5

       tg       : Gesamtzeit Periode, t= tg * t_s
       tp       : Pausezeit, t= tp * t_s
   ------------------------------------------------- */
void pwmt1_init(uint8_t prescale, uint16_t tg, uint16_t tp)
{
  PB1_output_init();         // PB1 is "Ausgabepin" fuer Output Compare 1 A (OC1A)

  // WGM10= 0 ; WGM11 WGM12, WGM13 = 1 ===> Modus 14 (Fast PWM)
  // Prescaler wird durch CS10, CS11, CS12 (1.0.0) gesetzt (keine Taktvorteilung)

  // TCCR1A  : WGM11:WGM10
  //         : COM1A1:COM1A0
  //
  //   COM1A1:COM1A0 : 1:0 ---> OC1A gesetzt bei Periodenbeginn, bei erreichen von
  //                            OCR1AH:OCR1AL zurueckgesetzt

  // TCCR1B  Bit4:Bit3 : WGM13:WGM12

  // WGM13:WGM12:GM11:WGM10  = 1:1:1:0 ---> FAST PWM mit Vergleichswert Gesamt-
  //                                        zeit in ICR1H:ICR1L

  TCCR1A = (1 << COM1A1) | (1 << WGM11);

  // ----------------------------------------------
  // Bits TCCR1B: CS12:CS11:CS10 = Bit2:Bit1:Bit0
  // ----------------------------------------------
  //  CS12   CS11  CS10
  //   0      0     0   ---> keine Taktquelle, Timer gestoppt
  //   0      0     1   ---> F_CPU / 1
  //   0      1     0   ---> F_CPU / 8
  //   0      1     1   ---> F_CPU / 64
  //   1      0     0   ---> F_CPU / 256
  //   1      0     1   ---> F_CPU / 1024
  //   1      1     0   ---> ext. Takt an T1, fallende Flanke
  //   1      1     1   ---> ext. Takt an T1, steigende Flanke

  TCCR1B = (1 << WGM13) | (1 << WGM12) | prescale;

  //   Bsp.:  F_CPU         = 16 MHz, prescale = 5 (Takt / 1024)
  //          ICR1H:ICR1L   = 10000 (Ende der Periode bei Erreichen entspricht Gesamtdauer)
  //          OCR1AH:OCR1AL = 2000 --> nach 2000 Takten Wechsel von 1 auf 0

  //  Timertakt     = 16 MHz / 1024 = 15.625 KHz = 0,064 ms
  //  Periodendauer = 200000 * (1 / 15.625 KHz) = 1.28 ss => 0.78125 Hz
  //  Pulsdauer     = 100000 * 0.064 ms = 0.64 s

  ICR1H = (tg >> 8);
  ICR1L = (uint8_t) tg & 0x00ff;
  OCR1AH = (tp >> 8);
  OCR1AL = (uint8_t) tp & 0x00ff;

}

/* -------------------------------------------------
                      pwmt1_icrget

     ermittelt aus gegebenen Prescaler (csbits)
     und gewuenschter Frequenz den Comparewert fuer
     ICR1H:ICR1L.

     Gewuenschte Zielfrequenz wird im Zeiger
     *destfreq uebergeben. In der Rueckgabe ent-
     haelt *destfreq die tatsaechliche Frequenz
     fuer eine Einstellung mit ICR1

     Uebergabe:
       csbits    : Prescaler 1..5 (siehe pwmt1_init)
       *destfreq : gewuenschte Zielfrequenz

     Rueckgabe:
       Funktionswert : Abeichung zur gewuenschten
                       Frequenz in Hz
       *destfreq : erzielte Frequenz bei Einstellung
                   mit ICR1H:ICR1L = icr
       *icr      : Zeiger auf Comparewert
   ------------------------------------------------- */
uint16_t pwmt1_icrget(uint8_t csbits, uint16_t *destfreq, uint16_t *icr)
{
  uint32_t tf, tclk, tk;
  uint16_t divisor;
  uint16_t f, fdif;;

  f= *destfreq;
  tk= *icr;

  switch (csbits)
  {
    case 1 : divisor= 1; break;
    case 2 : divisor= 8; break;
    case 3 : divisor= 64; break;
    case 4 : divisor= 256; break;
    case 5 : divisor= 1024; break;
    default : break;
  }
  tf= F_CPU / divisor;
  tclk= rez_val / tf;                                  // Periodendauer des Timer1
  tk= ((( rez_val / f) / tclk) - 1);                   // Anzahl Zyklen fuer gewuenschte Frequenz
  if ((tk> 0xffff) || (tk== 0)) { return 0xffff; }     // wenn Wert nicht einstellbar ist
  f= (rez_val / (tclk * (tk + 1)));                    // Frequenz die mit tk (spaeter dann ICR1)
                                                       // eingestellt wird
  fdif= abs(*destfreq - f);                            // Abweichung gewuenschte zu einstellbarer
                                                       // Frequenz
  *destfreq= f;
  *icr= (uint16_t)tk;
  return fdif;
}

/* -------------------------------------------------
                     pwmt1_setfreq

     setzt die nahest einstellbare Frequenz auf PB1
     ein.

     Uebergabe:
       freq : gewuenschte Frequenz
       duty : Tastgrad (Pusdauer : Perode) in %
   ------------------------------------------------- */
uint16_t pwmt1_setfreq(uint16_t freq, uint16_t duty)
{
  uint16_t destfreq, f;
  uint16_t icr;
  uint16_t fdif, fdif2;

  uint8_t  sel_csbits;
  uint16_t sel_icr;
  uint8_t  i;
  uint32_t d2;

  fdif= 0xfffe;

  // alle Prescaler nach der geringsten Abweichung scanen
  for (i= 1; i< 6; i++)
  {
    destfreq= freq;
    fdif2= pwmt1_icrget(i, &destfreq, &icr);
    if (fdif2< fdif)
    {
      fdif= fdif2;
      f= destfreq;                // eingestellte Frequenz speichern
      sel_csbits= i;              // dto. Prescaler
      sel_icr= icr;               // dto. ICR1
    }
  }

  d2= sel_icr;
  d2= (d2 * duty) / 100;
  pwmt1_init(sel_csbits, sel_icr, (uint16_t) d2);
  return f;
}


/* -------------------------------------------------
                      pwmt2_init

     initialisiert eine PWM auf Pin PD3 (OC2B).

     Uebergabe:

       prescale : Taktvorteiler Timer (siehe
                  Kommentare) t_s= t_F_CPU / Teiler
                  Gueltige Werte 0..5

       tg       : Gesamtzeit Periode, t= tg * t_s
       tp       : Pausezeit, t= tp * t_s
   ------------------------------------------------- */
void pwmt2_init(uint8_t prescale, uint8_t tg, uint8_t tp)
{
  PD3_output_init();         // PD3 is "Ausgabepin" fuer Output Compare 2 B (OC2B)

  // WGM20:WGM21, WGM22 in TCCR2B  = 1:1:1 ---> FAST PWM
  // COM2B1:COM2B0                 = 1:0   ---> setzt OC2B zu Beginn der Periode, setzt zurueck
  //                                            bei erreichen des Comparewerts (in OCR2B)
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);

  // ----------------------------------------------
  // Bits TCCR2B: CS22:CS21:CS20 = Bit2:Bit1:Bit0
  // ----------------------------------------------
  //  CS22   CS21  CS20
  //   0      0     0   ---> keine Taktquelle, Timer gestoppt
  //   0      0     1   ---> F_CPU / 1
  //   0      1     0   ---> F_CPU / 8
  //   0      1     1   ---> F_CPU / 32
  //   1      0     0   ---> F_CPU / 64
  //   1      0     1   ---> F_CPU / 128
  //   1      1     0   ---> F_CPU / 256
  //   1      1     1   ---> F_CPU / 1024

  TCCR2B = (1 << WGM22) | prescale ;


  //   Bsp.:  F_CPU = 16 MHz, prescale = 7 (Takt / 1024)
  //          OCR2A = 80 (Ende der Periode bei Erreichen entspricht Gesamtdauer)
  //          OCR2B = 10 nach 10 Takten Wechsel von 1 auf 0

  //  Timertakt     = 16 MHz / 1024 = 15.625 KHz = 0,064 ms
  //  Periodendauer = 80 * (1 / 15.625 KHz) = 5.12 ms => 195.3125 Hz
  //  Pulsdauer     = 10 * 0.064 ms = 0.64 ms

  OCR2A = tg;
  OCR2B = tp;
}

/* -------------------------------------------------
                      pwmt2_ocr2aget

     ermittelt aus gegebenen Prescaler (csbits)
     und gewuenschter Frequenz den Comparewert fuer
     OCR2A

     Gewuenschte Zielfrequenz wird im Zeiger
     *destfreq uebergeben. In der Rueckgabe ent-
     haelt *destfreq die tatsaechliche Frequenz
     fuer eine Einstellung mit OCR2A

     Uebergabe:
       csbits    : Prescaler 1..5 (siehe pwmt1_init)
       *destfreq : gewuenschte Zielfrequenz

     Rueckgabe:
       Funktionswert : Abeichung zur gewuenschten
                       Frequenz in Hz
       *destfreq : erzielte Frequenz bei Einstellung
                   mit OCR2A = icr
       *ocr2     : Zeiger auf Comparewert

   ------------------------------------------------- */
uint16_t pwmt2_ocraget(uint8_t csbits, uint16_t *destfreq, uint16_t *ocr2)
{
  uint32_t tf, tclk, tk;
  uint16_t divisor;
  uint16_t f, fdif;;

  f= *destfreq;
  tk= *ocr2;

  switch (csbits)
  {
    case 1 : divisor= 1; break;
    case 2 : divisor= 8; break;
    case 3 : divisor= 32; break;
    case 4 : divisor= 64; break;
    case 5 : divisor= 128; break;
    case 6 : divisor= 256; break;
    case 7 : divisor= 1024; break;
    default : break;
  }
  tf= F_CPU / divisor;
  tclk= rez_val / tf;                                  // Periodendauer des Timer1
  tk= ((( rez_val / f) / tclk) - 1);                   // Anzahl Cyclen fuer gewuenschte Frequenz
  if ((tk> 0xff) || (tk== 0)) { return 0xffff; }       // wenn Wert nicht einstellbar ist
  f= (rez_val / (tclk * (tk + 1)));                    // Frequenz die mit tk (spaeter dann OCR2A)
                                                       // eingestellt wird
  fdif= abs(*destfreq - f);                            // Abweichung gewuenschte zu einstellbarer
                                                       // Frequenz
  *destfreq= f;
  *ocr2= (uint16_t)tk;
  return fdif;
}

/* -------------------------------------------------
                     pwmt2_setfreq

     setzt die nahest einstellbare Frequenz auf PB1
     ein.

     Uebergabe:
       freq : gewuenschte Frequenz
       duty : Tastgrad (Pusdauer : Perode) in %
   ------------------------------------------------- */
uint16_t pwmt2_setfreq(uint16_t freq, uint16_t duty)
{
  uint16_t destfreq, f;
  uint16_t ocr2;
  uint16_t fdif, fdif2;

  uint8_t  sel_csbits;
  uint16_t sel_ocr2;
  uint8_t  i;
  uint32_t d2;

  fdif= 0xfffe;

  // alle Prescaler nach der geringsten Abweichung scanen
  for (i= 1; i< 8; i++)
  {
    destfreq= freq;
    fdif2= pwmt2_ocraget(i, &destfreq, &ocr2);
    if (fdif2< fdif)
    {
      fdif= fdif2;
      f= destfreq;                // eingestellte Frequenz speichern
      sel_csbits= i;              // dto. Prescaler
      sel_ocr2= ocr2;             // dto. OCRA
    }
  }

  d2= sel_ocr2;
  d2= (d2 * duty) / 100;
  pwmt2_init(sel_csbits, sel_ocr2, (uint16_t) d2);
  return f;
}
