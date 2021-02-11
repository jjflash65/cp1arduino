/* ---------------------------------------------------------------------------
                                cp1_tm1637.cpp
                 
     Grundfunktionen des TM1637 7-Segment- und Keyboardcontrollers auf 
     dem CP1+ Board
     
     27.01.2021    R. Seelig                      
   --------------------------------------------------------------------------- */
   
#include "cp1_tm1637.h"

 // Bitmuster der einzelnen Ziffern von 0..F, 16= aus
const uint8_t bmp7s[17] PROGMEM  =                   
                { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07,
                  0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00 };

const uint8_t bmpasciis[] PROGMEM =
  //               ,     -     .     /     0     1     2     3     4
                { 0x80, 0x40, 0x80, 0x00, 0x3f, 0x06, 0x5b, 0x4f, 0x66,
                  
  //               5     6     7     8     9     :     ;     <     =
                  0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x00, 0x00, 0x48,
                  
  //                >    ?     @ (Bitmap fuer hochgestelltes o)
                  0x00, 0x00, 0x63,
                  
  //                A    B     C     D     E     F     G     H     I 
                  0x77, 0x7F, 0x39, 0x3f, 0x79, 0x71, 0x3d, 0x76, 0x06,
                  
  //                J    K     L     M     N     O     P     Q     R                    
                  0x1e, 0x76, 0x38, 0x36, 0x37, 0x3f, 0x73, 0x67, 0x33,
                  
  //                S    T     U     V     W     X     Y     Z 
                  0x6D, 0x31, 0x3e, 0x3e, 0x36, 0x76, 0x6e, 0x5b,
                                  
  //               [     \     ]     ^     _     '
                  0x00, 0x00, 0x00, 0x23, 0x08, 0x02,
                  
  //               a     b     c     d     e     f     g     h     i    
                  0x5f, 0x7c, 0x58, 0x5e, 0x7b, 0x70, 0x6f, 0x74, 0x04,
                  
  //               j     k     l     m     n     o     p     q     r
                  0x1e, 0x70, 0x06, 0x14, 0x54, 0x5c, 0x73, 0x67, 0x50,                  
                  
  //               s     t     u     v     w     x     y     z
                  0x6d, 0x70, 0x1c, 0x1c, 0x14, 0x76, 0x66, 0x5b };                  


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
   
tm1637::tm1637(uint8_t scl, uint8_t sda, uint8_t shift)
{
  tm16_scl= scl;
  tm16_sda= sda;
  shift_key= shift;
  
  shift_init();
  scl_init();
  sda_init();
}

void tm1637::start()              // I2C Bus-Start
{
  bb_scl_hi();
  bb_sda_hi();
  puls_len();
  bb_sda_lo();
}

void tm1637::stop()               // I2C Bus-Stop
{
  bb_scl_lo();
  puls_len();
  bb_sda_lo();
  puls_len();
  bb_scl_hi();
  puls_len();
  bb_sda_hi();
}

void tm1637::write (uint8_t value)    // I2C Bus-Datentransfer
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
                    tm1637::read(uint8_t ack)

   liest ein Byte vom I2c Bus.

   Uebergabe:
               1 : nach dem Lesen wird dem Slave ein
                   Acknowledge gesendet
               0 : es wird kein Acknowledge gesendet

   Rueckgabe:
               gelesenes Byte
   ------------------------------------------------------- */
uint8_t tm1637::read(uint8_t ack)
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

/*  ---------------------------------------------------------
                          tm1637::clear

       loescht die Anzeige auf dem Modul
    --------------------------------------------------------- */
void tm1637::clear()
{
  uint8_t i;

  selectpos(0);
  for(i=0; i<6; i++) { write(0x00); }
  stop();

  setbright(hellig);
}

 /*  ---------------------------------------------------------
                           tm1637::selectpos

        waehlt die zu beschreibende Anzeigeposition aus
     --------------------------------------------------------- */
void tm1637::selectpos(char nr)
{
  start();
  write(0x40);                // Auswahl LED-Register
  stop();

  start();
  write(0xc0 | nr);           // Auswahl der 7-Segmentanzeige
}

/*  ----------------------------------------------------------
                           tm1637::setbright

       setzt die Helligkeit der Anzeige
       erlaubte Werte fuer Value sind 0 .. 15
    ---------------------------------------------------------- */
void tm1637::setbright(uint8_t value)
{
  start();
  hellig= value;
  write(0x88 | value);        // unteres Nibble beinhaltet Helligkeitswert
  stop();
}

/*  ---------------------------------------------------------
                            tm1637::setbmp

       gibt ein Bitmapmuster an einer Position aus
    --------------------------------------------------------- */
void tm1637::setbmp(uint8_t pos, uint8_t value)
{
  selectpos(pos);             // zu beschreibende Anzeige waehlen

  write(value);               // Bitmuster value auf 7-Segmentanzeige ausgeben
  stop();
}


