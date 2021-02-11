/* ----------------------------------------------------------------
                            cp1_vports.cpp

     Zusammenfassen beliebiger Portpins zu 2 Pseudoports, die
     unter P1_x und P2_x angesprochen werden koennen. Geschwindig-
     keit der Schaltgeschwindigkeit ist dadurch herabgesetzt, ist
     aber fuer Programme bei denen ein kompletter 8-Bit-Port
     verwendet werden soll oder muss und bei denen die Geschwindig-
     keit eine untergeordnete Rolle spielt, hilfreich und/oder
     sinnvoll.

     MCU: 28 pol. AVR ATmega

     Zuordnung realer Pins zu den virtuellen Ports
                                                    M     M
                                                    O     I     S
                                                    S     S     C
                                                    I     O     K

     virtual P1  :  P1_0  P1_1  P1_2  P1_3  P1_4  P1_5  P1_6  P1_7
     AVR-Portpins:   PD6   PD7   PB0   PB1   PB2   PB3   PB4   PB5
     Adruino     :    6     7     8     9    10    11    12    13

     virtual P2  :  P2_0  P2_1  P2_2  P2_3  P2_4  P2_5  P2_6  P2_7
     AVR-Portpins:   PC0   PC1   PC2   PC3   PD4   PD3   PD2   PD0
     Arduino     :   A0    A1    A2    A3     4     3     2     0

     25.12.2020 R. Seelig
   ---------------------------------------------------------------- */

#include "cp1_vports.h"

/* ----------------------------------------------------------
               Definitionen fuer virtual Port 1
   ---------------------------------------------------------- */


/* ----------------------------------------------------------
                          p1_config

     konfiguriert den "virtuellen" Port p1 im Stile, wie
     das auch in einem AVR vorgenommen wird. Sinn des
     "virtuellen" Ports ist, dass "zusammengewuerfelte" Pins
     verschiedener Ports zu einem 8 Bit = 1 Byte Ausgabe-
     Eingabeports zusammen gefasst werden. Gn eschwindigkeits-
     wunder duerfen keine erwartet werden.

     dirbyte:

         einzelne 1 an der Bitposition entspricht einem
         Ausgang, eine 0 einem Eingang

     Bsp.: p1_config(0x93);

         P1_7  P1_6  P1_5  P1_4  P1_3  P1_2  P1_1  P1_0
          out   in    in    in    in    in    out   out
          1     0     0     0     0     0     1     1  = 0x93
   ---------------------------------------------------------- */
void p1_config(uint8_t dirbyte)
{
  if (dirbyte & 0x01) p1_0_out_init(); else p1_0_in_init();
  if (dirbyte & 0x02) p1_1_out_init(); else p1_1_in_init();
  if (dirbyte & 0x04) p1_2_out_init(); else p1_2_in_init();
  if (dirbyte & 0x08) p1_3_out_init(); else p1_3_in_init();
  if (dirbyte & 0x10) p1_4_out_init(); else p1_4_in_init();
  if (dirbyte & 0x20) p1_5_out_init(); else p1_5_in_init();
  if (dirbyte & 0x40) p1_6_out_init(); else p1_6_in_init();
  if (dirbyte & 0x80) p1_7_out_init(); else p1_7_in_init();
}

/* ----------------------------------------------------------
                          p1_bytewrite

     schreibt ein komplettes Byte auf den virtuellen Port
     P1
   ---------------------------------------------------------- */
void p1_bytewrite(uint8_t value)
{
  p1_config(0xff);                               // alle Pins Ausgaenge
  if (value & 0x01) p1_0_set(); else p1_0_clr();
  if (value & 0x02) p1_1_set(); else p1_1_clr();
  if (value & 0x04) p1_2_set(); else p1_2_clr();
  if (value & 0x08) p1_3_set(); else p1_3_clr();
  if (value & 0x10) p1_4_set(); else p1_4_clr();
  if (value & 0x20) p1_5_set(); else p1_5_clr();
  if (value & 0x40) p1_6_set(); else p1_6_clr();
  if (value & 0x80) p1_7_set(); else p1_7_clr();
}

/* ----------------------------------------------------------
                          p1_bitwrite

     schreibt ein einzelnes Bit auf dem virtuellen Port P1.
     Unabhaengig davon in welcher Konfiguration der Ausgabe-
     pin war, er wird neu als Ausgang initialisiert.

     Bsp.: p1_bitwrite(2,1);   // setzt Portbit 2 auf "1"
   ---------------------------------------------------------- */
