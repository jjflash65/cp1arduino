/* ------------------------------------------------------------------
                                tm1637.cpp

     Softwaremodul zum Ansprechen eines TM1637, Treiberbaustein fuer
     6 stellige 7-Segmentanzeigen mit gemeinsamer Anode und
     zusaetzlichem Tasteninterface.

     Geeignet fuer: TM1651, TM1637

     MCU   : ATmega
     F_CPU : 8 MHz intern

     Pinbelegung

         ATmega
     _____________________________

          PD5         Shift-Taste
     _____________________________

                        TM1637
     _____________________________
          PC5             CLK
          PC4             DIO

     CLK und DIO sind mit jeweils 2,2 kOhm Pop-Up Wider-
     staenden nach +5V zu versehen

     AERGERLICH: TM1637 kann KEINE kombinierten Tasten erfassen, da
                 aber eine Doppelbelegung der Tasten vorgesehen ist
                 wird hier eine Shift- (Umschalt-) Taste direkt an
                 einen Portpin des ATmega angeschlossen !


     27.12.2020 R. Seelig
   ------------------------------------------------------------------ */

/*
                            10  9  8  7  6
                             :  :  :  :  :
                           +---------------+
                           |  seg1   seg2  |
                           |   --     --   |
                           |  |  |   |  |  |
                           |   --     --   |
                           |  |  |   |  |  |
                           |   --     --   |
                           +---------------+
                             :  :  :  :  :
                             1  2  3  4  5


       a            Pin-Nr.:  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  | 10
      ---           ---------------------------------------------------------------------
   f | g | b        Segment   |  e  |  d  |  c  |  g  |  dp | GX2 |  a  |  b  | GX1 |  f
      ---
   e |   | c
      ---
       d

    Grundsaetzlicher Hinweis:

    An den Anschluss GRID1 des TM16xx wird die hoechstwertigste Anzeige angeschlossen.

    -------------------------------------------

    Bsp. Anschluesse fuer TM1637 an 3 Stck.
    2821BS 2-Digit 7-Segmentanzeige

    TM1637 (Pin)          7-Segmentanzeige (Pin)
    --------------------------------------------
                                             ---
    GRID4   15   ---------------  SEG1       6  | 2-Digt
    GRID3   14   ---------------  SEG2       8  | Anzeige 1
                                             ---
    GRID2   13   ---------------  SEG3       6  |
    GRID1   12   ---------------  SEG4       8  | Anzeige 2
                                             ---
                                             ---
    GRID2   11   ---------------  SEG5       6  |
    GRID1   10   ---------------  SEG6       8  | Anzeige 3
                                             ---
    SEG1     2   ---------------    a        7
    SEG2     3   ---------------    b        8
    SEG3     4   ---------------    c        3
    SEG4     5   ---------------    d        2
    SEG5     6   ---------------    e        1
    SEG6     7   ---------------    f       10
    SEG7     8   ---------------    g        4
    SEG8     9   ---------------   dp        5

    VDD     16
    GND      1
    K2      20 (Keyscan 2)
    K1      19 (Keyscan 1)
    CLK     18
    DIO     17

*/

#include "cp1_tm1637.h"

/* ----------------------------------------------------------
                     Globale Variable
   ---------------------------------------------------------- */

uint8_t   hellig     = 15;                // beinhaltet Wert fuer die Helligkeit (erlaubt: 0x00 .. 0x0f);

uint8_t   bmp7s[17]  =                    // Bitmuster der einzelnen Ziffern von 0..F, 16= aus
               { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
                 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00 };

uint8_t   wait_unpress = 0;
uint8_t   wait_shiftunpress = 1;

uint8_t   tm16_fb[6];

uint8_t   showmask = 0x3f;                // Maske der Digits, die angezeigt werden sollen: 0xff fuer alle
                                          // Bsp.: 0x21 zeigt nur das hoechste und das niedrigste Digit an
// Bitmaps fuer 8 Kommandozeichen
// Bitmaps fuer Kommandozeichen
//               :       P     C     E     F     A     o     k     n     u     S     L    H     aus
uint8_t   cmdchar[] = { 0x73, 0x39, 0x79, 0x71, 0x77, 0x5c, 0x70, 0x54, 0x1c, 0x6d, 0x38, 0x74, 0x00 };
uint8_t   lastcmdchar= a_pc;