/*  ---------------------------------------------------------
                            tm1637::setzif

       gibt eine Ziffer an einer Position aus
    --------------------------------------------------------- */
void tm1637::setzif(uint8_t pos, uint8_t value)
{
  setbmp(pos, pgm_read_byte(bmp7s + value));
}

/*  ---------------------------------------------------------
                            tm1637::setascii

       gibt ein Bitmapmuster an einer Position aus
    --------------------------------------------------------- */
void tm1637::setascii(uint8_t pos, uint8_t ch)
{
  if (ch== ' ') ch= ':';       // ASCII Leerzeichen ist im Bitmap des Doppelpunktes 
  ch -= ',';                   // Bitmaps erst ab Zeichen ',' verfuegbar
  setbmp(pos, pgm_read_byte(bmpasciis + ch));
}


/*  ---------------------------------------------------------
                            tm1637::setzif_dp

       gibt eine Ziffer an einer Position aus
    --------------------------------------------------------- */
void tm1637::setzif_dp(uint8_t pos, uint8_t value)
{
  setbmp(pos, pgm_read_byte(bmp7s + value) | 0x80);
}

/*  ---------------------------------------------------------
                        tm1637::setdez

       gibt einen maximal 6-stelligen dezimalen Wert auf der
       Anzeige aus

       Uebergabe:

         value   : auszugebender Wert
         komma   : Position, an der ein Dezimalpunkt ausge-
                   geben wird
         leading : 0 = keine fuehrende Nullen
                   1 = Ausgabe fuehrender Nullen                 
    --------------------------------------------------------- */
void tm1637::setdez(int32_t value, uint8_t komma, uint8_t leading)
{
  typedef enum boolean { FALSE, TRUE }bool_t;

  static uint32_t zz[]  = { 100000, 10000, 1000, 100, 10 };
  bool_t not_first = FALSE;

  uint8_t zi;
  uint32_t z, b;
  bool_t kommaflag = FALSE;
  bool_t minusflag = FALSE;
  
  clear();
  if (leading) not_first= TRUE;
  komma= 6-komma;

  if (!value)
  {
    setzif(5, 0);
  }
  else
  {
    if(value < 0)
    {
      minusflag= TRUE;
      value = -value;
    }

    for(zi = 0; zi < 5; zi++)
    {
      z = 0;
      b = 0;

      if ((zi+1) == komma) kommaflag= TRUE; else kommaflag= FALSE;

      while(z + zz[zi] <= value)
      {
        b++;
        z += zz[zi];
      }

      if(b || not_first || kommaflag)
      {
        if (kommaflag) setzif_dp(zi, b); else setzif(zi, b);
        not_first = TRUE;
      }

      value -= z;
    }
    setzif(zi, value);    
  }
  if (minusflag)  
  {
    if (komma == 1) setbmp(0, 0xc0); else setbmp(0, 0x40);
  }
}

/*  ---------------------------------------------------------
                        tm1637::sethex
                        
       gibt einen maximal 6-stelligen hexadezimalen Wert 
       auf der Anzeige aus

       Uebergabe:

         value   : auszugebender Wert
         digits  : Anzahl auszugebender Digits
    --------------------------------------------------------- */
void tm1637::sethex(uint32_t value, uint8_t digits)
{
  uint8_t i,v;

  for (i= 6; i> digits; i--)
  {
    setbmp(i-1, pgm_read_byte(bmp7s + 16));
  }

  for (i= digits; i> 0; i--)
  {
    v= value % 0x10;
    setbmp(i+5-digits, pgm_read_byte(bmp7s + v));
    value= value / 0x10;
  }
}

/*  ---------------------------------------------------------
                       tm1637::setseg

       setzt ein einzelnes Segment einer Anzeige

       Uebergabe:
       
         pos: Digit (0..5)
         seg: das einzelne Segment
    --------------------------------------------------------- */
void tm1637::setseg(uint8_t digit, uint8_t seg)
{

  segbuf |= 1 << seg;
  setbmp(digit, segbuf);
}

/*  ---------------------------------------------------------
                       tm1637::clrseg

       loescht ein einzelnes Segment einer Anzeige

       Uebergabe:
       
         pos: Digit (0..5)
         seg: das einzelne Segment
    --------------------------------------------------------- */
void tm1637::clrseg(uint8_t digit, uint8_t seg)
{

  segbuf &=  ~(1 << seg);
  setbmp(digit, segbuf);
}

/*  ---------------------------------------------------------
                         tm1637::readkey

      liest angeschlossene Tasten ein und gibt dieses als
      Argument zurueck.

      Anmerkung:
        Es wird keine Tastenmatrix zurueck geliefert. Ist
        mehr als eine Taste aktiviert, wird nur die hoechste
        Taste zurueck geliefert. Somit ist es nicht moeglich
        mehrere Tasten gleichzeitig zu betaetigen.
    --------------------------------------------------------- */
