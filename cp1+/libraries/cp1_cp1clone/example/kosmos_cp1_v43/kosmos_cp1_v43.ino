/* ----------------------------------------------------------
                       kosmos_cp1_v41.ino

     Spassprojekt: Emulation des  "Lerncomputer" Kosmos CP1
     aus dem Jahre 1983. Dieser bassierte auf einem Mikro-
     controller 8049 und hatte einen virtuellen Prozessor
     als Firmware

     Hardware:
       China I/O Board mit TM1687 7-Segmentanzeige Controler,
       hier mit 8 Digits und 16 Tasten.

     Der originale KOSMOS CP1 besitzt 30 Tasten wobei das
     Ziffernfeld 0..9 parallel geschaltete Tasten sind.

     Um mit dem I/O Board die zur Steuerung notwendigen
     20 Tastenfunktionen zu erreichen, sind 10 Tasten
     doppelt belegt. Ohne gedrueckte Shifttaste werden
     Ziffern eingegeben, mit gedrueckter Shifttaste werden
     die Funktionstasten ausgewaehlt.

     Tastenbelegung:

               ..... ..... ..... ...... .....
              : cal : run : stp : step : acc :
              : 0   : 1   : 2   : 3    : 4   :
               ..... ..... ..... ...... .....
               ..... ..... ..... ...... .....
     .......  : clr : pc  : out : inp  : cas :
    : SHIFT : : 5   : 6   : 7   : 8    : 9   :
     .......   ..... ..... ..... ...... .....


     Zuordnung AVR Pins zu den Ports 1 und 2 des Kosmos CP1
     (siehe ./include/vports.h)

     virtual P1  :  P1_0  P1_1  P1_2  P1_3  P1_4  P1_5  P1_6  P1_7
     AVR-Portpins:   PD6   PD7   PB0   PB1   PB2   PB3   PB4   PB5

     virtual P2  :  P2_0  P2_1  P2_2  P2_3  P2_4  P2_5  P2_6  P2_7
     AVR-Portpins:   PC0   PC1   PC2   PC3   PD4   PD3   PD2   PD0


     MCU  :   ATmega328p
     Takt :   8 MHz

     10.01.2021       R. Seelig
   ---------------------------------------------------------- */


#include "kosmos_cp1_v43.h"

#define puts(txt)           uart_putromstring(PSTR(txt))  // String aus Flashspeicher anzeigen
#define puts_ram(txt)       uart_putramstring(txt);

#define  delay    _delay_ms


/*  ---------------------------------------------------------
      globale Variable
    --------------------------------------------------------- */

kcomp     cp1;                    // das "Gesamtsystem" in einer Struktur
uint8_t   err= 0;                 // Fehlercode
uint8_t   swint10_dp = 0;         // Maske fuer Dezimalpunktanzeige Softwareinterrupt 10

volatile  uint32_t millis_t0 = 0;    // Timerticker

uint8_t   seg7_bright = 12;        // Helligkeit der 7-Segmentanzeigen


/*  ---------------------------------------------------------
                   ISR - Timer0 compare 0

      wird jede Millisekunde aufgerufen und zaehlt millis_t0
      hoch
    --------------------------------------------------------- */
ISR (TIMER0_COMPA_vect)
{
  millis_t0++;
}

/*  ---------------------------------------------------------
                          timer0_init

      initialisiert Timer0 fuer Interruptaufruf jede Milli-
      sekunde
    --------------------------------------------------------- */
void timer0_init(void)
{
  // Modus compare match A (CTC) ==> WGM 02:01:00 = 0:1:0
  // clk-scaler / 64,            ==> CS0 02:01:00 = 0:1:1
  // comp-value = 250            ==> 250 * 64 = 160000 ( Wert fuer 16 MHz )
  // comp-value = 125            ==> 125 * 64 =  80000 ( Wert fuer  8 MHz )

  TCCR0A = 1 << WGM01;
  TCCR0B = (1 << CS01) | (1 << CS00);

  #if (F_CPU > 10000000)
    OCR0A = 250;
  #else
    OCR0A = 125;
  #endif
  TCNT0 = 0;

  TIMSK0 = 1 << OCIE0A;           // Timerinterruptfreigabe fuer compare match a
  cli();                          // Interrupt nur bei cp1_delay starten
}

/*  ---------------------------------------------------------
                          tim1_delay

      Verzoegerungsschleife ueber im Timer1 - Interrupt
      hochgezaehlte Variable millis_t0.

      Verzoegert die Programmausfuehrung um dtime * 1 ms
    --------------------------------------------------------- */
void tim1_delay(uint32_t dtime)
{
  volatile uint32_t now;

  now= millis_t0;

  while(now + dtime > millis_t0);
}

/* --------------------------------------------------------
                         get16zufall

      generiert eine 16-Bit grosse Pseudozufallszahl die
      mittels mitgekoppeltem Schieberegister erreicht wird

    Uebergabe:
      startwert  : Ausgangspunkt im Zahlenbereich
      xormask    : Maske beim Schieben, hierdurch kann
                   ein Zahlenbereich eingeschraenkt werden

          xormask   Anzahl unterschiedlicher Zahlen
          -----------------------------------------
          0xb400                65535
          0x07c1                 2047
          0x0be0                 4095
  -------------------------------------------------------- */
uint16_t get16zufall(uint16_t startwert, uint16_t xormask)
{
  // Variable MUESSEN static sein, damit das Schieberegister
  // mit jedem Tick von der vorherigen Position aus weiter
  // schiebt
  static uint16_t start_state= 1;
  static uint16_t lfsr= 1;
  static int8_t first = 0;

  uint16_t lsb;

  if (first== 0)
  {
    first= 1;
    start_state= startwert;
    lfsr= start_state;
  }

  lsb = lfsr & 1;                        // niederwertigstes Bit des Schieberegisters
  lfsr = lfsr >> 1;                      // das Schieberegister, eine Stelle nach rechts
  if (lsb) { lfsr ^= xormask; }          // wenn LSB gesetzt, XOR-Togglemaske auf SR anwenden

  return lfsr;
}

/* -------------------------------------------------
     Lookup-table fuer NTC-Widerstand
     R25-Widerstandswert muss genauso groß wie der
     PullUp-Widerstandswert sein
     Materialkonstante beta: 3950
     Aufloesung des ADC: 10 Bit
     Einheit eines Tabellenwertes: 0.1 Grad Celcius
     Temperaturfehler der Tabelle: 0.5 Grad Celcius
   -------------------------------------------------*/
const int PROGMEM ntctable[] = {
  1269, 1016, 763, 621, 520, 439, 370, 308,
  250, 194, 139, 83, 22, -47, -132, -256,
  -380
};

/* -------------------------------------------------
                     ntc_gettemp

    zuordnen des Temperaturwertes aus gegebenem
    ADC-Wert.
   ------------------------------------------------- */
int ntc_gettemp(uint16_t adc_value)
{
  int p1,p2;

  // Stuetzpunkt vor und nach dem ADC Wert ermitteln.
  p1 = pgm_read_word(&(ntctable[ (adc_value >> 6)    ]));
  p2 = pgm_read_word(&(ntctable[ (adc_value >> 6) + 1]));

  // zwischen beiden Punkten interpolieren.
  return p1 - ( (p1-p2) * (adc_value & 0x003f) ) / 64;
}