/*  ------------------- Kommunikation -----------------------

    Der Treiberbaustein tm16 wird etwas "merkwuerdig
    angesprochen. Er verwendet zur Kommunikation ein I2C
    Protokoll, jedoch OHNE eine Adressvergabe. Der Chip ist
    somit IMMER angesprochen. Aus diesem Grund wird die
    Kommunikation mittels Bitbanging vorgenommen. Hierfuer
    kann jeder freie I/O Anschluss des Controllers verwendet
    werden (siehe defines am Anfang).

    Ausserdem erfolgt im Gegensatz zu vielen andern I2C Bau-
    steinen die serielle Ausgabe mit dem niederwertigsten Bit
    (LSB first)
   ---------------------------------------------------------- */

void tm16_start(void)              // I2C Bus-Start
{
  bb_scl_hi();
  bb_sda_hi();
  puls_len();
  bb_sda_lo();
}

void tm16_stop(void)               // I2C Bus-Stop
{
  bb_scl_lo();
  puls_len();
  bb_sda_lo();
  puls_len();
  bb_scl_hi();
  puls_len();
  bb_sda_hi();
}

void tm16_write (uint8_t value)    // I2C Bus-Datentransfer
{
  uint8_t i;

  for (i = 0; i <8; i++)
  {
    bb_scl_lo();

    //  serielle Bitbangingausgabe, LSB first
    if (value & 0x01) { bb_sda_hi(); }
                   else { bb_sda_lo(); }
    puls_len();
    value = value >> 1;
    bb_scl_hi();
    puls_len();
  }
  bb_scl_lo();
  puls_len();                        // der Einfachheit wegen wird ACK nicht abgefragt
  bb_scl_hi();
  puls_len();
  bb_scl_lo();

}

/* -------------------------------------------------------
                    tm16_read(uint8_t ack)

   liest ein Byte vom I2c Bus.

   Uebergabe:
               1 : nach dem Lesen wird dem Slave ein
                   Acknowledge gesendet
               0 : es wird kein Acknowledge gesendet

   Rueckgabe:
               gelesenes Byte
   ------------------------------------------------------- */
uint8_t tm16_read(uint8_t ack)
{
  uint8_t data= 0x00;
  uint8_t i;

  bb_sda_hi();

  for(i= 0; i< 8; i++)
  {
    bb_scl_lo();
    puls_len();
    bb_scl_hi();

    puls_len();

    if(bb_is_sda()) data|= (1 << i);
  }

  bb_scl_lo();
  bb_sda_hi();

  puls_len();

  if (ack)
  {
    bb_sda_lo();
    puls_len();
  }

  bb_scl_hi();
  puls_len();

  bb_scl_lo();
  puls_len();

  bb_sda_hi();

  return data;
}


/*  ----------------------------------------------------------
                      Benutzerfunktionen
    ---------------------------------------------------------- */

/*  ---------------------------------------------------------
                             zif2buf

       setzt eine Ziffer in den Framebuffer ein. Das Bitmap
       der Ziffer ist im globalen Array bmp7s abgelegt.

       Uebergabe:

              ziffer : einzufuegende Ziffer (0 .. 15)
              pos    : Position der Ziffer ( links = 0)
    --------------------------------------------------------- */
void zif2buf(uint8_t ziffer, uint8_t pos)
{
  tm16_fb[pos]= bmp7s[ziffer];
  return;
}

/*  ---------------------------------------------------------
                             bmp2buf

       setzt ein Bitmuster in den Framebuffer ein.

       Uebergabe:

              bmp    : einzufuegendes Bitmap
              pos    : Position der Ziffer ( links = 0)
    --------------------------------------------------------- */
void bmp2buf(uint8_t bmp, uint8_t pos)
{
  tm16_fb[pos]= bmp;
}


 /*  ---------------------------------------------------------
                           tm16_selectpos

        waehlt die zu beschreibende Anzeigeposition aus
     --------------------------------------------------------- */
void tm16_selectpos(char nr)
{
  tm16_start();
  tm16_write(0x40);                // Auswahl LED-Register
  tm16_stop();

  tm16_start();
  tm16_write(0xc0 | nr);           // Auswahl der 7-Segmentanzeige
}

/*  ----------------------------------------------------------
                           tm16_setbright

       setzt die Helligkeit der Anzeige
       erlaubte Werte fuer Value sind 0 .. 15
    ---------------------------------------------------------- */
void tm16_setbright(uint8_t value)
{
  tm16_start();
  tm16_write(0x80 | value);        // unteres Nibble beinhaltet Helligkeitswert
  tm16_stop();
}