uint8_t tm1637::readkey(void)
{
  uint8_t key;

  key= 0;
  start();
  write(0x42);
  key= ~read(1);
  stop();
  if (key) key -= 8; else key= 0xff;
  return key;
}

/*  ---------------------------------------------------------
                      tm1637::readhiftkeys

      liest angeschlossene Tasten ein und ermittelt eine
      evtl. zusaetzliche gedrueckte Shift-Taste.

      Bei gedrueckter Shift-Taste wird dem Tastenwert
      0x80 als Kennung dafuer, dass Shift-Taste gedrueckt
      hinzuaddiert.

      Bsp. Taste 5 ohne Shift => 0x05
                   mit Shift => 0x85

      Uebergabe:
          wait_unpress : wartet bis Taste(n) losgelassen
                         wurde
                         
          wait_shiftunpress : wartet bis erst Zifferntaste
                              und dann Shifttaste losge-
                              lassen wurde                          
    --------------------------------------------------------- */
uint8_t tm1637::readshiftkeys(uint8_t wait_unpress, uint8_t wait_shiftunpress)
{
  uint8_t shflag;
  uint8_t key;

  shflag= 0;
  if (is_shift()) shflag= 1;
  key= readkey();
  if (key== 0xff) return 0xff;          // es wurde keine Taste gedrueckt

  // warten bis Taste und Shift-Taste losgelassen sind

  if (wait_unpress)
    while ((is_shift()) || (readkey() != 0xff));
  if (wait_shiftunpress)
    while (is_shift());

  if (shflag) return (0x80 | key); else return key;
}

/*  ---------------------------------------------------------
                        tm1637::input

      liest einen max. 5-stelligen dezimalen Zahlenwert
      auf der Tastatur ein und gibt dieses als Funktions-
      ergebnis zurück.

      Ein SHIFT-INP repraesentiert hierbei (aufgrund der
      Doppelbelegung der Tasten) die Enter Taste

      Wird versucht, mehr als 5 Ziffern einzugeben, wird
      die Eingabe solange wiederholt, bis ein gueltiger 
      Zahlenwert eingegeben wurde.

      Uebergabe

        bmp     : Bitmap, das auf dem linken Digit
                  waehrend der Eingabe angezeigt wird
        *endkey : die zuletzt eingegebene Funktionstaste
    --------------------------------------------------------- */
uint32_t tm1637::input(uint8_t bmp, uint8_t *endkey)
{
  uint8_t  key, anz;
  uint32_t inp;

  anz= 0;  inp= 0;
  clear();
  setbright(hellig);  
  setdez(inp, 0, 0);
      
  while(1)
  {
    // bevor Zahl eingelesen werden kann, sollen alle Tasten
    // ungedrueckt sein
    while(readshiftkeys(1,1) != 0xff);
    delay(20);

    setbmp(0, bmp);      
    // wiederholen, bis eine Taste gedrueckt ist
    do
    {
      key= readshiftkeys(1,1);
      delay(20);
    }while (key== 0xff);

    // warten bis die Taste losgelassen wurde
    while(readshiftkeys(1,1) != 0xff) delay(20);

    if (key == 0x82) { *endkey= 0x82; return 0; }
    if (key == 0x88)             // es wurde SHIFT-INP gedrueckt = Enter
    {
      return inp;                
    }
    else
    {
      // es war Zifferntaste
      if (anz != 5)
      {
        inp= (inp*10)+key;
        anz++;
      }
      else
      {
        inp= 0; anz= 0;
        // Fehleingabe anzeigen
        clear();
        setbmp(3,0x79);       // "E"
        setbmp(4,0x50);       // "r"
        setbmp(5,0x50);       // "r"
        delay(1500);
        clear();
        setbright(hellig);
      }
      setdez(inp, 0, 0);
    }
  }
}

/*  ---------------------------------------------------------
                       tm1637::puts

       gibt einen im RAM gespeicherten String auf dem
       Display aus

       Uebergabe:
       
         pos: Digit, ab der ausgegeben wird
         *s : auszugebender String
    --------------------------------------------------------- */
void tm1637::puts(uint8_t pos, char *s)
{  
  while(*s)
  {
    setascii(pos, *s);   
    pos++; 
    s++;
  }
}

/*  ---------------------------------------------------------
                      tm1637::puts_rom

       gibt einen im ROM gespeicherten String auf dem
       Display aus

       Uebergabe:
       
         pos: Digit, ab der ausgegeben wird
         *s : auszugebender String
         
       Benutzung:
           tm16.puts_rom(1, PSTR("HALLO")
    --------------------------------------------------------- */
void tm1637::puts_rom(uint8_t pos, const PROGMEM uint8_t *s)
{
  unsigned char c;

  for (c=pgm_read_byte(s); c; ++s, c=pgm_read_byte(s))
  {
    setascii(pos, c);
    pos++;
  }
}
