/* ------------------------------------------------------------------
                              tm1637.h

     Header zum Ansprechen eines TM16xx, Treiberbaustein fuer
     6 stellige 7-Segmentanzeigen mit gemeinsamer Anode und
     zusaetzlichem Tasteninterface.

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
      ---           _____________________________________________________________________
   f | g | b        Segment   |  e  |  d  |  c  |  g  |  dp | GX2 |  a  |  b  | GX1 |  f
      ---
   e |   | c
      ---
       d

    Grundsaetzlicher Hinweis:

    An den Anschluss GRID1 des TM16xx wird die hoechstwertigste Anzeige angeschlossen.

    ___________________________________________
    
    Bsp. 3 Stck. 2821BS 2-Digit 7-Segmenanzeige
    angeschlossen an TM1637

    TM1637 (Pin)          7-Segmentanzeige (Pin)
    ___________________________________________
                                             ---
    GRID4   15   ---------------  SEG1       6  | 2-Digit
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

#ifndef in_tm1637
  #define in_tm1637

  #include <string.h>
  #include <util/delay.h>
  #include <avr/io.h>

  /* ----------------------------------------------------------
            Anschluss CLK und DIO  an den Controller
     ---------------------------------------------------------- */
  // DIO nach PB0
  #define bb_datport     C
  #define bb_datbitnr    4

  // CLK nach PB1
  #define bb_clkport     C
  #define bb_clkbitnr    5

  // Shift Taste
  #define shift_port     D
  #define shift_bitnr    5

  #define puls_us        1
  #define puls_len()     _delay_us(puls_us)


  /* ----------------------------------------------------------
                       Globale Variable
     ---------------------------------------------------------- */

  extern uint8_t  hellig;                 // beinhaltet Wert fuer die Helligkeit (erlaubt: 0x00 .. 0x0f);
  extern uint8_t  bmp7s[17];              // Bitmuster der einzelnen Ziffern von 0..F, 16= aus

  extern uint8_t  wait_unpress;           // Flag, ob beim Tastendruck gewartet werden soll
                                          // bis Taste wieder losgelassen wurde

  extern uint8_t  wait_shiftunpress;      // nur warten bis Shift-Taste losgelassen wurde

  extern uint8_t tm16_fb[6];

  extern uint8_t showmask;                // Maske der Digits, die angezeigt werden sollen: 0xff fuer alle
                                          // Bsp.: 0x21 zeigt nur das hoechste und das niedrigste Digit an

  // Namen fuer die Anzeigezeichen
  enum { a_pc, a_cell, a_inp, a_err, a_akku, a_o, a_k, a_n, a_u, a_store, a_load, a_h, a_aus };

  extern uint8_t cmdchar[];
  extern uint8_t lastcmdchar;

  #define tm16_bufclr()     memset(tm16_fb, 0, 6)

  /* ----------------------------------------------------------
                           PROTOTYPEN
     ---------------------------------------------------------- */

  void zif2buf(uint8_t ziffer, uint8_t pos);
  void bmp2buf(uint8_t bmp, uint8_t pos);

  void tm16_init(void);
  void tm1638_setdp(uint8_t pos, uint8_t enable);
  void tm16_clear(void);
  void tm16_showbuffer(void);

  void tm16_setbright(uint8_t value);
  void tm16_setbmp(uint8_t DIG, uint8_t value);
  void tm16_setzif(uint8_t DIG, uint8_t zif);
  void tm16_setseg(uint8_t DIG, uint8_t seg);
  void tm16_setdp(uint8_t pos, uint8_t enable);
  void tm16_setdez_u8(uint8_t value, uint8_t pos);
  void tm16_setdez_nodp(uint32_t value);
  void tm16_setdez(uint32_t value, uint8_t cmdchar);
  void tm16_sethex(uint32_t value);
  void tm16_setbin(uint32_t value);
  uint8_t tm16_readkey(void);
  uint8_t tm16_readshiftkeys(void);

  // "Kurznamen" der Funktionen (weil ich zu faul bin, immer den Controllertype
  // vornanzustellen

  #define clear()                    tm16_clear()
  #define showbuffer()               tm16_showbuffer()
  #define setbmp(pos, val)           tm16_setbmp(pos, val)
  #define setdp(pos, enable)         tm16_setdp(pos, enable)
  #define setdez_u8(value, pos)      tm16_setdez_u8(value, pos)
  #define setdez_nodp(val)           tm16_setdez_nodp(val)
  #define setdez(val, cmdchar)       tm16_setdez(val, cmdchar)
  #define sethex(val)                tm16_sethex(val)
  #define setbin(val)                tm16_setbin(val)
  #define readkeys()                 tm16_readkeys()
  #define readshiftkeys()            tm16_readshiftkeys()
  #define bufclr()                   tm16_bufclr()


  // ----------------------------------------------------------------
  // Praeprozessormacros um 2 Stringtexte zur weiteren Verwendung
  // innerhalb des Praeprozessors  zu verknuepfen
  //
  // Bsp.:
  //        #define ionr      A
  //        #define ioport    conc2(PORT, ionr)
  //
  //        ioport wird nun als "PORTA" behandelt
  #define CONC2EXP(a,b)     a ## b
  #define conc2(a,b)        CONC2EXP(a, b)
  // ----------------------------------------------------------------

  // ----------------------------------------------------------------
  //   Makros zum Initialiseren der verwendeten Pins als Ausgaenge
  //   sowie zum Setzen / Loeschen dieser Pins (Bitbanging)
  // ----------------------------------------------------------------

  #define datport           conc2(PORT,bb_datport)
  #define datddr            conc2(DDR,bb_datport)
  #define datpin            conc2(PIN,bb_datport)
  #define clkport           conc2(PORT,bb_clkport)
  #define clkddr            conc2(DDR,bb_clkport)

  #define shport            conc2(PORT,shift_port)
  #define shddr             conc2(DDR,shift_port)
  #define shpin             conc2(PIN,shift_port)

  // ----------------------------------------------------------------
  //   SCK und DIO
  //   Anmerkung zum Setzen von 1 und 0 auf den Pins
  //
  //   Die Pins sind in der Grundkonfiguration als Eingang geschaltet.
  //   Beim Setzen einer 1 wird nur die Konfiguration des Pins als
  //   Eingang benoetigt, da dieser dann hochohmig ist und die Leitung
  //   ueber den Pull-Up Widerstand auf 1 gelegt wird.
  //   Bei der Ausgabe einer 0 wird der Pin als Ausgang konfiguriert
  //   und dieser Pin mit einer 0 beschrieben
  // ----------------------------------------------------------------
  #define sda_init()        datddr &= ~(1 << bb_datbitnr)
  #define bb_sda_hi()       sda_init()
  #define bb_sda_lo()       { datddr |= (1 << bb_datbitnr);  datport&= (~(1 << bb_datbitnr)); }
  #define bb_is_sda()       (( datpin & ( 1 << bb_datbitnr) ) >> bb_datbitnr )

  #define scl_init()        datddr &= ~(1 << bb_clkbitnr)
  #define bb_scl_hi()       scl_init()
  #define bb_scl_lo()       { clkddr |= (1 << bb_clkbitnr);  clkport&= (~(1 << bb_clkbitnr)); }

  // ----------------------------------------------------------------
  //   Shift-Taste
  //   Portpin als Eingang mit eingeschaltetem Pull-Up Widerstand
  // ----------------------------------------------------------------
  #define shift_init()      { shddr &= ~(1 << shift_bitnr); shport |= (1 << shift_bitnr); }
  #define is_shift()        (!(( shpin & ( 1 << shift_bitnr) ) >> shift_bitnr ))

#endif
