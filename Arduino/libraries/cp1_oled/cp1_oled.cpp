/* -----------------------------------------------------
                         cp1_oled.h

    Anbindung eines OLED Displays mit SSD1306 
    Controller und SPI Interface an das CP1+ Board

     Board : CP1+
     F_CPU : 8 MHz intern

     28.01.2021        R. Seelig

  ------------------------------------------------------ */
/*

    Pinbelegung:

       CP1+     ATmega328    Arduino    Display
    ----------------------------------------------------

      Port 1

       GND                                GND        (1)
       +5V                                Vcc        (2)
       P1_0  ---   PD6   ---   D6   ---   D0 (CLK)   (3)
       P1_1  ---   PD7   ---   D7   ---   D1 (MOSI)  (4)
       P1_2  ---   PB0   ---   D8   ---   RST        (5)
       P1_3  ---   PB1   ---   D9   ---   DC         (6)
       P1_4  ---   PB2   ---   D10  ---   CE         (7)

      Port 2

       GND                                GND        (1)
       +5V                                Vcc        (2)
       P2_0  ---   PC0   ---   A0   ---   D0 (CLK)   (3)
       P2_1  ---   PC1   ---   A1   ---   D1 (MOSI)  (4)
       P2_2  ---   PC2   ---   A2   ---   RST        (5)
       P2_3  ---   PC3   ---   A3   ---   DC         (6)
       P2_4  ---   PD4   ---    4   ---   CE         (7)


           G   V           R
           N   c   D   D   E   D   C
           D   c   0   1   S   C   s
       +-------------------------------+
       |   o   o   o   o   o   o   o   |
       |                               |
       |   -------------------------   |
       |  |                         |  |
       |  |                         |  |
       |  |                         |  |
       |  |                         |  |
       |  |                         |  |
       |  |                         |  |
       |   -----+-------------+-----   |
       |        |             |        |
       |        +-------------+        |
       +-------------------------------+
       
*/       

#include "cp1_oled.h"

extern const uint8_t PROGMEM font8x8[][8];
extern const uint8_t PROGMEM font5x7[][5];

/* -------------------------------------------------------------
                          oled::oled
                          Konstruktor
                          
     initialisiert die Bitbanging-SPI-Schnittstelle und 
     konfiguriert sowohl das Display und den Framebuffer                          
   ------------------------------------------------------------- */
oled::oled(uint8_t port)
{
  oled_port= port;
  
  spi_init();

  if (oled_port== 2)
  {  
    p2_oled_enable();
    _delay_ms(10);
  
    p2_rst_clr();                 // Display-reset
    _delay_ms(10);
    p2_rst_set();
  
    p2_oled_cmdmode();            // nachfolgende Daten als Kommando
  
    spi_out(0x8d);                // Ladungspumpe an
    spi_out(0x10);
  
    spi_out(0xaf);                // Display on
    _delay_ms(150);
  
    spi_out(0xa1);                // Segment Map
    spi_out(0xc0);                // Direction Map
    p2_oled_datamode();
  }
  else
  {
    p1_oled_enable();
    _delay_ms(10);
  
    p1_rst_clr();                 // Display-reset
    _delay_ms(10);
    p1_rst_set();
  
    p1_oled_cmdmode();            // nachfolgende Daten als Kommando
  
    spi_out(0x8d);                // Ladungspumpe an
    spi_out(0x14);
  
    spi_out(0xaf);                // Display on
    _delay_ms(150);
  
    spi_out(0xa1);                // Segment Map
    spi_out(0xc0);                // Direction Map
    p1_oled_datamode();    
  }
  
  fb_init(128, 8);
  fb_clear();
  clrscr();
  gotoxy(0,0);
}
/* -------------------------------------------------------------
                          oled::spi_init

     Anschlusspins des SPI-Interface konfigurieren
   ------------------------------------------------------------- */
void oled::spi_init(void)
{
  if (oled_port== 2)
  {
    p2_mosiinit();
    p2_csinit();
    p2_resinit();
    p2_dcinit();
    p2_sckinit();
  }  
  else
  {
    p1_mosiinit();
    p1_csinit();
    p1_resinit();
    p1_dcinit();
    p1_sckinit();
  }
}

/* -----------------------------------------------------
                      oled::spi_out

        Byte ueber Software SPI senden / empfangen
        data ==> zu sendendes Datum
   ----------------------------------------------------- */
void oled::spi_out(uint8_t value)
{
  char a;

  for (a= 0; a< 8; a++)
  {
    
    if (oled_port== 2)
    {
      if((value & 0x80)> 0) { p2_mosi_set(); }
                       else { p2_mosi_clr(); }
      // Taktimpuls erzeugen
      p2_sck_set();
      p2_sck_clr();
    }
    else
    {
      if((value & 0x80)> 0) { p1_mosi_set(); }
                       else { p1_mosi_clr(); }
      // Taktimpuls erzeugen
      p1_sck_set();
      p1_sck_clr();
    }
    value= value << 1;
  }
}

/* -----------------------------------------------------
                      oled::lcd_setxypos

        addressiert das Displayram in Abhaengigkeit
        der X-Y Koordinate
   ----------------------------------------------------- */
void oled::setxypos(uint8_t x, uint8_t y)
{
  if (oled_port== 2) p2_oled_cmdmode(); else p1_oled_cmdmode();
  y= 7-y;

  spi_out(0xb0 | (y & 0x0f));
  spi_out(0x10 | (x >> 4 & 0x0f));
  spi_out(x & 0x0f);
}