/*  ---------------------------------------------------------
                             anz_clr

      loescht die Anzeige und es ist nur der Punkt zu
      sehen
    --------------------------------------------------------- */
void anz_clr(void)
{
  showmask= 0x00;
  bufclr();
  setdp(3,1);
}

/*  ---------------------------------------------------------
                          keyb_input

      liest einen dezimalen Wert auf der Tastatur und gibt
      in den Referenzen den Opcode, das Datum, die Anzahl
      der gedrueckten Tasten und die Funktionstaste
      zurueck, mit der die Eingabe beendet wurde

      Funktion wird beendet wenn eine der Shifttasten

      In *opc stehen die oberen beiden Digits als Dezimal-
      wert und stellen den Opcode des CP1 dar, die unteren
      3 Digits beinhalten ein Byte in dezimaler Darstellung
    --------------------------------------------------------- */
void keyb_input(uint8_t *opc, uint16_t *data, uint8_t *kanz, uint8_t *fkey)
{
  uint8_t  key, anz;
  uint32_t inp;

  showmask= 0;

  anz= 0;  inp= 0;
  lastcmdchar= a_aus;

  while(1)
  {
    // wiederholen, bis eine Taste gedrueckt ist
    do
    {
      key= readshiftkeys();
      delay(20);
    }while (key== 0xff);

    // warten bis die Taste losgelassen wurde
    while( readshiftkeys() != 0xff) delay(20);

    if (key & 0x80)             // es wurde eine Funktionstaste gedrueckt
    {
      *fkey= key;
      *data= inp %1000;
      *opc= inp / 1000;
      *kanz= anz;
      return;
    }

    // es war Ziferntaste
    anz++;
    showmask= (showmask << 1) | 1;

    if (anz == 6)             // 6 Eingaben sind unzulaessig
    {
      *kanz= anz;
      return;
    }
    else
    {
      inp= (inp*10)+key;
      if (inp> ((int32_t)opcmax * 1000)) { err= 4; return; }
    }

    setdez(inp,0);
  }
}

/*  ---------------------------------------------------------
                             selftest

      wird gestartet mit 9 + run
    --------------------------------------------------------- */
void selftest(void)
{
  int32_t z;

  showmask= 0x3f;

  for (z= 999999; z> -1; z -= 111111)
  {
    setdez(z,1);
    tm16_setdp(3,1);
    delay(150);
  }
}

/*  ---------------------------------------------------------
                           cp1_delay

      Zeitverzoegerung um ca. 1ms. Kann durch STP-Taste
      abgebrochen werden.
      
      Schaltet den Timerinterrupt ein und bei Verlassen der
      Funktion wieder aus

      Rueckgabe:
        0x00 : Zeit durchgelaufen
        0x82 : STP wurde aktiviert
    --------------------------------------------------------- */
uint8_t cp1_delay(uint32_t dtime)
{
  volatile uint32_t now;
  volatile uint8_t key;

  now= millis_t0;

  TCNT0= 0;
  sei();                         // Interrupt zulassen
  while(now + dtime > millis_t0)
  {
    key= readshiftkeys();
    if (key== 0x82)
    {
      cli();                     // Interrupt stoppen
      return 0x82;
    }
  }
  cli();
  return 0;
}

/*  ---------------------------------------------------------
                             mem2opc

      wandelt eine im Memory gespeicherte Kombination von
      Opcode und Datum in eine Dezimalzahl
    --------------------------------------------------------- */
uint16_t mem2opc(uint16_t value)
{
  return (((value & 0xff00) >> 8) * 1000) + (value & 0xff);
}


/*  ---------------------------------------------------------
                          store_showok

      zeigt auf der Anzeige: "S     ok" an
    --------------------------------------------------------- */
void store_showok(void)
{
  // um linke Stelle und die rechten unteren 3 auszuwaehlen
  showmask= 0x27;
  setdez(0,0);

  bmp2buf(cmdchar[a_aus],3);
  bmp2buf(cmdchar[a_o],4);
  bmp2buf(cmdchar[a_k],5);
  bmp2buf(cmdchar[a_store],0);
  showmask= 0x27;
  setdp(3,1);
}

/*  ---------------------------------------------------------
                       store_showrunning

      zeigt auf der Anzeige: "S     n" an
    --------------------------------------------------------- */
void store_showrunning(void)
{
  // um linke Stelle und die rechten unteren 3 auszuwaehlen
  showmask= 0x27;
  setdez(0,0);

  bmp2buf(cmdchar[a_aus],3);
  bmp2buf(cmdchar[a_aus],4);
  bmp2buf(cmdchar[a_n],5);
  bmp2buf(cmdchar[a_store],0);
  showmask= 0x27;
  setdp(3,1);
}

/*  ---------------------------------------------------------
                          load_showok

      zeigt auf der Anzeige: "L     ok" an
    --------------------------------------------------------- */
void load_showok(void)
{
  // um linke Stelle und die rechten unteren 3 auszuwaehlen
  showmask= 0x27;
  setdez(0,0);

  bmp2buf(cmdchar[a_aus],3);
  bmp2buf(cmdchar[a_o],4);
  bmp2buf(cmdchar[a_k],5);
  bmp2buf(cmdchar[a_load],0);
  showmask= 0x27;
  setdp(3,1);
}


/*  ---------------------------------------------------------
                       load_showrunning

      zeigt auf der Anzeige: "L     u" an
    --------------------------------------------------------- */
void load_showrunning(void)
{
  // um linke Stelle und die rechten unteren 3 auszuwaehlen
  showmask= 0x27;
  setdez(0,0);

  bmp2buf(cmdchar[a_aus],3);
  bmp2buf(cmdchar[a_aus],4);
  bmp2buf(cmdchar[a_u],5);
  bmp2buf(cmdchar[a_load],0);
  showmask= 0x27;
  setdp(3,1);
}

/*  ---------------------------------------------------------
                           reaktionstest

      ein kurzes "Spiel" wie es im originalen KOSMOS abge-
      legt ist (zumindest so angenommen, weil es nur eine
      Beschreibung im Manual gibt). Wird durch 8 + RUN
      gestartet.


    --------------------------------------------------------- */
void reaktionstest(void)
{
  uint16_t  i, i2, dtime;
  uint8_t   key, keybreak;

  anz_clr();
  dtime= get16zufall(48, 0x0be0) % 6000;         // Verzoegerungszeit nach Starten

                                                // max. 6 Sekunden

  delay(50);
  key= 0; keybreak= 0;

  dtime += 1200;
  showmask= 0x07;
  for (i= 0; i< dtime; i++)
  {
    key= cp1_delay(1);
    if (key!= 0)
    {
      keybreak= 1;
      i= dtime;
    }
  }

  if (!keybreak)
  {
    i= 0; keybreak= 0;
    do
    {
      key= cp1_delay(2);
      if (key== 0x82)
      {
        keybreak= 1;
      }
      else
      {
        i+= 2;
        setdez(i,0);
      }
    } while ((i< 252) && (!keybreak));
  }
  else
  {
    // Anzeige fuer zu frueh gedrueckt
    setdez(0,0);
  }
}

/*  ---------------------------------------------------------
                       registers_show

       zeigt die Inhalte der Register auf dem UART an
    --------------------------------------------------------- */