/*  ---------------------------------------------------------
                             tm16_clear

       loescht die Anzeige auf dem Modul
    --------------------------------------------------------- */
void tm16_clear(void)
{
  uint8_t i;

  tm16_selectpos(0);
  for(i=0; i<6; i++) { tm16_write(0x00); }
  tm16_stop();

  tm16_setbright(hellig);

}

/*  ---------------------------------------------------------
                            tm16_setbmp

       gibt ein Bitmapmuster an einer Position aus
    --------------------------------------------------------- */
void tm16_setbmp(uint8_t pos, uint8_t value)
{
  tm16_selectpos(pos);             // zu beschreibende Anzeige waehlen

  tm16_write(value);               // Bitmuster value auf 7-Segmentanzeige ausgeben
  tm16_stop();
}

/*  ---------------------------------------------------------
                            tm16_showbuffer

       zeigt den 6 Byte grossen Pufferspeicher tm16_fb
       auf den 7-Segmentanzeigen an.

       tm16_fb[0] wird links angezeigt

       Uebergabe:
            *buffer   : Zeiger auf einen 8 Byte grossen
                        Pufferspeicher
    --------------------------------------------------------- */
void tm16_showbuffer(void)
{
  uint8_t i;

  for (i= 0; i< 6; i++)
  {
    tm16_setbmp(i, tm16_fb[i]);
  }
}

/*  ---------------------------------------------------------
                            tm16_setzif

       gibt ein Ziffer an einer Position aus
       Anmerkung: das Bitmuster der Ziffern ist in
                  bmp7s definiert
    --------------------------------------------------------- */
void tm16_setzif(uint8_t pos, uint8_t zif)
{
  tm16_selectpos(pos);             // zu beschreibende Anzeige waehlen

  zif= bmp7s[zif];
  tm16_write(zif);               // Bitmuster value auf 7-Segmentanzeige ausgeben
  tm16_stop();

}
/*  ---------------------------------------------------------
                            tm16_setseg

       setzt ein einzelnes Segment einer Anzeige

       pos: Anzeigeposition (0..3)
       seg: das einzelne Segment (0..7 siehe oben)
    --------------------------------------------------------- */
void tm16_setseg(uint8_t pos, uint8_t seg)
{

  tm16_selectpos(pos);             // zu beschreibende Anzeige waehlen
  tm16_write(1 << seg);
  tm16_stop();
}

/*  ---------------------------------------------------------
                          tm16_setdp

      setzt oder loescht einen Dezimalpunkt

      Uebergabe:
         pos    : Position, an der ein Dezimalpunkt gesetzt
                  oder geloescht wird
         enable : 1 = dp wird gesetzt
                  0 = dp wird geloescht
    --------------------------------------------------------- */
void tm16_setdp(uint8_t pos, uint8_t enable)
{
  if (enable)
    tm16_fb[5-pos] |= 0x80;
  else
    tm16_fb[5-pos] &= 0x7f;
  tm16_showbuffer();
}

/*  ---------------------------------------------------------
                        tm16_setdez_u8

       gibt einen 8-Bit Integer dezimal auf dem Display
       an gegebener Position aus

       Uebergabe:

         value   : auszugebender Wert
         pos     : Ausgabeposition, 0 = links
    --------------------------------------------------------- */
void tm16_setdez_u8(uint8_t value, uint8_t pos)
{
  int32_t teiler = 100;
  uint8_t i,v, bmp;

  tm16_bufclr();
  for (i= 0; i< 3; i++)
  {
    v= value / teiler;
    value= value - ( v * teiler);
    teiler /= 10;
    zif2buf(v, i+pos);
  }  

  for (i= 0; i< 6; i++)
  {
    if (!(showmask & (1 << (5-i)))) zif2buf(16,i);    // nur Ziffern anzeigen, die in der Maske
                                                      // selektiert sind
  }
  tm16_showbuffer();
}

/*  ---------------------------------------------------------
                        tm16_setdez_nodp

       gibt einen 6-stelligen dezimalen Wert auf der
       Anzeige aus ohne Dezimalpunktsteuerung

       Uebergabe:

         value   : auszugebender Wert
    --------------------------------------------------------- */
void tm16_setdez_nodp(uint32_t value)
{
  int32_t teiler = 100000;
  uint8_t i,v, bmp;

  tm16_bufclr();
  for (i= 0; i< 6; i++)
  {
    v= value / teiler;
    value= value - ( v * teiler);
    teiler /= 10;
    zif2buf(v, i);

    if (!(showmask & (1 << (5-i)))) zif2buf(16,i);    // nur Ziffern anzeigen, die in der Maske
                                                      // selektiert sind
  }
  tm16_showbuffer();
}