/*  ---------------------------------------------------------
                       oled::setxybyte

      setzt ein Byte an Koordinate x,y

      Anmerkung:
            da Display monochrom werden (leider) immer
            8 Pixel in Y Richtung geschrieben, daher ist
            Wertebereich fuer y = 0..7 !

            Bsp. Koordinate y== 6 beschreibt tatsaechliche
            y-Koordinaten 48-55 (inclusive)
    --------------------------------------------------------- */
void oled::setxybyte(uint8_t x, uint8_t y, uint8_t value)
{
    if (oled_port== 2) p2_oled_cmdmode(); else p1_oled_cmdmode();
    y= 7-y;

    spi_out(0xb0 | (y & 0x0f));   
    spi_out(0x10 | (x >> 4 & 0x0f));
    spi_out(x & 0x0f);

    if (oled_port== 2) p2_oled_datamode(); else p1_oled_datamode();
    
    spi_out(value);
}

/*  ---------------------------------------------------------
                           oled::clrscr

      loescht den Displayinhalt mit der in bkcolor ange-
      gebenen "Farbe" (0 = schwarz, 1 = hell)
    --------------------------------------------------------- */

void oled::clrscr(void)
{
  uint8_t x,y;

  if (oled_port== 2)
  {
     p2_oled_enable();

     p2_rst_clr();                    // Display-reset
     _delay_ms(10);
     p2_rst_set();
  }
  else
  {
     p1_oled_enable();

     p1_rst_clr();                    // Display-reset
     _delay_ms(10);
     p1_rst_set();
  }

  // nachfolgende Daten als Kommando senden
  if (oled_port== 2) { p2_oled_cmdmode(); } else { p1_oled_cmdmode(); }

  spi_out(0x8d);                // Ladungspumpe an
  spi_out(0x14);

  spi_out(0xaf);                // Display on

  spi_out(0xa1);                // Segment Map

  spi_out(0xc0);                // Direction Map
  

  for (y= 0; y< 8; y++)         // ein Byte in Y-Achse = 8 Pixel...
                                // 8*8Pixel = 64 Y-Reihen
  {
    if (oled_port== 2) { p2_oled_cmdmode(); } else { p1_oled_cmdmode(); }

    spi_out(0xb0 | y);          // Pageadresse schreiben
    spi_out(0x00);              // MSB X-Adresse
    spi_out(0x00);              // LSB X-Adresse (+Offset)

    if (oled_port== 2) { p2_oled_datamode(); } else { p1_oled_datamode(); }
    
    for (x= 0; x< 128; x++)
    {
      if (bkcolor) spi_out(0xff); else spi_out(0x00);
    }
  }
  gotoxy(0,0);
}

/*  ---------------------------------------------------------
                          oled::gotoxy

       legt die naechste Textausgabeposition auf dem
       Display fest. Koordinaten 0,0 bezeichnet linke obere
       Position
    --------------------------------------------------------- */
void oled::gotoxy(uint8_t x, uint8_t y)
{
  aktxp= x;
  aktyp= y;
  x *= 8;
  if (rot270 == 1) x= 127-x; else y= 7-y;
  
  if (oled_port== 2) p2_oled_cmdmode(); else p1_oled_cmdmode(); 
  
  spi_out(0xb0 | (y & 0x0f));
  spi_out(0x10 | (x >> 4 & 0x0f));
  spi_out(x & 0x0f);
  
  if (oled_port== 2) p2_oled_datamode(); else p1_oled_datamode();
  
}

/*  ---------------------------------------------------------
                         oled::reversebyte

       spiegelt die Bits eines Bytes. D0 tauscht mit D7
       die Position, D1 mit D6 etc.
    --------------------------------------------------------- */
uint8_t oled::reversebyte(uint8_t value)
{
  uint8_t hb, b;

  hb= 0;
  for (b= 0; b< 8; b++)
  {
    if (value & (1 << b)) hb |= (1 << (7-b));
  }
  return hb;
}


/*  ---------------------------------------------------------
                         oled::directputchar

       gibt ein Zeichen auf dem Display aus. Steuerzeichen
       (fuer bspw. printf) sind implementiert:

               13 = carriage return
               10 = line feed
                8 = delete last char
    --------------------------------------------------------- */
void  oled::directputchar(uint8_t ch)
{
  uint8_t  i, b;
  uint8_t  z1;
  uint16_t z2[8];
  uint16_t z;
  uint8_t  outbyte;
  char     ox;

  if (ch== 13)                                          // Fuer <printf> "/r" Implementation
  {
    aktxp= 0;
    gotoxy(aktxp, aktyp);
    return;
  }
  if (ch== 10)                                          // fuer <printf> "/n" Implementation
  {
    aktyp++;
    if (doublechar) aktyp++;
    gotoxy(aktxp, aktyp);
    return;
  }

  if (ch== 8)
  {
    if ((aktxp> 0))
    {

      aktxp--;
      gotoxy(aktxp, aktyp);

      for (i= 0; i< 8; i++)
      {
       if (invchar) spi_out(0xff); else spi_out(0x00);
      }
      gotoxy(aktxp, aktyp);
    }
    return;

  }

  if (doublechar)
  {
    ox= aktxp;
    gotoxy(aktxp + 2, aktyp);

    for (i= 0; i< 8; i++)
    {
      // Zeichen auf ein 16x16 Zeichen vergroessern
      z1= pgm_read_byte(&(font8x8[ch-' '][7-i]));
      z2[i]= 0;
      for (b= 0; b< 8; b++)
      {
        if (z1 & (1 << b))
        {
          z2[i] |= (1 << (b*2));
          z2[i] |= (1 << ((b*2)+1));
        }
      }
    }

    for (i= 0; i< 8; i++)
    {
      z= z2[i];
      if (invchar) z= ~z;
      z= z & 0xff;
      if (bkcolor) z= ~z;
      spi_out(z);
      spi_out(z);
    }
    gotoxy(aktxp, aktyp+1);

    for (i= 0; i< 8; i++)
    {
      z= z2[i];
      if (invchar) z= ~z;
      z= z >> 8;
      if (bkcolor) z= ~z;
      spi_out(z);
      spi_out(z);
    }
    aktyp--;
    aktxp =ox+2;

    if (aktxp> 15)
    {
      aktxp= 0;
      aktyp +=2;
    }
    gotoxy(aktxp,aktyp);
  }
  else                                   // normale Ausgabe
  {
    ox= aktxp;
    gotoxy(aktxp + 1, aktyp);

    for (i= 0; i< 8; i++)
    {
      outbyte= pgm_read_byte( &(font8x8[ch-' '][7-i]) );

      if (invchar) spi_out(~(outbyte));
              else spi_out(outbyte);
    }

    aktxp= ox+1;
    if (aktxp> 15)
    {
      aktxp= 0;
      aktyp++;
    }
    gotoxy(aktxp,aktyp);
  }
}