void registers_show(kcomp *vcp1)
{
  puts("\n\r  PC |  A  |  B  |  C  |  D  |  E  |  SP  |  PSW : ZCLG  |  ERR  |");
  puts("\n\r ----+-----+-----+-----+-----+-----+------+--------------+-------+");
  puts("\n\r ");
  uart_uint8out(vcp1->pc);
  puts(" | ");
  uart_uint8out(vcp1->a);
  puts(" | ");
  uart_uint8out(vcp1->b);
  puts(" | ");
  uart_uint8out(vcp1->c);
  puts(" | ");
  uart_uint8out(vcp1->d);
  puts(" | ");
  uart_uint8out(vcp1->e);
  puts(" | ");
  uart_uint8out(vcp1->sp);
  puts("  |        ");
  if (vcp1->psw & 0x01) { uart_putchar('1'); } else { uart_putchar('0'); }
  if (vcp1->psw & 0x02) { uart_putchar('1'); } else { uart_putchar('0'); }
  if (vcp1->psw & 0x04) { uart_putchar('1'); } else { uart_putchar('0'); }
  if (vcp1->psw & 0x08) { uart_putchar('1'); } else { uart_putchar('0'); }
  puts("  |  ");
  uart_uint8out(err);
  puts("  |");
  puts("\n\n\r");
}

/*  ---------------------------------------------------------
                          prg_moveto

       verschiebt den Speicherinhalt und passt Sprungziele
       innerhalb des zu verschiebenden Blocks an die neuen
       Adressen an
    --------------------------------------------------------- */
void prg_moveto(kcomp *vcp1, uint8_t movadr, uint8_t startadr, uint8_t stopadr)
{
  uint8_t movend;
  uint8_t  *ptr, *dptr, *hptr, *hptr2;
  uint16_t i;
  uint16_t memw;
  int      data, opc;

  if ((startadr != movadr) && (stopadr >= startadr))
  {
    movend= movadr + (stopadr-startadr);
    ptr= (uint8_t*)&vcp1->mem[0];
    dptr= ptr;

    ptr  += startadr*2;
    dptr += movadr*2;
    memmove(dptr, ptr, ((stopadr-startadr)+1)*2);

    ptr= (uint8_t*)&vcp1->mem[0];
    ptr  += startadr*2;

    // frei gewordenen Speicherbereich loeschen
    for (i= startadr; i< stopadr+1; i++)
    {
      if ((i< movadr) || (i> movend))
      {
        *ptr = 0; ptr++;
        *ptr = 0; ptr++;
      }
      else
      {
        ptr += 2;
      }
    }

    for (i= movadr; i< movend+1; i++)
    {
      memw= vcp1->mem[i];
      opc= memw >> 8;
      data= memw & 0xff;

      // Opcodes fuer Spruenge und Call
      if (opc== 9  || opc== 11 || opc== 24 || opc== 51 ||     \
          opc== 52 || opc== 53 || opc== 64)
      {
        // Sprungziel gefunden, das innerhalb der Verschiebung liegt
        if ((data >= startadr) && (data <= stopadr))
        {
          data += (movadr-startadr);
          memw &= 0xff00;
          memw |= data;
          vcp1->mem[i]= memw;
        }
      }
    }
  }

}

/*  ---------------------------------------------------------
                       terminal_showhelp

       zeigt Menu des Terminals an
    --------------------------------------------------------- */
void terminal_showhelp(void)
{
  puts("\n\r ----------------------------------");
  puts("\n\r   CP1-Terminal v0.41");
  puts("\n\r");
  puts("\n\r   2021 by R. Seelig");
  puts("\n\r ----------------------------------\n\r");

  puts("\n\r press a key to enter function\n\r");
  puts("\n\r [a]ssemble       [d]isassemble    [m]emory adress set   single s[t]ep");
  puts("\n\r [l]oad program   [s]tore program  [r]un program         [c]lear and reset");
  puts("\n\r m[o]ve memory    d[u]mp memory");

  puts("\n\n\r [h]elp           [q]uit terminal mode\n\n\r");
}

/*  ---------------------------------------------------------
                        terminal_mode

                   startet das UART-Terminal
    --------------------------------------------------------- */
