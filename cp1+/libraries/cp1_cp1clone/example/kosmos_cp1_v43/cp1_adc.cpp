/* ------------------------------------------------------------------
                             cp1_adc.c

     Softwaremodul zum Ansprechen des internen AD-Wandlers

     MCU   : ATmega328

     25.11.2019        R. Seelig
   ------------------------------------------------------------------ */

#include "cp1_adc.h"

/* --------------------------------------------------------
                        adc_10bit

     liest die "Ergebnisregister" des ADC aus. Hier
     wichtig: Niederwertiges Register ist zuerst zu
     lesen.

     Rueckgabe:
        10-Bit ADC-Wert des durch ADC's
   -------------------------------------------------------- */
unsigned int getadc_10bit (void)
{
  uint16_t adcwert,adch;

  adcwert= ADCL;                        // niederwertiges Register lesen
  adch= ADCH;                           // hoeherwertiges Register lesen
  adch = adch<<8;
  adcwert|= adch;
  return(adcwert);
}

/* --------------------------------------------------------
                        adc_init

     initialisiert den ADC und den Eingangsmultiplexer

     Uebergabe

     vref    : zu verwendende Referenzspannung
     channel : zu verwendenden Analogeingang (ADC0 .. ADC7)
               Hinweis: ADC6 und ADC7 nur in 32 pol.
                        Gehaeusen vorhanden
   -------------------------------------------------------- */
void adc_init(uint8_t vref, uint8_t channel)
{
/* <vref> bezeichnet die zu verwendende Spannungsreferenz
     0 : externe Referenz an Aref
     1 : +AVcc mit ext. Kondensator
     2 : reserviert
     3 : interne Referenz: 1,1V ATmega168 / 2,56V ATmega8

   <channel> bezeichnet den Kanal auf dem gemessen werden soll

   Spannungsreferenz ATMEGA328:

   ADMUX
   -----------------------------------------------------------------------
     Bit |  7    |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
   -----------------------------------------------------------------------
    Name | REFS1 | REFS0 | ADLAR |   -   | MUX3  | MUX2  | MUX1  | MUX0  |


    REFS1  REFS0      Referenz
    --------------------------
      0      0        ext. Referenz an  Aref
      0      1        +AVcc mit ext. Kondensator
      1      0        reserviert
      1      1        Interne Spannungsreferenz: 1,1V (ATmega168) / 2,56V (ATmega8)

    ADLAR
    -----
    0 = rechtsbuendig, 1 = linksbuendig

     MUXx   
    3 2 1 0
    -------------------------
    0 0 0 0   --  
       ..       |->  Eingangskanaele ADC0 - ADC7
    0 1 1 1   --
    1 0 0 0   --
       ..       |->  reserviert
    1 1 0 1   --
    1 1 1 0          legt 1.1V an den ADC
    1 1 1 1          legt GND an den ADC


   ADCSRA
   -----------------------------------------------------------------------
     Bit |  7    |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
   -----------------------------------------------------------------------
    Name | ADEN  | ADSC  | ADATE | ADIF  | ADIE  | ADPS2 | ADPS1 | ADPS0 |


    Scalermodus | ADPS2 | ADPS1 | ADPS0 | Teilerfaktor
    ---------------------------------------------------
         0      |   0  |   0   |   0   |       2
         1      |   0  |   0   |   1   |       2
         2      |   0  |   1   |   0   |       4
         3      |   0  |   1   |   1   |       8
         4      |   1  |   0   |   0   |      16
         5      |   1  |   0   |   1   |      32
         6      |   1  |   1   |   0   |      64
         7      |   1  |   1   |   1   |     128

*/

   ADMUX = (vref << 6) | channel;

   ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | 7;       // ADC enable, single, auto trigger
                                                                // Taktprescalermode 7 ( / 128)

   _delay_ms(1);
   ADCSRA &= ~(1 << ADSC);                                      // ADC single auf 0
}