/*  ---------------------------------------------------------
                         oled::doublebits

      dupliziert Bits eines Nibbles, so dass diese "doppelt"
      vorhanden sind (von Zeichenausgabe mit textsize > 0
      benoetigt).

      Uebergabe:
            b      : zu duplizierendes Byte
            nibble : Nibble, dessen Bytes dupliziert werden
                     soll

      Bsp.:
         b= doublebits(0xc5, 0);

         Unteres Nibble ist relevant,
         Value hier = 5 entspricht 0101b

         b hat den Wert 00110011b
    --------------------------------------------------------- */
uint8_t oled::doublebits(uint8_t b, uint8_t nibble)
{
  uint8_t b2;

  if (nibble) b = b >> 4;
  b= b & 0x0f;
  b2= 0;
  b2 = ((b & 1) << 0) | ((b & 1) << 1) | ((b & 2) << 1) | ((b & 2) << 2) |           //
       ((b & 4) << 2) | ((b & 4) << 3) | ((b & 8) << 3) | ((b & 8) << 4);

  return b2;
}

/*  ---------------------------------------------------------
                           oled::setfont

      legt Schriftstil fuer die Ausgabe fest.

      fnr== 0  => Font 5x7
      fnr== 1  => Font 8x8
    --------------------------------------------------------- */
void oled::setfont(uint8_t fnr)
{
  if (fnr > 1) { fontnr= 0; return; };
  fontnr= fnr;
  if (fnr == 0) { fontsizex= 6; return; };
  if (fnr == 1) { fontsizex= 8; return; };
  return ;
}

/* ------------------------------------------------------------
                         oled::putpixel

     setzt einen Pixel im Framebufferspeicher an Position
     x,y.

     col:      0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen
   ------------------------------------------------------------ */
void oled::putpixel(uint8_t x, uint8_t y, uint8_t col)
{
  uint16_t fbi;
  uint8_t  xr, yr, pixpos;

  // "Kopfstehende Ausgabe
  x= vram[0]-x; y= (vram[1] << 3)-y;

  xr= vram[0]; yr= vram[1];
  fbi= ((y >> 3) * xr) + 2 + x;
  pixpos= 7- (y & 0x07);

  switch (col)
  {
    case 0  : vram[fbi] &= ~(1 << pixpos); break;
    case 1  : vram[fbi] |= 1 << pixpos; break;
    case 2  : vram[fbi] ^= 1 << pixpos; break;

    default : break;
  }
}

/* ------------------------------------------------------------
                           oled::line

     Zeichnet eine Linie von den Koordinaten x0,y0 zu x1,y1
     im Screenspeicher.

     Linienalgorithmus nach Bresenham (www.wikipedia.org)

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen
   ------------------------------------------------------------ */
void oled::line(int x0, int y0, int x1, int y1, uint8_t col)
{

  //    Linienalgorithmus nach Bresenham (www.wikipedia.org)

  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2;

  for(;;)
  {
    putpixel(x0,y0, col);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
}

/* ------------------------------------------------------------
                           oled::rectangle

     Zeichnet ein Rechteck von den Koordinaten x0,y0 zu x1,y1
     im Screenspeicher.

     Linienalgorithmus nach Bresenham (www.wikipedia.org)

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen
   ------------------------------------------------------------ */
void oled::rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t col)
{
  line(x1,y1,x2,y1, col);
  line(x2,y1,x2,y2, col);
  line(x1,y2,x2,y2, col);
  line(x1,y1,x1,y2, col);
}

/* ------------------------------------------------------------
                           oled::ellipse

     Zeichnet eine Ellipse mit Mittelpunt an der Koordinate xm,ym
     mit den Hoehen- Breitenverhaeltnis a:b
     im Screenspeicher.

     Ellipsenalgorithmus nach Bresenham (www.wikipedia.org)

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen
   ------------------------------------------------------------ */
void oled::ellipse(int xm, int ym, int a, int b, uint8_t col )
{
  // Algorithmus nach Bresenham (www.wikipedia.org)

  int dx = 0, dy = b;                       // im I. Quadranten von links oben nach rechts unten
  long a2 = a*a, b2 = b*b;
  long err = b2-(2*b-1)*a2, e2;             // Fehler im 1. Schritt */

  do
  {
    putpixel(xm+dx, ym+dy,col);            // I.   Quadrant
    putpixel(xm-dx, ym+dy,col);            // II.  Quadrant
    putpixel(xm-dx, ym-dy,col);            // III. Quadrant
    putpixel(xm+dx, ym-dy,col);            // IV.  Quadrant

    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; }
    if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; }
  } while (dy >= 0);

  while (dx++ < a)                         // fehlerhafter Abbruch bei flachen Ellipsen (b=1)
  {
    putpixel(xm+dx, ym,col);              // -> Spitze der Ellipse vollenden
    putpixel(xm-dx, ym,col);
  }
}