void p1_bitwrite(uint8_t bitnr, uint8_t value)
{
  switch (bitnr)
  {
    case 0 :
    {
      p1_0_out_init();
      if (value) p1_0_set(); else p1_0_clr();
      break;
    }

    case 1 :
    {
      p1_1_out_init();
      if (value) p1_1_set(); else p1_1_clr();
      break;
    }

    case 2:
    {
      p1_2_out_init();
      if (value) p1_2_set(); else p1_2_clr();
      break;
    }

    case 3:
    {
      p1_3_out_init();
      if (value) p1_3_set(); else p1_3_clr();
      break;
    }

    case 4:
    {
      p1_4_out_init();
      if (value) p1_4_set(); else p1_4_clr();
      break;
    }

    case 5:
    {
      p1_5_out_init();
      if (value) p1_5_set(); else p1_5_clr();
      break;
    }

    case 6:
    {
      p1_6_out_init();
      if (value) p1_6_set(); else p1_6_clr();
      break;
    }

    case 7:
    {
      p1_7_out_init();
      if (value) p1_7_set(); else p1_7_clr();
      break;
    }

    default: break;
  }
}

/* ----------------------------------------------------------
                          p1_byteread

     liest ein komplettes Byte vom virtuellen Port P1 ein
   ---------------------------------------------------------- */
uint8_t p1_byteread(void)
{
  uint8_t value;

  p1_config(0x00);               // alle Pins Eingaenge
  value= 0;

  if is_p1_0() value |= 0x01;
  if is_p1_1() value |= 0x02;
  if is_p1_2() value |= 0x04;
  if is_p1_3() value |= 0x08;
  if is_p1_4() value |= 0x10;
  if is_p1_5() value |= 0x20;
  if is_p1_6() value |= 0x40;
  if is_p1_7() value |= 0x80;

  return value;
}


/* ----------------------------------------------------------
                          p1_bitread

     liest ein einzelnes Bit vom virtuellen Port P1.
     Unabhaengig davon in welcher Konfiguration der Eingangs--
     pin war, er wird neu als Eingang initialisiert.

     Bsp.: b= p1_bitread(3);   // liest Bit Nr. 3 vom
                               // virtuellen Port1
   ---------------------------------------------------------- */
uint8_t p1_bitread(uint8_t bitnr)
{
  switch (bitnr)
  {
    case 0 :
    {
      p1_0_in_init()
      if (is_p1_0()) return 1; else return 0;
      // break;                 // break kann hier entfallen, da sowieso
                                // Position nicht erreicht werden kann
    }

    case 1 :
    {
      p1_1_in_init()
      if (is_p1_1()) return 1; else return 0;
      // break;

    }

    case 2 :
    {
      p1_2_in_init()
      if (is_p1_2()) return 1; else return 0;
      // break;

    }

    case 3 :
    {
      p1_3_in_init()
      if (is_p1_3()) return 1; else return 0;
      // break;

    }

    case 4 :
    {
      p1_4_in_init()
      if (is_p1_4()) return 1; else return 0;
      // break;

    }

    case 5 :
    {
      p1_5_in_init()
      if (is_p1_5()) return 1; else return 0;
      // break;

    }

    case 6 :
    {
      p1_6_in_init()
      if (is_p1_6()) return 1; else return 0;
      // break;

    }

    case 7 :
    {
      p1_7_in_init()
      if (is_p1_7()) return 1; else return 0;
      // break;

    }
    default : break;
  }
}

/* ----------------------------------------------------------
               Definitionen fuer virtual Port 2
   ---------------------------------------------------------- */

/* ----------------------------------------------------------
                          p2_config

     konfiguriert den "virtuellen" Port p2 im Stile, wie
     das auch in einem AVR vorgenommen wird. Sinn des
     "virtuellen" Ports ist, dass "zusammengewuerfelte" Pins
     verschiedener Ports zu einem 8 Bit = 1 Byte Ausgabe-
     Eingabeports zusammen gefasst werden. Gn eschwindigkeits-
     wunder duerfen keine erwartet werden.

     dirbyte:

         einzelne 1 an der Bitposition entspricht einem
         Ausgang, eine 0 einem Eingang

     Bsp.: p2_config(0x93);

         p2_7  p2_6  p2_5  p2_4  p2_3  p2_2  p2_1  p2_0
          out   in    in    in    in    in    out   out
          1     0     0     0     0     0     1     1  = 0x93
   ---------------------------------------------------------- */
void p2_config(uint8_t dirbyte)
{
  if (dirbyte & 0x01) p2_0_out_init(); else p2_0_in_init();
  if (dirbyte & 0x02) p2_1_out_init(); else p2_1_in_init();
  if (dirbyte & 0x04) p2_2_out_init(); else p2_2_in_init();
  if (dirbyte & 0x08) p2_3_out_init(); else p2_3_in_init();
  if (dirbyte & 0x10) p2_4_out_init(); else p2_4_in_init();
  if (dirbyte & 0x20) p2_5_out_init(); else p2_5_in_init();
  if (dirbyte & 0x40) p2_6_out_init(); else p2_6_in_init();
  if (dirbyte & 0x80) p2_7_out_init(); else p2_7_in_init();
}