uint8_t terminal_mode(kcomp *vcp1, uint8_t showhelp, uint8_t showregs)
{
  uint8_t  strsrc[20];
  uint8_t  ch, mch;
  uint16_t prgword;
  uint8_t  memaddr;
  uint8_t  startadr, stopadr, movadr, movend;
  uint8_t  prgnr;
  uint8_t  d, l, w;
  uint8_t  *ptr, *dptr, *hptr, *hptr2;
  uint16_t i, i2;
  uint16_t memw;
  int      data, opc;

  // rS-232 auf Display anzeigen
  showmask= 0x3f;
  setdez(232,0);
  bmp2buf(0x50,0);              // r
  bmp2buf(0x6d,1);              // S
  bmp2buf(0x40,2);              // -
  showmask= 0x3f;
  setdp(3,0);                   // erneuert auch die Anzeige
  uart_init(baudrate);
  uart_clr();

  if (showhelp) terminal_showhelp();
  if (showregs) registers_show(vcp1);

  do
  {
    do
    {
      mch= 0;
      if (uart_ischar()) mch= uart_getchar();
      if (readshiftkeys() == 0x82) mch= 'q';         // STP-Taste auf Tastatur beendet Terminal
    } while (mch== 0);

    switch (mch)
    {
      // Memory m[o]ve
      case 'o' :
      {
        puts("\n\r ---- Move memory ----\n\r");
        puts("\n\rstart address: ");
        ch= uart_readu8int(&startadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }
        puts("\n\rlast address: ");
        ch= uart_readu8int(&stopadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }
        puts("\n\raddress move to: ");
        ch= uart_readu8int(&movadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }
        prg_moveto(vcp1, movadr, startadr, stopadr);

        puts("\n\r");

        break;
      }
      // Disassembler
      case 'd' :
      {
        puts("\n\r ---- Disassemble ----\n\r");
        puts("\n\rfirst address to show: ");
        ch= uart_readu8int(&startadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }
        puts("\n\rlast address to show : ");
        ch= uart_readu8int(&stopadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }

        puts("\n\r");
        for (i= startadr; i < (uint16_t)(stopadr+1); i++)
        {
          puts("\n\r");
          uart_uint8out(i);
          puts(":   ");
          uart_uint16out( mem2opc(vcp1->mem[i]), 2);
          puts("   ");

          disassemble_prgword(&strsrc[0], vcp1->mem[i]);
          puts_ram(&strsrc[0]);

        }
        puts("\n\n\r");
        break;
      }

      // dump memory
      case 'u' :
      {
        puts("\n\r ---- Memory dump ----\n\r");
        puts("\n\rfirst adress to show: ");
        ch= uart_readu8int(&startadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }
        puts("\n\rlast adress to show : ");
        ch= uart_readu8int(&stopadr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
          break;
        }

        puts("\n\r");
        i2= 0;
        for (i= startadr; i < (uint16_t)(stopadr+1); i++)
        {
          if (!(i2 % 10))
          {
            puts("\n\r");
            uart_uint16out((i2+startadr), 0);
            puts(":   ");
          }
          i2++;
          uart_uint16out( mem2opc(vcp1->mem[i]), 2);
          puts("  ");
        }
        puts("\n\n\r");
        break;
      }

      // Memory set
      case 'm' :
      {
        puts("\n\r ---- Set address ----\n\r");
        puts("\n\rset memory address: ");
        ch= uart_readu8int(&memaddr);
        if (ch== 0x82)
        {
          mch= 'q';                                  // Beenden des Terminals vorbereiten
          ch= 0x1b;
        }
        else
        {
          if (memaddr> memsize-1)
           {
            puts("\n\rinvalid memory address\n\r");
          }
          else
          {
            puts("\n\r memaddr and PC set to: ");
            uart_uint8out(memaddr);
            vcp1->addr= memaddr;
            vcp1->pc= memaddr;
            puts("\n\r");
          }
        }
        break;
      }

      // Zeilenassembler
      case 'a' :
      {
        puts("\n\r ---- Assemblermode ----");
        puts("\n\r [esc] quits assemble mode\n\r");
        puts("\n\raddr mnemonic         opcode"),
        puts("\n\r--------------------------------\n\r");
        do
        {
          d= vcp1->addr;
          uart_uint8out(d);
          puts(": ");

          ch= uart_readstr(&strsrc[0], 16);
          if (ch== 0xfe)                             // STP-Taste auf Platine gedrueckt
          {
            mch= 'q';                                // Beenden des Terminals vorbereiten
            ch= 0x1b;
          }
          if (ch== 0x0d)
          {
      //      printf("\n\r%s: ",&strsrc[0]);
            l= strlen(strsrc);
            if (!l) break;
            err= assemble_line(&strsrc[0], &prgword, vcp1);
            if ((err>0) && (err< 0xff))
            {
              switch (err)
              {
                case  2 : puts("\n\r datavalue too large, max. 255\n\r");
                case  5 : puts("\n\r unknown mnemonic\n\r"); break;
                case  6 : puts("   :  comment only\n\r"); break;
                default : puts("\n\r error code: "); uart_uint8out(err); break;
              }
            }
            else
            if (ch != 0x1b)
            {
              if (err != 0xff)                              // .org
              {
                if (vcp1->addr < memsize)
                {
                  for (i2= l; i2< 17; i2++) uart_putchar(' ');
                  uart_uint16out(prgword, 2);
                  puts("\n\r");

                  vcp1->mem[vcp1->addr]= ((prgword / 1000) << 8) + (prgword % 1000);
                  vcp1->addr++;
                }
                else
                {
                  puts("\n\r Memory adress to large\n\r");
                }
              }
              else
              {
                // neue Adresse wegen .org setzen
                if (vcp1->addr > memsize)
                {
                  puts("\n\r Memory adress to large\n\r");
                }
                else
                {
                  puts("\n\r");
                  vcp1->addr= prgword & 0xff;
                }
              }
            }
          }
        } while (ch != 27);           // bis ESC
        puts("\n\rassemble mode quits\n\n\r");
        break;
      }

      // Programm starten
      case 'r' :
      {
        puts("n\r program is running...\n\r");
//        vcp1->pc= 0;
        vcp1->sp= 7;
        err= 0;
        return 1;
      }

      // step: einen Programmschritt ausfuehren
      case 't' :
      {
        puts("\n\r        ");
        uart_uint8out(vcp1->pc);
        puts(":   ");

        disassemble_prgword(&strsrc[0], vcp1->mem[vcp1->pc]);
        puts_ram(&strsrc[0]);

        err= 0;
        cpu_run(vcp1, 1);
        registers_show(vcp1);
        do
        {
          i2= readshiftkeys();
          delay(20);
        }while (i2 != 0xff);
        delay(20);

        err= 0;

        break;
      }

      // Programm laden
      case 'l' :
      {
        puts("\n\r numbers of storage places: ");
        uart_uint8out((eep_memsize / 512) + 1);
        puts("\n\r enter program number to load: ");
        uart_readu8int(&prgnr);
        puts("\n\r");
        if (prgnr> ((eep_memsize / 512) + 1))
        {
          puts("\n\r there is no storage place with this number");
          puts("\n\r no program is loaded\n\n\r");
          break;
        }
        else
        {
          puts("\n\r loading program... ");
          if (prgnr < 2)
          {
            for (w= 0; w != (memsize-1); w++)
            {
              uint16_t hib, lob;

              hib= eeprom_read_byte((w * 2) + (prgnr*memsize*2));        // Hi-Byte lesen
              lob= eeprom_read_byte((w * 2) + 1 + (prgnr*memsize*2));    // Lo-Byte lesen

              vcp1->mem[w]= (hib << 8) | (lob & 0xff);
            }
          }
          else
          {
            ptr = (uint8_t *)&vcp1->mem[0];
            eep_readbuf((prgnr-2) * memsize, ptr, memsize * 2);
          }
          puts("done !\n\r");
        }

        break;
      }

      // Pogramm speichern
      case 's' :
      {
        puts("\n\r numbers of storage places: ");
        uart_uint8out((eep_memsize / 512) + 1);
        puts("\n\r enter program number to store: ");
        uart_readu8int(&prgnr);
        puts("\n\r");
        if (prgnr> ((eep_memsize / 512) + 1))
        {
          puts("\n\r there is no storage place with this number");
          puts("\n\r no program is stored\n\n\r");
          break;
        }
        else
        {
          puts("\n\r saving program... ");
          if (prgnr < 2)
          {
            for (w= 0; w != (memsize-1); w++)
            {
              eeprom_write_byte((w * 2) + (prgnr*memsize*2), vcp1->mem[w] >> 8);          // Hi-Byte schreiben
              eeprom_write_byte(((w * 2)+1) + (prgnr*memsize*2), vcp1->mem[w] & 0xff);
            }
          }
          else
          {
            ptr = (uint8_t *)&vcp1->mem[0];
            eep_writebuf((prgnr-2) * memsize, ptr, memsize * 2);
          }
          puts("done !\n\r");
        }
        break;
      }

      // clear, loescht den Programmspeicher und setzt
      // die CPU zurueck
      case 'c' :
      {
        puts("\n\r clearing program memory and reset cpu... ");
        puts("\n\n\r do you REALY want to do that ? [y/n]");
        ch= uart_getchar();
        if (ch== 'y')
        {
          cpu_reset(vcp1);
          puts(" ... memory cleared !\n\n\r");
        }
        else
        {
          puts("\n\r");
        }
        break;
      }

      // Show help
      case 'h' :
      {
        terminal_showhelp();
        break;
      }

      default: break;
    }
  } while(mch != 'q');

  puts("terminal quits\n\n\r");
  uart_deinit();

  // beim Beenden Teminal, PC auf 0 und anzeigen
  lastcmdchar= a_pc;
  showmask= 0x27;                               // nur 3-stellig einschalten
  vcp1->pc= 0;
  setdez(vcp1->pc,0);                           // P + PC anzeigen
  return 0;
}

void debugprint(uint16_t val)
{
  uart_init(baudrate);
  puts("\n\r Debug_Int Val.: ");
  uart_uint16out(val,0);
  puts("\n\r");
}

/*  ---------------------------------------------------------
                         softw_int_input

      liest einen max. 3-stelligen dezimalen Zahlenwert
      auf der Tastatur ein und gibt dieses als Funktions-
      ergebnis zurück.

      Ein SHIFT-INP repraesentiert hierbei (aufgrund der
      Doppelbelegung der Tasten) die Enter Taste

      Wird ein Zahlenwert > 255 eingegeben, wird die Eingabe
      solange wiederholt, bis ein gueltiger Zahlenwert
      eingegeben wurde

      Uebergabe
        inpanz : 2 Ziffern, die links angezeigt werden
                   0 => "IN"
                   1 => "AN"
                   2 => "EN"
                   3 => "NE"
        *endkey : die zuletzt eingegebene Funktionstaste
    --------------------------------------------------------- */