/* ------------------------------------------------------------
                            oled::circle

     Zeichnet einen Kreis mit Mittelpunt an der Koordinate xm,ym
     und dem Radius r im Screenspeicher.

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen
   ------------------------------------------------------------ */
void oled::circle(int x, int y, int r, uint8_t col )
{
  ellipse(x,y,r,r,col);
}

/* ------------------------------------------------------------
                         oled::fastxline

     zeichnet eine Linie in X-Achse mit den X Punkten
     x1 und x2 auf der Y-Achse y1

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen

   ------------------------------------------------------------ */
void oled::fastxline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t col)
{
  uint8_t x;

  if (x2< x1) { x= x1; x1= x2; x= x2= x; }

  for (x= x1; x< (x2+1); x++)
  {
    putpixel(x,y1, col);
  }
}

/* ------------------------------------------------------------
                         oled::fillrect

     zeichnet ein ausgefuelltes Rechteck mit den
     Koordinatenpaaren x1/y1 (linke obere Ecke) und
     x2/y2 (rechte untere Ecke);

     col       0 = loeschen
               1 = setzen
               2 = Pixelpositon im XOR-Modus verknuepfen

   ------------------------------------------------------------ */
void oled::fillrect(int x1, int y1, int x2, int y2, uint8_t col)
{
  int y;

  if (y1> y2)
  {
    y= y1;
    y1= y2;
    y2= y;
  }

  for (y= y1; y< y2+1; y++)
  {
    fastxline(x1,y,x2, col);
  }
}

/* ---------------------------------------------------------------
                          oled::fillellipse

     Zeichnet eine ausgefuellte Ellipse mit Mittelpunt an der
     Koordinate xm,ym mit den Hoehen- Breitenverhaeltnis a:b
     mit der angegebenen Farbe

     Parameter:
        xm,ym = Koordinate des Mittelpunktes der Ellipse
        a,b   = Hoehen- Breitenverhaeltnis

        col       0 = loeschen
                  1 = setzen
                  2 = Pixelpositon im XOR-Modus verknuepfen


     Ellipsenalgorithmus nach Bresenham (www.wikipedia.org)
   --------------------------------------------------------------- */
void oled::fillellipse(int xm, int ym, int a, int b, uint8_t col )
{
  // Algorithmus nach Bresenham (www.wikipedia.org)

  int dx = 0, dy = b;                       // im I. Quadranten von links oben nach rechts unten
  long a2 = a*a, b2 = b*b;
  long err = b2-(2*b-1)*a2, e2;             // Fehler im 1. Schritt */

  do
  {
    fastxline(xm+dx, ym+dy,xm-dx, col);            // I. und II.   Quadrant
    fastxline(xm-dx, ym-dy,xm+dx, col);            // III. und IV. Quadrant

    e2 = 2*err;
    if (e2 <  (2*dx+1)*b2) { dx++; err += (2*dx+1)*b2; }
    if (e2 > -(2*dy-1)*a2) { dy--; err -= (2*dy-1)*a2; }
  } while (dy >= 0);

  while (dx++ < a)                        // fehlerhafter Abbruch bei flachen Ellipsen (b=1)
  {
    putpixel(xm+dx, ym,col);             // -> Spitze der Ellipse vollenden
    putpixel(xm-dx, ym,col);
  }
}

/* ---------------------------------------------------------------
                          oled::fillcircle

     Zeichnet einen ausgefuellten Kreis mit Mittelpunt an der
     Koordinate xm,ym und dem Radius r mit der angegebenen Farbe

     Parameter:
        xm,ym = Koordinate des Mittelpunktes der Ellipse
        r     = Radius des Kreises

        col       0 = loeschen
                  1 = setzen
                  2 = Pixelpositon im XOR-Modus verknuepfen
   --------------------------------------------------------------- */
void oled::fillcircle(int x, int y, int r, uint8_t col )
{
  fillellipse(x,y,r,r,col);
}

/* ----------------------------------------------------------
                         oled::fb_init

     initalisiert einen Framebufferspeicher.

     x = Framebuffergroesse in x Richtung
     y = Framebuffergroesse in y Richtung
         Aufloesung der y-Pixel muss durch 8 geteilt werden

     Bsp. fuer einen Framebuffer der Pixelgroesse 128x64

                  fb_init(128, 8);

     benoetigt als Framebufferspeicher (128*8)+2 = 1026 Bytes
   ---------------------------------------------------------- */
void oled::fb_init(uint8_t x, uint8_t y)
{
  vram[0]= x;
  vram[1]= y;
}

/* ----------------------------------------------------------
                         oled::fb_clear

     loescht den Framebufferspeicher
   ---------------------------------------------------------- */
void oled::fb_clear(void)
{
  uint16_t i;

  for (i= 2; i< fb_size; i++)
  {
    if (bkcolor) vram[i]= 0xff; else vram[i]= 0x00;
  }
}

/* ----------------------------------------------------------
                        oled::putcharxy

     gibt ein Zeichen auf dem Framebuffer aus
   ---------------------------------------------------------- */