/*  ---------------------------------------------------------
                         tm16_setdez

       gibt einen 6-stelligen dezimalen Wert auf der
       Anzeige aus

       Uebergabe:

         value   : auszugebender Wert
         cmd     : Anzeige Kommandozeichen link
                   0 => wird angezeigt
                   1 => wird nicht angezeigt
    --------------------------------------------------------- */
void tm16_setdez(uint32_t value, uint8_t cmd)
{
  tm16_setdez_nodp(value);
  if (!cmd)
  {
    bmp2buf(cmdchar[lastcmdchar], 0);
  }
  tm16_setdp(3,1);
}

/*  ---------------------------------------------------------
                         tm16_sethex

       gibt einen 6-stelligen hexadezimalen Wert auf der
       Anzeige aus
    --------------------------------------------------------- */
void tm16_sethex(uint32_t value)
{
  int32_t teiler = 0x100000;
  uint8_t i,v;

  tm16_bufclr();
  for (i= 0; i< 6; i++)
  {
    v= value / teiler;
    value= value - ( v * teiler);
    teiler /= 16;
    zif2buf(v, i);

    if (!(showmask & (1 << (5-i)))) zif2buf(16,i);    // nur Ziffern anzeigen, die in der Maske
                                                      // selektiert sind
  }
  tm16_showbuffer();
}

/*  ---------------------------------------------------------
                         tm16_setbin

       gibt einen 6-stelligen binaeren Wert auf der
       Anzeige aus
    --------------------------------------------------------- */
void tm16_setbin(uint32_t value)
{
  int32_t teiler = 32;
  uint8_t i,v;

  tm16_bufclr();
  for (i= 0; i< 6; i++)
  {
    v= value / teiler;
    value= value - ( v * teiler);
    teiler /= 2;
    zif2buf(v, i);

    if (!(showmask & (1 << (5-i)))) zif2buf(16,i);    // nur Ziffern anzeigen, die in der Maske
                                                      // selektiert sind
  }
  tm16_showbuffer();
}
/*  ---------------------------------------------------------
                          tm16_readkey

      liest angeschlossene Tasten ein und gibt dieses als
      Argument zurueck.

      Anmerkung:
        Es wird keine Tastenmatrix zurueck geliefert. Ist
        mehr als eine Taste aktiviert, wird nur die hoechste
        Taste zurueck geliefert. Somit ist es nicht moeglich
        mehrere Tasten gleichzeitig zu betaetigen.
    --------------------------------------------------------- */
uint8_t tm16_readkey(void)
{
  uint8_t key;

  key= 0;
  tm16_start();
  tm16_write(0x42);
  key= ~tm16_read(1);
  tm16_stop();
  if (key) key -= 8; else key= 0xff;
  return key;
}

/*  ---------------------------------------------------------
                          tm16_readhiftkeys

      liest angeschlossene Tasten ein und ermittelt eine
      evtl. zusaetzliche gedrueckte Shift-Taste.

      Bei gedrueckter Shift-Taste wird dem Tastenwert
      0x80 als Kennung dafuer, dass Shift-Taste gedrueckt
      hinzuaddiert.

      Bsp. Taste 5 ohne Shift => 0x05
                   mit Shift  => 0x85

    --------------------------------------------------------- */
uint8_t tm16_readshiftkeys(void)
{
  uint8_t shflag;
  uint8_t key;

  shflag= 0;
  if (is_shift()) shflag= 1;
  key= tm16_readkey();
  if (key== 0xff) return 0xff;          // es wurde keine Taste gedrueckt

  // warten bis Taste und Shift-Taste losgelassen sind

  if (wait_unpress)
    while ((is_shift()) || (tm16_readkey() != 0xff));
  if (wait_shiftunpress)
    while (is_shift());

  if (shflag) return (0x80 | key); else return key;
}

/*  ---------------------------------------------------------
                           tm16_init

       initialisiert die Anschluesse des Controllers zur
       Kommunikation als Ausganege, zusaetzlich den Anschluss
       der Shift-Taste als Anzeige.

       Anzeige wird geloescht
    ---------------------------------------------------------- */
void tm16_init(void)
{
  shift_init();
  scl_init();
  sda_init();
  tm16_clear();
}