uint8_t softw_int_input(uint8_t inpanz, uint8_t *endkey)
{
  uint8_t  key, anz;
  uint16_t inp;

  showmask= 0;

  anz= 0;  inp= 0;

  while(1)
  {
    // bevor Zahl eingelesen werden kann, sollen alle Tasten
    // ungedrueckt sein
    while(readshiftkeys() != 0xff);
    delay(20);

    // vor jedem Tastendruck linksbuendig 2 Ziffern anzeigen anzeigen
    switch(inpanz)
    {
      case 0 :
      {
        setbmp(0,0x06);       // "I"
        setbmp(1,0x37);       // "N"
        break;
      }
      case 1 :
      {
        setbmp(0,0x77);       // "A"
        setbmp(1,0x37);       // "N"
        break;
      }
      case 2 :
      {
        setbmp(0,0x79);       // "E"
        setbmp(1,0x37);       // "N"
        break;
      }
      case 3 :
      {
        setbmp(0,0x37);       // "N"
        setbmp(1,0x79);       // "E"
        break;
      }
      default : break;
    }

    // wiederholen, bis eine Taste gedrueckt ist
    do
    {
      key= readshiftkeys();
      delay(20);
    }while (key== 0xff);

    // warten bis die Taste losgelassen wurde
    while(readshiftkeys() != 0xff) delay(20);

    if (key == 0x82) { *endkey= 0x82; return 0; }
    if (key == 0x88)             // es wurde SHIFT-INP gedrueckt = Enter
    {
      if (inp< 256)
      {
        *endkey= 0x88;
        return inp;
      }
      else
      {
        inp= 0; anz= 0; showmask= 0;
        // Fehleingabe anzeigen
        setbmp(3,0x79);       // "E"
        setbmp(4,0x50);       // "r"
        setbmp(5,0x50);       // "r"
        delay(1500);
        tm16_clear();
        tm16_setbright(seg7_bright);
      }
    }
    else
    if (key < 0x80)
    {
      // es war Zifferntaste
      if (anz != 3)
      {
        showmask= (showmask << 1) | 1;
        inp= (inp*10)+key;
        anz++;
      }
      setdez_nodp(inp);
    }
  }
}

/*  ---------------------------------------------------------
                          softw_intr

      wird vom Maschineninterpreter aufgerufen und der
      Zustand der Maschine wird in vcp1 uebergeben. In
      Data ist das extrahierte Datum des Opcodes enthalten.

      Hier koennen nach "belieben" komplette Funktionen
      eingehaengt werden.

      Fehlercodes fuer Softwareinterrupts sind 20 - 29
    ---------------------------------------------------------*/