void oled::putcharxy(uint8_t x, uint8_t y, uint8_t ch)
{
  uint8_t xo, yo;
  uint8_t rb, rt;

  if (textsize < 2)
  {
    for (xo= 0; xo < fontsizex; xo++)
    {
      if (fontnr) rb= pgm_read_byte(&(font8x8[ch-32][xo]));
             else rb= pgm_read_byte(&(font5x7[ch-32][xo]));
      if ((xo== 5) && (fontsizex== 6)) rb= 0;

      if (invchar) {rb= ~rb;}

      for (yo= 0; yo < 8; yo++)
      {
        if (rb & 0x01) putpixel(x+(xo*(textsize+1)), y+yo, 1);
                  else putpixel(x+(xo*(textsize+1)), y+yo, 0);
        if (textsize)
        {
          if (rb & 0x01) putpixel(x+(xo*(textsize+1)+1), y+yo, 1);
                    else putpixel(x+(xo*(textsize+1)+1), y+yo, 0);
        }
        rb= rb >> 1;
      }
    }
  }

  if (textsize == 2)
  {
    for (xo= 0; xo < fontsizex; xo++)
    {
      if (fontnr) rb= pgm_read_byte(&(font8x8[ch-32][xo]));
             else rb= pgm_read_byte(&(font5x7[ch-32][xo]));

      if (invchar) {rb= ~rb;}
      rt= rb;
      rb= doublebits(rb, 0);

      for (yo= 0; yo < 8; yo++)
      {
        if (rb & 0x01)
        {
          putpixel(x+(xo*2), y+yo, 1);
          putpixel(x+(xo*2)+1, y+yo, 1);
        }
        else
        {
          putpixel(x+(xo*2), y+yo, 0);
          putpixel(x+(xo*2)+1, y+yo, 0);
        }
        rb= rb >> 1;
      }

      rb= doublebits(rt, 1);

      for (yo= 0; yo < 8; yo++)
      {
        if (rb & 0x01)
        {
          putpixel(x+(xo*2), y+yo+8, 1);
          putpixel(x+(xo*2)+1, y+yo+8, 1);
        }
        else
        {
          putpixel(x+(xo*2), y+yo+8, 0);
          putpixel(x+(xo*2)+1, y+yo+8, 0);
        }
        rb= rb >> 1;
      }
    }
  }
}

/* ------------------------------------------------------------
                      oled::outtextxy

     gibt einen Text auf dem Framebuffer aus (nicht auf dem
     Display)

     Uebergabe:
       x,y: Textkoordinate innerhalb des Framebuffers an der
            der Text ausgegeben wird.
        *p: Zeiger auf den Auszugebenden Text
   ------------------------------------------------------------ */
void oled::outtextxy(uint8_t x, uint8_t y, char *p)
{
  while (*p)
  {
    putcharxy(x,y, *p );
    x += fontsizex;
    p++;
  }
}

/* ----------------------------------------------------------
                        oled::bmpsw_show

   Kopiert ein im Flash abgelegtes Bitmap in den Screens-
   peicher. Bitmap muss byteweise in Zeilen gespeichert
   vorliegen Hierbei entspricht 1 Byte 8 Pixel.
   Bsp.: eine Reihe mit 6 Bytes entsprechen 48 Pixel
         in X-Achse

   ox,oy        => linke obere Ecke an der das Bitmap
                   angezeigt wird
   image        => das anzuzeigende Bitmap
   fwert        => Farbwert mit der das Pixel gezeichnet wird

   Speicherorganisation des Bitmaps:

   Y       X-Koordinate
   |        0  1  2  3  4  5  6  7    8  9 10 11 12 13 14 15
   K               Byte 0                    Byte 1
   o  0     D7 D6 D5 D4 D3 D2 D1 D0   D7 D6 D5 D4 D3 D2 D1 D0
   o
   r         Byte (Y*XBytebreite)     Byte (Y*XBytebreite)+1
   d  1     D7 D6 D5 D4 D3 D2 D1 D0   D7 D6 D5 D4 D3 D2 D1 D0
   i
   n
   a
   t
   e
   ---------------------------------------------------------- */
void oled::bmpsw_show(uint16_t ox, uint16_t oy, const unsigned char* const image, uint16_t fwert)
{
  int      x, y;
  uint8_t  b, bp;
  uint16_t resX, resY;

  ox++;
  resX= (readarray(image,0) << 8) + readarray(image,1);
  resY= (readarray(image,2) << 8) + readarray(image,3);

  if ((resX % 8) == 0) { resX= resX / 8; }
                 else  { resX= (resX / 8) + 1; }

  for (y= 0; y< resY; y++)
  {
    for (x= 0; x < resX; x++)
    {
      b= readarray(image, y *resX + x + 4);
      for (bp= 8; bp> 0; bp--)
      {
        if (b & (1 << (bp-1))) { putpixel(ox + (x*8) + 8 - bp, oy+y,fwert); }
      }
    }
  }
}


/* ----------------------------------------------------------
                       oled::fb_show

   zeigt den Framebufferspeicher ab der Koordinate x,y
   (links oben) auf dem Display an
   ---------------------------------------------------------- */
void oled::fb_show(uint8_t x, uint8_t y)
{
  uint8_t   xp, yp;
  uint16_t  fb_ind;
  uint8_t   value;


  fb_ind= 2;
  for (yp= y; yp< vram[1]+y; yp++)
  {
    setxypos(x, yp);

    if (oled_port== 2) p2_oled_datamode(); else p1_oled_datamode();

    for (xp= x; xp< vram[0]+x; xp++)
    {
      value= vram[fb_ind];

      spi_out(value);

      fb_ind++;
    }
  }

}

/* ---------------------------------------------------------
                           font8x8h

      Zeichensatz fuer OLED Display mit SSD1306 Controller
   --------------------------------------------------------- */