/* ----------------------------------------------------------
                          p2_bytewrite

     schreibt ein komplettes Byte auf den virtuellen Port
     p2
   ---------------------------------------------------------- */
void p2_bytewrite(uint8_t value)
{
  p2_config(0xff);                               // alle Pins Ausgaenge

  if (value & 0x01) p2_0_set(); else p2_0_clr();
  if (value & 0x02) p2_1_set(); else p2_1_clr();
  if (value & 0x04) p2_2_set(); else p2_2_clr();
  if (value & 0x08) p2_3_set(); else p2_3_clr();
  if (value & 0x10) p2_4_set(); else p2_4_clr();
  if (value & 0x20) p2_5_set(); else p2_5_clr();
  if (value & 0x40) p2_6_set(); else p2_6_clr();
  if (value & 0x80) p2_7_set(); else p2_7_clr();
}

/* ----------------------------------------------------------
                          p2_bitwrite

     schreibt ein einzelnes Bit auf dem virtuellen Port p2.
     Unabhaengig davon in welcher Konfiguration der Ausgabe-
     pin war, er wird neu als Ausgang initialisiert.

     Bsp.: p2_bitwrite(2,1);   // setzt Portbit 2 auf "1"
   ---------------------------------------------------------- */
void p2_bitwrite(uint8_t bitnr, uint8_t value)
{
  switch (bitnr)
  {
    case 0 :
    {
      p2_0_out_init();
      if (value) p2_0_set(); else p2_0_clr();
      break;
    }

    case 1 :
    {
      p2_1_out_init();
      if (value) p2_1_set(); else p2_1_clr();
      break;
    }

    case 2:
    {
      p2_2_out_init();
      if (value) p2_2_set(); else p2_2_clr();
      break;
    }

    case 3:
    {
      p2_3_out_init();
      if (value) p2_3_set(); else p2_3_clr();
      break;
    }

    case 4:
    {
      p2_4_out_init();
      if (value) p2_4_set(); else p2_4_clr();
      break;
    }

    case 5:
    {
      p2_5_out_init();
      if (value) p2_5_set(); else p2_5_clr();
      break;
    }

    case 6:
    {
      p2_6_out_init();
      if (value) p2_6_set(); else p2_6_clr();
      break;
    }

    case 7:
    {
      p2_7_out_init();
      if (value) p2_7_set(); else p2_7_clr();
      break;
    }

    default: break;
  }
}

/* ----------------------------------------------------------
                          p2_byteread

     liest ein komplettes Byte vom virtuellen Port p2 ein
   ---------------------------------------------------------- */
uint8_t p2_byteread(void)
{
  uint8_t value;

  value= 0;

  p2_config(0x00);             // alle Pins Eingaenge

  if is_p2_0() value |= 0x01;
  if is_p2_1() value |= 0x02;
  if is_p2_2() value |= 0x04;
  if is_p2_3() value |= 0x08;
  if is_p2_4() value |= 0x10;
  if is_p2_5() value |= 0x20;
  if is_p2_6() value |= 0x40;
  if is_p2_7() value |= 0x80;

  return value;
}


/* ----------------------------------------------------------
                          p2_bitread

     liest ein einzelnes Bit vom virtuellen Port p2.
     Unabhaengig davon in welcher Konfiguration der Eingangs--
     pin war, er wird neu als Eingang initialisiert.

     Bsp.: b= p2_bitread(3);   // liest Bit Nr. 3 vom
                               // virtuellen Port1
   ---------------------------------------------------------- */
uint8_t p2_bitread(uint8_t bitnr)
{
  switch (bitnr)
  {
    case 0 :
    {
      p2_0_in_init()
      if (is_p2_0()) return 1; else return 0;
      // break;                 // break kann hier entfallen, da sowieso
                                // Position nicht erreicht werden kann
    }

    case 1 :
    {
      p2_1_in_init()
      if (is_p2_1()) return 1; else return 0;
      // break;

    }

    case 2 :
    {
      p2_2_in_init()
      if (is_p2_2()) return 1; else return 0;
      // break;

    }

    case 3 :
    {
      p2_3_in_init()
      if (is_p2_3()) return 1; else return 0;
      // break;

    }

    case 4 :
    {
      p2_4_in_init()
      if (is_p2_4()) return 1; else return 0;
      // break;

    }

    case 5 :
    {
      p2_5_in_init()
      if (is_p2_5()) return 1; else return 0;
      // break;

    }

    case 6 :
    {
      p2_6_in_init()
      if (is_p2_6()) return 1; else return 0;
      // break;

    }

    case 7 :
    {
      p2_7_in_init()
      if (is_p2_7()) return 1; else return 0;
      // break;

    }
    default : break;
  }
}