uint8_t softw_intr(kcomp *vcp1, int data)
{
  int      i, cnt;
  uint8_t  wu2;
  uint8_t  lastkey;
  uint16_t tmp;
  int16_t temp;

  lastkey= 0;

  switch (data)
  {
    // #############################################################
    //                           INT 1
    //    Verzoegerungsschleife mit Zeiteinheit in Register B
    //    und Anzahl der verzoegerten Zeiteinheit im Akku
    // #############################################################
    case 1:
    {
      // Register B beinhaltet die Timebase des INT 1
      switch (vcp1->b)
      {
        case 1 : cnt= 1; break;       //   1ms
        case 2 : cnt= 10; break;      //  10ms
        case 3 : cnt= 100; break;     // 100ms
        case 4 : cnt= 1000; break;    //   1s
        break;
      }
      
      wu2= wait_shiftunpress;
      wait_shiftunpress= 0;
      for (i= 0; i< cnt; i++)
      {
        if (cp1_delay(vcp1->a) == 0x82) return;
      }

      wait_shiftunpress= wu2;
      break;
    }

    // #############################################################
    //                      int 2 / int 3
    // int 2 : PWM - Port1 / Frequenzgenerator
    // int 3 : PWM - Port 2 / Frequenzgenerator
    //         Frequenz = A, Timebase = RegB,
    //         Dutycycle in % = RegC
    //         Jede Dutycycle > 100 stopt den Generator

    // RegB   |  Timebase
    // -------+------------
    //   0    |  x   1  Hz
    //   1    |  x  10  Hz
    //   2    |  x 100  Hz
    //   3    |  x  1  KHz

    // Hinweis:
    //    fuer Timebase 3 sind Werte > 65 gueltig. Fuer Port 1 sind
    //    Frequenzen von 1 Hz bis 65 kHz einstellbar, fuer Port 2 sind
    //    gilt zudem die Beschraenkung, dass Frequenzen < 63 nicht
    //    einstellbar sind.
    //    Fehlercodes 20..29 sind die Fehlercodes fuer int 2 / int 3
    // #############################################################
    case 2:
    case 3:
    {
      uint32_t freq;

      if (vcp1->c > 100)
      {
        if (data== 2) { pwmt1_stop(); }
                 else { pwmt2_stop(); }
        return;
      }
      freq= 1;
      for (i= 0; i< vcp1->b; i++) { freq= freq * 10; }
      freq= freq * vcp1->a;
      if (freq> 65000)
      {
        err= 22;
        break;
      }
      // Fuer Port2 koennen keine Frequenzen < 63 eingestellt werden
      if ((freq< 63) && (data== 3))
      {
        err= 23;
        break;
      }

      if (data== 2) { pwmt1_setfreq(freq & 0xffff, vcp1->c); }
               else { pwmt2_setfreq(freq & 0xffff, vcp1->c); }

      break;
    }

    // #############################################################
    //                          int 4
    //                    Servo-Motor PWM
    // #############################################################
    case 4 :
    {
      // PWM-Frequenz = 50 Hz
      // Servo-Motor benoetigt einen Tastgrad von 3..13 % von einem
      // Anschlag zum anderen.
      // ToDo: Timing bisher nur fuer 8 MHz
      // Akku = 0 ==> linker Anschlag, Akku = 255 ==> rechter Anschlag
      tmp= vcp1->a;
      tmp= 550 + ((2100 / 255) * tmp);
      pwmt1_init(t01_div8, 20000, tmp);
      break;
    }

    // #############################################################
    //                          int 8
    //               Analog - Digital - Wandler
    //    als Referenz wird die Betriebsspannung verwendet (was
    //    somit abhaengig von der Spannung ist, die der USB-Port
    //    liefert
    // #############################################################
    case 8 :
    {
      // Register B beinhaltet die Unterfunktion des INT 8
      switch (vcp1->b)
      {
        //  ---------------------------------------------------------
        //                  INT 8 Funktion 0
        //    ADC-Wandler und Eingangskanal initialisieren, Eingangs-
        //    kanal im Akku: A == 0..3 => P2.0 .. P2.3
        //  ---------------------------------------------------------
        case 0 :
        {
          // Analogpin auf Eingang
          DDRC &= ~(1 << vcp1->a);
          // kein PopUp Widerstand
          PORTC &=~(1 << vcp1->a);

          if ((vcp1->a) > 3) { err= 40; return; }   // kein Eingangskanal > 3 vorhanden
          adc_init(adc_ref_avcc, vcp1->a);
          break;
        }
        
        //  ---------------------------------------------------------
        //                   INT 8 Funktion 1
        //    8-Bit Analogwert einlesen, A -> Analogwert
        //  ---------------------------------------------------------
        case 1 :
        {
          tmp= 0;

          // ADC 8 mal lesen und aufaddieren um Mittelwert zu bilden
          for (i= 0; i< 8; i++) { tmp += getadc_10bit(); delay(4);}

          // Mittelwert bilden und auf 8-Bit Aufloesung reduzieren
          tmp= tmp >> 5;
          vcp1->a= tmp;
          break;
        }
        
        // ---------------------------------------------------------
        //                     INT 8 Funktion 2
        //     8-Bit Analogwert einlesen, A -> Analogwert in %
        // ---------------------------------------------------------
        case 2 :
        {
          tmp= 0;

          // ADC 8 mal lesen und aufaddieren um Mittelwert zu bilden
          for (i= 0; i< 8; i++) { tmp += getadc_10bit(); delay(4);}

          // Mittelwert bilden 
          tmp= tmp >> 3;
          
          // und in Prozent umrechnen
          tmp= tmp / 10.24;
          vcp1->a= tmp;
          break;
        }

        // ---------------------------------------------------------
        //                     INT 8 Funktion 3
        //    8-Bit Analogwert einlesen, A -> Analogwert x 100 mV
        //    Bsp.:  23 => 2.3 V
        // ---------------------------------------------------------
        case 3 :
        {
          tmp= 0;

          // ADC 8 mal lesen und aufaddieren um Mittelwert zu bilden
          for (i= 0; i< 8; i++) { tmp += getadc_10bit(); delay(4);}

          // Mittelwert bilden
          tmp= tmp >> 3;
          
          // und auf Spannungswert umrechnen
          tmp= (tmp * own_ub100mv) / 1024;
          vcp1->a= tmp;
          break;
        }

        // ---------------------------------------------------------
        //                     INT 8 Funktion 10
        //    Spannungsabfall NTC-Widerstand messen und in Temperatur
        //    umrechnen. Rueckgabewert im Akku als Grad Celcius
        //    Less-Flag ist gesetzt, negative Temperaturen werden
        //    mit einem gesetzten Less-Flag gekennzeichnet
        // ---------------------------------------------------------
        case 10 :
        {
          tmp= 0;

          // ADC 8 mal lesen und aufaddieren um Mittelwert zu bilden
          for (i= 0; i< 8; i++) { tmp += getadc_10bit(); delay(4);}

          // Mittelwert bilden 
          tmp= tmp >> 3;
          temp= ntc_gettemp(tmp);
          vcp1->a= abs(temp / 10);
          if (temp < 0) { vcp1->psw |= 0x04; }  // Less-Flag bei neg. Temp. setzen
          break;
        }
        default : break;
      }
      break;
    }
    
    // #############################################################
    //                          int 9
    //    Tastaturfunktionen
    //    Fehlercodes 30..34 sind die Fehlercodes fuer int 9
    // #############################################################
    case 9 :
    {
      // Register B beinhaltet die Unterfunktion des INT 9
      switch (vcp1->b)
      {
        // ---------------------------------------------------------
        //                     INT 9 Funktion 0
        //   lese eine max. 3-stellige dezimale Zahl ein. Gelesene
        //   Zahl wird im Akku zurueck gegeben. Bestaetigen der Eingabe
        //   erfolgt mit SHIFT-INP als Return-Ersatz
        // ---------------------------------------------------------
        case 0 :
        {
          bufclr();
          showbuffer();
          vcp1->a= softw_int_input(0, &lastkey);
          // 2-Digits links loeschen
          setbmp(0,0x00);
          setbmp(1,0x00);
          break;
        }

        // ---------------------------------------------------------
        //                     INT 9 Funktion 1
        //   warte auf einen Zifferntastendruck und gebe den Wert im
        //   Akku zurueck
        // ---------------------------------------------------------
        case 1 :
        {
          do
          {
            lastkey= readshiftkeys();
            delay(20);
          }while (lastkey== 0xff);
          if (lastkey== 0x82) break;
          vcp1->a = lastkey;
          break;
        }

        // ---------------------------------------------------------
        //                     INT 9 Funktion 2
        //   feststellen, ob eine Taste gedrueckt ist, wartet aber nicht
        //   darauf. Bei gedrueckter Taste wird diese gelesen und das
        //   Zero-Flag gesetzt
        // ---------------------------------------------------------
        case 2:
        {
          lastkey= readshiftkeys();
          delay(20);
          if (lastkey< 0x80)                // Taste ohne Shift
          {
            vcp1->a = lastkey;
            vcp1->psw |= 0x01;
          }
          else
          {
            vcp1->a = 0xff;
            vcp1->psw &= ~(0x01);
          }
          break;
        }
        default : break;
      }
      break;
    }
    
    // #############################################################
    //                          int 10
    //    Displayfunktionen
    //    Fehlercodes 35..39 sind die Fehlercodes fuer int 10
    // #############################################################
    case 10 :
    {
      // Register B beinhaltet die Unterfunktion des INT 10
      switch (vcp1->b)
      {

        // ---------------------------------------------------------
        //                     INT 10 Funktion 0
        //   schaltet Displaydigits nach Zustand der Bitmaske im 
        //   Akku ein, fuer jedes eingeschaltete Digit erscheint 
        //   eine "0"
        // ---------------------------------------------------------
        case 0 :
        {
          if ((vcp1->a) > 63) {err= 35; return;}   // 63 = Code fuer alle Digits sichtbar
          showmask= vcp1->a;

          setdez_nodp(0);
          if (showmask & 0x20)
          {
            bmp2buf(bmp7s[0],0);                     // hoechstes Digit (pos0) auch mit "0" beschreiben
          }
          else
          {
            bmp2buf(0,0);
          }

          showbuffer();

          // Dezimalpunkte loeschen
          for (i= 0; i< 6; i++)
          {
            setdp(i, 0);
          }
          swint10_dp= 0;                            // Dezimalpunktmaske loeschen
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 1
        //   zeigt Akkuinhalt dezimal an 
        //   (wie Mnemonic cdis ohne Dezimalpunkt)
        // ---------------------------------------------------------
        case 1 :
        {
          setdez_nodp(vcp1->a);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }

          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 2
        //     zeige den 16-Bit Wert in C-D dezimal an
        // ---------------------------------------------------------
        case 2 :
        {
          tmp= vcp1->c;
          tmp= (tmp << 8) | vcp1->d;
          setdez_nodp(tmp);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 3
        //   zeigt Akkuinhalt hexadezimal an
        // ---------------------------------------------------------
        case 3 :
        {
          sethex(vcp1->a);
          bmp2buf(cmdchar[a_h], 0);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 4
        //    zeige den 16-Bit Wert in C-D hexadezimal an
        // ---------------------------------------------------------
        case 4 :
        {
          tmp= vcp1->c;
          tmp= (tmp << 8) | vcp1->d;

          sethex(tmp);
          bmp2buf(cmdchar[a_h], 0);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 5
        //    zeigt Akkuinhalt binaer an
        // ---------------------------------------------------------
        case 5 :
        {
          if ((vcp1->a) > 63) {err= 36; return;}   // mehr als 63 ist binaer nicht darstellbar

          setbin(vcp1->a);
          if ((vcp1->a) < 32) bmp2buf(bmp7s[0x0b], 0);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 6
        //    zeige ein 7-Segmentbitmap auf einer Postiion an.
        //    Akku enthaelt das Bitmap das angezeigt werden soll, 
        //    Register C die Position, an der das Bitmap ausgegeben
        //    werden soll (pos. 0 = links)
        // ---------------------------------------------------------
        case 6 :
        {
          setbmp(vcp1->c,vcp1->a);
          break;
        }
        
        // ---------------------------------------------------------
        //                     INT 10 Funktion 7
        //   zeigt Akkuinhalt dezimal an 
        //   (wie Funktion 1 mit zusaetzlicher Ausgabeposition)
        // ---------------------------------------------------------
        case 7 :
        {
          if ((vcp1->c) > 3) {err= 37; return;}   // nur 4 Ausgabepositionen moeglich
          setdez_u8(vcp1->a, vcp1->c);

          // Dezimalpunkt setzen
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }

          break;
        }        

        // ---------------------------------------------------------
        //                     INT 10 Funktion 20
        //    Bitmaske fuer Dezimalpunktanzeige
        // ---------------------------------------------------------
        case 20 :
        {
          if ((vcp1->a) > 63) {err= 35; return;}   // 63 = Code fuer alle Punkte sichtbar
          swint10_dp= vcp1->a;
          for (i= 0; i< 6; i++)
          {
            setdp(i, swint10_dp & (1 << i));
          }
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 21
        //    einzelnen Dezimalpunkt setzen
        // ---------------------------------------------------------
        case 21 :
        {
          if ((vcp1->a) > 5) {err= 37; return;}   // mehr als Punkt 5 ist nicht anzeigbar
          setdp(vcp1->a, 1);
          swint10_dp |= 1 << (vcp1->a);
          break;
        }

        // ---------------------------------------------------------
        //                     INT 10 Funktion 22
        //    einzelnen Dezimalpunkt loeschen
        // ---------------------------------------------------------
        case 22 :
        {
          if ((vcp1->a) > 5) {err= 37; return;}   // mehr als Punkt 5 ist nicht loeschbar
          setdp(vcp1->a, 0);
          swint10_dp &= ~(1 << (vcp1->a));
          break;
        }
        default : break;
      }
      break;
    }

    default : break;
  }
  return lastkey;
}  // softw_intr

void program_download(kcomp *vcp1)
{
  uint8_t hby, lby, ch;
  int     i;
  
  tm16_bufclr();
  
  // Anzeige "LOAd"
  bmp2buf(0x38, 1);  
  bmp2buf(0x3f, 2);  
  bmp2buf(0x77, 3);  
  bmp2buf(0x5e, 4);  
  showbuffer();
  
  uart_init(baudrate);
  do
  {
    ch= 0;
    if (uart_ischar())
    {
      ch= uart_getchar();
    }  
    if (readshiftkeys() == 0x82) ch= 'E';    
  } while ((ch != 'U') && (ch != 'E'));
  if (ch == 'U')
  {
    delay(200);
    uart_putchar('u');
    showmask= 0x27;
    lastcmdchar= a_load;
    setdp(3,1);
    for (i= 0; i< 256; i++)
    {
      lby= uart_getchar();
      hby= uart_getchar();
      vcp1->mem[i]= (uint16_t)(hby << 8) | lby;
      setdez(i,0);
      uart_putchar('o');
    }
    load_showok();
    delay(1000);
    vcp1->sp= 7;
    vcp1->pc= 0;
  }
  showmask= 0x27;
  lastcmdchar= a_pc;
  setdp(3,1);
  setdez(vcp1->pc,0);    
  uart_deinit();  
}

/*  -----------------------------------------------------------------------------
                                      MAIN
    ----------------------------------------------------------------------------- */
int main(void)
{
  int8_t    i;
  uint8_t   value;
  uint8_t   exitcode, helpanz, reganz;
  uint8_t   kanz, fkey, opc;
  uint16_t  data;
  uint16_t  w;
  uint8_t   outset= 0;
  uint8_t   ack;
  uint8_t   *ptr;
  uint8_t   lastkey;

  uint8_t   startadr, stopadr, movadr;

  i2c_master_init();

  p1_config(0xff);
  p1_bytewrite(0);

  p2_config(0xff);
  p2_bytewrite(0);

  timer0_init();
  tm16_init();
  tm16_setbright(seg7_bright);

  showmask= 0x27;
  lastcmdchar= a_pc;
  setdp(3,1);
  setdez(0,0);
  cpu_reset(&cp1);

  while(1)
 {

    err= 0;
    keyb_input(&opc, &data, &kanz, &fkey);

    if (kanz== 6) err= 1;

    switch (fkey)
    {
      // run
      case 0x81 :
      {
        // Festprogramm: Empfang Binaerprogramm
        if ((kanz== 1) && (data== 5))
        {
          program_download(&cp1);
          break;
        }
        
        // Festprogramm: Selbsttest
        if ((kanz== 1) && (data== 9))
        {
          selftest();
          break;
        }

        // Festprogramm: Reaktionsspiel
        if ((kanz== 1) && (data== 8))
        {
          reaktionstest();
          while(readshiftkeys()== 0xff);
          delay(10);
          break;
        }

        // Festprogramm: Terminal-Mode
        if ((kanz== 1) && (data== 7))
        {
          helpanz= 1;
          reganz= 0;
          do
          {
            exitcode= terminal_mode(&cp1, helpanz, reganz);
//            while(readshiftkeys()== 0xff);
            delay(100);
            helpanz= 0;
            switch (exitcode)
            {
              // Terminal mit "quit" verlassen
              case 0  : break;

              // Terminal mit "run" verlassen
              case 1  :
              {
                reganz= 1;                                   // nach Beenden des Programms Register anzeigen
                showmask= 0x00;
                setdez(0,0);
                showmask= 0x07;                              // waehrend Programmlauf nur
                                                             // Zahlen anzeigen
                cpu_run(&cp1, 0);
                delay(80);

                if (err== 255)                               // wurde Programm mit STP angehalten ?
                {
                  err= 0;                                    // dann Taste loeschen und letzen
                  showmask= 0x1f;
                  setdez(cp1.a,0);
                  fkey= 0;
                }

                delay(200);
                break;
              }
              default : break;
            }
          } while (exitcode);
          break;
        }  // Terminal - Mode

        // Festprogramm: Speicherinhalt verschieben
        if ((kanz== 1) && (data== 6))
        {
          bufclr();
          showbuffer();
          setdp(3,1);
          startadr= softw_int_input(1, &lastkey);
          if (lastkey== 0x82)
          {
            showmask= 0x27;
            lastcmdchar= a_pc;
            setdp(3,1);
            setdez(cp1.pc,0);
            break;
          }

          bufclr();
          showbuffer();
          setdp(3,1);
          stopadr= softw_int_input(2, &lastkey);
          if (lastkey== 0x82)
          {
            showmask= 0x27;
            lastcmdchar= a_pc;
            setdp(3,1);
            setdez(cp1.pc,0);
            break;
          }

          bufclr();
          showbuffer();
          setdp(3,1);
          movadr= softw_int_input(3, &lastkey);
          if (lastkey== 0x82)
          {
            showmask= 0x27;
            lastcmdchar= a_pc;
            setdp(3,1);
            setdez(cp1.pc,0);
            break;
          }

          prg_moveto(&cp1, movadr, startadr, stopadr);
          bufclr();
          bmp2buf(0x5c,0);
          bmp2buf(0x70,1);
          bmp2buf(0,2);
          bmp2buf(0,3);
          bmp2buf(0,4);
          bmp2buf(0,5);
          setdp(3,0);

          delay(10);
          break;
        }  // Speicherinhalt verschieben

        showmask= 0x00;
        setdez(0,0);
        showmask= 0x07;                              // waehrend Programmlauf nur
                                                     // Zahlen anzeigen
        cpu_run(&cp1, 0);
        delay(80);

        if (err== 255)                               // wurde Programm mit STP angehalten ?
        {
          err= 0;                                    // dann Taste loeschen und letzen
          showmask= 0x1f;
          setdez(cp1.a,0);
          fkey= 0;
          break;                                     // Displayinhalt stehen lassen
        }
        err = 0;

        // PC nach Anhalten des Programms anzeigen
        lastcmdchar= a_pc;                            // P fuer PC
        showmask= 0x27;                               // nur 3-stellig einschalten
        setdez(cp1.pc,0);                             // P + PC anzeigen

        break;
      }

      // step
      case 0x83 :
      {
        cpu_run(&cp1, 1);

        lastcmdchar= a_pc;                            // P fuer PC
        showmask= 0x27;                               // nur 3-stellig einschalten
        setdez(cp1.pc,0);                             // P + PC anzeigen

        break;
      }
      // Akku anzeigen
      case 0x84 :
      {
        lastcmdchar= a_akku;                          // A fuer Akku
        showmask= 0x3f;                               // alle Anzeigen einschalten
        setdez(cp1.a,0);                              // A + Akku anzeigen
        break;
      }

      // clr
      case 0x85 :
      {
        anz_clr();
        break;
      }

      // set PC
      case 0x86 :
      {
        if (kanz == 0)
        {
          lastcmdchar= a_pc;                         // P fuer PC
          showmask= 0x27;                            // nur 3-stellig einchalten
          setdez(cp1.pc,0);                          // P + PC anzeigen
          break;
        }
        if (kanz != 3) err= 1;
        if (data> memsize-1) err= 3;
        cp1.pc= data;
        lastcmdchar= a_pc;                           // P fuer PC
        showmask= 0x27;                              // nur 3-stellig einschalten
        setdez(cp1.pc,0);                            // P + PC anzeigen

        // wird der Programmcounter neu gesetzt, werden auch alle
        // Register geloescht
        cp1.a= 0;
        cp1.b= 0;
        cp1.c= 0;
        cp1.d= 0;
        cp1.psw= 0;
        cp1.sp= 7;

        break;

      }

      // out : Anzeige des Speicherinhalts; bei keiner Datumseingabe
      //       zusaetzlich Adresse inkrementieren
      case 0x87 :
      {
        // aktuellen Adresszeiger anzeigen
        if ((kanz== 1) && (data== 9))
        {
          lastcmdchar= a_cell;                       // C fuer Cell
          showmask= 0x27;                            // nur 3-stellig einschalten
          setdez(cp1.addr+outset,0);                 // die Eingabe + "C" anzeigen
          break;
        }
        // ansonsten "normale" Ausgabe an aktuelle Adresse
        else
        {
          if ((kanz!= 3) && (kanz != 0)) err= 1;    // nur Adresseingabe ohne Opcode
                                                    // oder gar keine Adresseingabe erlaubt
          if (data> 255) err= 4;
          if (!err)
          {
            lastcmdchar= a_cell;
            showmask= 0x3f;
            if (kanz== 0)
            {
              cp1.addr++;
              setdez(mem2opc(cp1.mem[cp1.addr]),0);
              outset= 1;
            }
            else
            {
              setdez(mem2opc(cp1.mem[data]),0);
              if (data> memsize-1)
              {
                err= 3;
              }
              else
              {
                cp1.addr= data;
                outset= 0;
              }
            }
          }
        }
        break;
      }

      // inp : Eingabe von Opcode und Datum
      case 0x88 :
      {
        if ((kanz != 3) && (kanz != 5)) err= 1;
        if (data> 255) err= 4;
        if (opc> opcmax) err= 4;
        if (!err)
        {
          cp1.mem[cp1.addr]= (opc << 8) | data;     // Adresse mit Opcode und Datum beschreiben

          lastcmdchar= a_inp;                       // E fuer Input okay
          showmask= 0x3f;                           // alles anzeigen
          if (cp1.addr> memsize-1)
          {
            err= 3;
          }
          else
          {
            setdez(mem2opc(cp1.mem[cp1.addr]),0);     // die Eingabe + "E" anzeigen
            cp1.addr++;
          }
        }
        break;
      }

      // CAS: "Cassette Store"
      // ==> Aenderung zum Original. Im Datenfeld wird die Angabe des Speicherplatzes
      //     erwartet. Speicherplaetze 0 und 1 sind das interne EEPROM des ATmega328
      //     ( insgesamt 1 kByte). Alle anderen Speichernummern beschreiben das
      //     externe EEProm. Die hoechstmoegliche Angabe in Data richtet sich somit
      //     nach der Groesse des EEPROMS. Bsp.: Speichergroesse EEPROM = 8192 sind
      //     16 x 512 Byte. Somit sind insgesamt 18 Speicherplaetze vorhanden und die
      //     hoechste Angabe fuer Data ist 17.
      case 0x89 :
      {
        if (kanz != 3) { err= 1; break; }
        if (data> (eep_memsize / 512) + 1) { err= 7; break; }
        if (opc> 0) { err= 7; break; }

        store_showrunning();

        if (data < 2)
        {
          for (w= 0; w< memsize; w++)
          {
            eeprom_write_byte((w * 2) + (data*memsize*2), cp1.mem[w] >> 8);        // Hi-Byte schreiben
            eeprom_write_byte(((w * 2)+1) + (data*memsize*2), cp1.mem[w] & 0xff);
          }
        }
        else
        {
          ptr = (uint8_t *)&cp1.mem[0];
          eep_writebuf((data-2) * memsize, ptr, memsize * 2);
        }

        store_showok();
        delay(1000);
        lastcmdchar= a_pc;                           // P fuer PC
        showmask= 0x27;                              // nur 3-stellig einschalten
        setdez(cp1.pc,0);                            // P + PC anzeigen
        break;
      }

      // CAL: "Cassette Load"
      // ==> Aenderung zum Original. Im Datenfeld werden fuer die Speicher-
      //     position im internen oder externen EEProm erwartet
      case 0x80 :
      {
        if (kanz != 3) { err= 1; break; }
        if (data> (eep_memsize / 512) + 1) { err= 7; break; }
        if (opc> 0) { err= 7; break; }

        load_showrunning();
        delay(500);                  // einfach nur um die Anzeige kurz zu sehen

        if (data < 2)
        {
          for (w= 0; w< memsize; w++)
          {
            uint16_t hib, lob;

            hib= eeprom_read_byte((w * 2) + (data*memsize*2));        // Hi-Byte lesen
            lob= eeprom_read_byte((w * 2) + 1 + (data*memsize*2));    // Lo-Byte lesen

            cp1.mem[w]= (hib << 8) | (lob & 0xff);
          }
        }
        else
        {
          ptr = (uint8_t *)&cp1.mem[0];
          eep_readbuf((data-2) * memsize, ptr, memsize * 2);
        }

        cp1.pc= 0;
        showmask= 0x27;
        lastcmdchar= a_pc;
        setdez(cp1.pc,0);
        break;
      }
      default: break;
    }

    if (err)
    {
      lastcmdchar= a_err;
      showmask= 0x27;
      setdez(err,0);
    }
  }
}