const uint8_t PROGMEM font8x8[][8] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },    // Ascii 32 = ' '
  { 0x00, 0x00, 0x00, 0x5f, 0x5f, 0x00, 0x00, 0x00 },    // Ascii 33 = '!'
  { 0x00, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x00 },    // Ascii 34 = '"'
  { 0x14, 0x7f, 0x7f, 0x14, 0x7f, 0x7f, 0x14, 0x00 },    // Ascii 35 = '#'
  { 0x00, 0x24, 0x2a, 0x7f, 0x7f, 0x2a, 0x12, 0x00 },    // Ascii 36 = '$'
  { 0x00, 0x46, 0x66, 0x30, 0x18, 0x0c, 0x66, 0x62 },    // Ascii 37 = '%'
  { 0x00, 0x30, 0x7a, 0x4f, 0x5d, 0x37, 0x7a, 0x48 },    // Ascii 38 = '&'
  { 0x00, 0x00, 0x04, 0x07, 0x03, 0x00, 0x00, 0x00 },    // Ascii 39 = '''
  { 0x00, 0x00, 0x1c, 0x3e, 0x63, 0x41, 0x00, 0x00 },    // Ascii 40 = '('
  { 0x00, 0x00, 0x41, 0x63, 0x3e, 0x1c, 0x00, 0x00 },    // Ascii 41 = ')'
  { 0x08, 0x2a, 0x3e, 0x1c, 0x1c, 0x3e, 0x2a, 0x08 },    // Ascii 42 = '*'
  { 0x00, 0x08, 0x08, 0x3e, 0x3e, 0x08, 0x08, 0x00 },    // Ascii 43 = '+'
  { 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00 },    // Ascii 44 = ','
  { 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 },    // Ascii 45 = '-'
  { 0x00, 0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 0x00 },    // Ascii 46 = '.'
  { 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x01, 0x00 },    // Ascii 47 = '/'
  { 0x3e, 0x7f, 0x51, 0x49, 0x45, 0x7f, 0x3e, 0x00 },    // Ascii 48 = '0'
  { 0x00, 0x00, 0x02, 0x7f, 0x7f, 0x00, 0x00, 0x00 },    // Ascii 49 = '1'
  { 0x00, 0x42, 0x63, 0x71, 0x59, 0x4f, 0x46, 0x00 },    // Ascii 50 = '2'
  { 0x00, 0x22, 0x63, 0x49, 0x49, 0x7f, 0x36, 0x00 },    // Ascii 51 = '3'
  { 0x18, 0x1c, 0x16, 0x13, 0x7f, 0x7f, 0x10, 0x00 },    // Ascii 52 = '4'
  { 0x00, 0x27, 0x67, 0x45, 0x45, 0x7d, 0x39, 0x00 },    // Ascii 53 = '5'
  { 0x00, 0x3e, 0x7f, 0x49, 0x49, 0x7b, 0x32, 0x00 },    // Ascii 54 = '6'
  { 0x00, 0x01, 0x01, 0x71, 0x79, 0x0f, 0x07, 0x00 },    // Ascii 55 = '7'
  { 0x00, 0x36, 0x7f, 0x49, 0x49, 0x7f, 0x36, 0x00 },    // Ascii 56 = '8'
  { 0x00, 0x06, 0x4f, 0x69, 0x39, 0x1f, 0x0e, 0x00 },    // Ascii 57 = '9'
  { 0x00, 0x00, 0x00, 0x6c, 0x6c, 0x00, 0x00, 0x00 },    // Ascii 58 = ':'
  { 0x00, 0x00, 0x01, 0x6d, 0x6c, 0x00, 0x00, 0x00 },    // Ascii 59 = ';'
  { 0x00, 0x08, 0x1c, 0x36, 0x63, 0x41, 0x00, 0x00 },    // Ascii 60 = '<'
  { 0x00, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x00 },    // Ascii 61 = '='
  { 0x00, 0x41, 0x63, 0x36, 0x1c, 0x08, 0x00, 0x00 },    // Ascii 62 = '>'
  { 0x00, 0x02, 0x03, 0x51, 0x59, 0x0f, 0x06, 0x00 },    // Ascii 63 = '?'
  { 0x3e, 0x7f, 0x41, 0x5d, 0x5d, 0x5f, 0x1e, 0x00 },    // Ascii 64 = '@'
  { 0x7c, 0x7e, 0x13, 0x11, 0x13, 0x7e, 0x7c, 0x00 },    // Ascii 65 = 'A'
  { 0x7f, 0x7f, 0x49, 0x49, 0x49, 0x7f, 0x36, 0x00 },    // Ascii 66 = 'B'
  { 0x1c, 0x3e, 0x63, 0x41, 0x41, 0x63, 0x22, 0x00 },    // Ascii 67 = 'C'
  { 0x7f, 0x7f, 0x41, 0x41, 0x63, 0x3e, 0x1c, 0x00 },    // Ascii 68 = 'D'
  { 0x7f, 0x7f, 0x49, 0x49, 0x49, 0x41, 0x41, 0x00 },    // Ascii 69 = 'E'
  { 0x7f, 0x7f, 0x09, 0x09, 0x09, 0x01, 0x01, 0x00 },    // Ascii 70 = 'F'
  { 0x1c, 0x3e, 0x63, 0x41, 0x51, 0x73, 0x72, 0x00 },    // Ascii 71 = 'G'
  { 0x7f, 0x7f, 0x08, 0x08, 0x08, 0x7f, 0x7f, 0x00 },    // Ascii 72 = 'H'
  { 0x00, 0x41, 0x7f, 0x7f, 0x41, 0x00, 0x00, 0x00 },    // Ascii 73 = 'I'
  { 0x30, 0x70, 0x40, 0x40, 0x40, 0x7f, 0x3f, 0x00 },    // Ascii 74 = 'J'
  { 0x7f, 0x7f, 0x09, 0x1d, 0x36, 0x63, 0x41, 0x00 },    // Ascii 75 = 'K'
  { 0x7f, 0x7f, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00 },    // Ascii 76 = 'L'
  { 0x7f, 0x7f, 0x06, 0x0c, 0x06, 0x7f, 0x7f, 0x00 },    // Ascii 77 = 'M'
  { 0x7f, 0x7f, 0x06, 0x0c, 0x18, 0x7f, 0x7f, 0x00 },    // Ascii 78 = 'N'
  { 0x1c, 0x3e, 0x63, 0x41, 0x63, 0x3e, 0x1c, 0x00 },    // Ascii 79 = 'O'
  { 0x7f, 0x7f, 0x09, 0x09, 0x09, 0x0f, 0x06, 0x00 },    // Ascii 80 = 'P'
  { 0x1c, 0x3e, 0x63, 0x51, 0x33, 0x6e, 0x5c, 0x00 },    // Ascii 81 = 'Q'
  { 0x7f, 0x7f, 0x09, 0x09, 0x19, 0x7f, 0x66, 0x00 },    // Ascii 82 = 'R'
  { 0x26, 0x6f, 0x49, 0x49, 0x49, 0x7b, 0x32, 0x00 },    // Ascii 83 = 'S'
  { 0x01, 0x01, 0x7f, 0x7f, 0x01, 0x01, 0x00, 0x00 },    // Ascii 84 = 'T'
  { 0x3f, 0x7f, 0x40, 0x40, 0x40, 0x7f, 0x3f, 0x00 },    // Ascii 85 = 'U'
  { 0x1f, 0x3f, 0x60, 0x40, 0x60, 0x3f, 0x1f, 0x00 },    // Ascii 86 = 'V'
  { 0x7f, 0x7f, 0x30, 0x18, 0x30, 0x7f, 0x7f, 0x00 },    // Ascii 87 = 'W'
  { 0x63, 0x77, 0x1c, 0x08, 0x1c, 0x77, 0x63, 0x00 },    // Ascii 88 = 'X'
  { 0x00, 0x07, 0x0f, 0x78, 0x78, 0x0f, 0x07, 0x00 },    // Ascii 89 = 'Y'
  { 0x41, 0x61, 0x71, 0x59, 0x4d, 0x47, 0x43, 0x00 },    // Ascii 90 = 'Z'
  { 0x00, 0x00, 0x7f, 0x7f, 0x41, 0x41, 0x00, 0x00 },    // Ascii 91 = '['
  { 0x01, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00 },    // Ascii 92 = '\'
  { 0x00, 0x00, 0x41, 0x41, 0x7f, 0x7f, 0x00, 0x00 },    // Ascii 93 = ']'
  { 0x00, 0x08, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x08 },    // Ascii 94 = '^'
  { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },    // Ascii 95 = '_'
  { 0x00, 0x00, 0x01, 0x03, 0x06, 0x04, 0x00, 0x00 },    // Ascii 96 = '`'
  { 0x20, 0x74, 0x54, 0x54, 0x54, 0x7c, 0x78, 0x00 },    // Ascii 97 = 'a'
  { 0x7f, 0x7f, 0x44, 0x44, 0x44, 0x7c, 0x38, 0x00 },    // Ascii 98 = 'b'
  { 0x38, 0x7c, 0x44, 0x44, 0x44, 0x6c, 0x28, 0x00 },    // Ascii 99 = 'c'
  { 0x38, 0x7c, 0x44, 0x44, 0x44, 0x7f, 0x7f, 0x00 },    // Ascii 100 = 'd'
  { 0x38, 0x7c, 0x54, 0x54, 0x54, 0x5c, 0x18, 0x00 },    // Ascii 101 = 'e'
  { 0x08, 0x7e, 0x7f, 0x09, 0x09, 0x03, 0x02, 0x00 },    // Ascii 102 = 'f'
  { 0x98, 0xbc, 0xa4, 0xa4, 0xa4, 0xfc, 0x7c, 0x00 },    // Ascii 103 = 'g'
  { 0x7f, 0x7f, 0x04, 0x04, 0x04, 0x7c, 0x78, 0x00 },    // Ascii 104 = 'h'
  { 0x00, 0x00, 0x00, 0x7d, 0x7d, 0x00, 0x00, 0x00 },    // Ascii 105 = 'i'
  { 0x40, 0xc0, 0x80, 0x80, 0xfd, 0x7d, 0x00, 0x00 },    // Ascii 106 = 'j'
  { 0x7f, 0x7f, 0x10, 0x10, 0x38, 0x6c, 0x44, 0x00 },    // Ascii 107 = 'k'
  { 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x00, 0x00, 0x00 },    // Ascii 108 = 'l'
  { 0x78, 0x7c, 0x0c, 0x18, 0x0c, 0x7c, 0x78, 0x00 },    // Ascii 109 = 'm'
  { 0x7c, 0x7c, 0x04, 0x04, 0x04, 0x7c, 0x78, 0x00 },    // Ascii 110 = 'n'
  { 0x38, 0x7c, 0x44, 0x44, 0x44, 0x7c, 0x38, 0x00 },    // Ascii 111 = 'o'
  { 0xfc, 0xfc, 0x24, 0x24, 0x24, 0x3c, 0x18, 0x00 },    // Ascii 112 = 'p'
  { 0x18, 0x3c, 0x24, 0x24, 0x24, 0xfc, 0xfc, 0x00 },    // Ascii 113 = 'q'
  { 0x7c, 0x7c, 0x04, 0x04, 0x04, 0x0c, 0x08, 0x00 },    // Ascii 114 = 'r'
  { 0x48, 0x5c, 0x54, 0x54, 0x54, 0x74, 0x20, 0x00 },    // Ascii 115 = 's'
  { 0x04, 0x3f, 0x7f, 0x44, 0x44, 0x64, 0x20, 0x00 },    // Ascii 116 = 't'
  { 0x3c, 0x7c, 0x40, 0x40, 0x40, 0x7c, 0x7c, 0x00 },    // Ascii 117 = 'u'
  { 0x1c, 0x3c, 0x60, 0x40, 0x60, 0x3c, 0x1c, 0x00 },    // Ascii 118 = 'v'
  { 0x3c, 0x7c, 0x60, 0x30, 0x60, 0x7c, 0x3c, 0x00 },    // Ascii 119 = 'w'
  { 0x44, 0x6c, 0x38, 0x10, 0x38, 0x6c, 0x44, 0x00 },    // Ascii 120 = 'x'
  { 0x9c, 0xbc, 0xa0, 0xa0, 0xa0, 0xfc, 0x7c, 0x00 },    // Ascii 121 = 'y'
  { 0x00, 0x44, 0x64, 0x74, 0x5c, 0x4c, 0x44, 0x00 },    // Ascii 122 = 'z'
  { 0x00, 0x08, 0x08, 0x3e, 0x77, 0x41, 0x41, 0x00 },    // Ascii 123 = '{'
  { 0x00, 0x00, 0x00, 0x77, 0x77, 0x00, 0x00, 0x00 },    // Ascii 124 = '|'
  { 0x00, 0x41, 0x41, 0x77, 0x3e, 0x08, 0x08, 0x00 },    // Ascii 125 = '}'
  { 0x00, 0x02, 0x01, 0x01, 0x02, 0x02, 0x01, 0x00 },    // Ascii 126 = '~'
  { 0x70, 0x78, 0x4c, 0x46, 0x46, 0x4c, 0x78, 0x70 },    // Ascii 127 = ''

  { 0x3c, 0x42, 0x81, 0x81, 0x81, 0x42, 0x3c, 0x00 },    // Ascii 128 abweichend: rundes o
  { 0x3c, 0x42, 0xbd, 0xbd, 0xbd, 0x42, 0x3c, 0x00 },    // Ascii 129 abweichend: ausgefuelltes o
  { 0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e, 0x00 }     // Ascii 130 abweichend: hochgestelltes o
};


const uint8_t PROGMEM font5x7[][5] =
{
  { 0x00, 0x00, 0x00, 0x00, 0x00 },   // space
  { 0x00, 0x00, 0x2f, 0x00, 0x00 },   // !
  { 0x00, 0x07, 0x00, 0x07, 0x00 },   // "
  { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // #
  { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $
  { 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // %
  { 0x36, 0x49, 0x55, 0x22, 0x50 },   // &
  { 0x00, 0x05, 0x03, 0x00, 0x00 },   // '
  { 0x00, 0x1c, 0x22, 0x41, 0x00 },   // (
  { 0x00, 0x41, 0x22, 0x1c, 0x00 },   // )
  { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // *
  { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // +
  { 0x00, 0x00, 0x50, 0x30, 0x00 },   // ,
  { 0x10, 0x10, 0x10, 0x10, 0x10 },   // -
  { 0x00, 0x60, 0x60, 0x00, 0x00 },   // .
  { 0x20, 0x10, 0x08, 0x04, 0x02 },   // /
  { 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0
  { 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1
  { 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2
  { 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3
  { 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4
  { 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5
  { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6
  { 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7
  { 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8
  { 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9
  { 0x00, 0x36, 0x36, 0x00, 0x00 },   // :
  { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ;
  { 0x08, 0x14, 0x22, 0x41, 0x00 },   // <
  { 0x14, 0x14, 0x14, 0x14, 0x14 },   // =
  { 0x00, 0x41, 0x22, 0x14, 0x08 },   // >
  { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ?
  { 0x32, 0x49, 0x59, 0x51, 0x3E },   // @
  { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A
  { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B
  { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C
  { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D
  { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E
  { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F
  { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G
  { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H
  { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I
  { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J
  { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K
  { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L
  { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M
  { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N
  { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O
  { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P
  { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q
  { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R
  { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S
  { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T
  { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U
  { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V
  { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W
  { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X
  { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y
  { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z
  { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [
  { 0x55, 0x2A, 0x55, 0x2A, 0x55 },   // "Yen"
  { 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ]
  { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^
  { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _
  { 0x00, 0x01, 0x02, 0x04, 0x00 },   // '
  { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a
  { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b
  { 0x38, 0x44, 0x44, 0x44, 0x20 },   // c
  { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d
  { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e
  { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f
  { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g
  { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h
  { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i
  { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j
  { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k
  { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l
  { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m
  { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n
  { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o
  { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p
  { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q
  { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r
  { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s
  { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t
  { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u
  { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v
  { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w
  { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x
  { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y
  { 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z
  // Zeichen vom Ascii-Satz abweichend
  { 0x3E, 0x7F, 0x7F, 0x3E, 0x00 },   // Zeichen 123 : ausgefuelltes Oval
  { 0x06, 0x09, 0x09, 0x06, 0x00 },   // Zeichen 124 : hochgestelltes kleines o (fuer Gradzeichen);
  { 0x01, 0x01, 0x01, 0x01, 0x01 },   // Zeichen 125 : Strich in der obersten Reihe
  { 0x00, 0x1D, 0x15, 0x17, 0x00 }    // Zeichen 126 : "hoch 2"
};

