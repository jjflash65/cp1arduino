/* ---------------------------------------------------------------------------
                               st7735.h

     Funktionen fuer TFT-Display mit ST7735 Grafikcontrollerchip

     20.03.2021    R. Seelig
   --------------------------------------------------------------------------- */
#ifndef in_st7735
 #define in_st7735

  #include "Arduino.h"
  #include <avr/pgmspace.h>  
  #include <avr/io.h>
  
  /*  ------------------------------------------------------------
                       EGA - Farbzuweisungen
    ------------------------------------------------------------ */
  
  #define black                   0
  #define blue                    1
  #define green                   2
  #define cyan                    3
  #define red                     4
  #define magenta                 5
  #define brown                   6
  #define grey                    7
  #define darkgrey                8
  #define lightblue               9
  #define lightgreen              10
  #define lightcyan               11
  #define lightred                12
  #define lightmagenta            13
  #define yellow                  14
  #define white                   15  
  
  enum {_RGB, _BGR };
  enum { FNT8x8, FNT12x16, FNT5x7};    
  
  uint16_t rgbfromvalue(uint8_t r, uint8_t g, uint8_t b);
  uint16_t rgbfromega(uint8_t entry);
  
  class st7735
  {
    public:   
      // ------------------------------------
      //            Variable Text
      // ------------------------------------
      
      int      aktxp;               // Beinhaltet die aktuelle Position des Textcursors in X-Achse
      int      aktyp;               // dto. fuer die Y-Achse
      uint16_t textcolor = 0xffff;  // Beinhaltet die Farbwahl fuer die Vordergrundfarbe
      uint16_t bkcolor = 0;         // dto. fuer die Hintergrundfarbe
      uint8_t  outmode = 0;
      uint8_t  textsize;            // Skalierung der Ausgabeschriftgroesse
      uint8_t  txoutmode = 0;       // Drehrichtung fuer die Textausgabe
      uint8_t  fntfilled = 1;       // gibt an, ob eine Zeichenausgabe ueber einen Hintergrund gelegt
                                    // wird, oder ob es mit der Hintergrundfarbe aufgefuellt wird
                                    // 1 = Hintergrundfarbe wird gesetzt, 0 = es wird nur das Fontbitmap
                                    // gesetzt, der Hintergrund wird belassen
      uint8_t  fontnr    = 0;       // standardmaessig ist 8x8 Font gesetzt
      uint8_t  fontsizex = 8;
      uint8_t  fontsizey = 8;
        
      st7735(uint8_t rst, uint8_t dc, uint8_t ce);
      void init(uint16_t xres, uint16_t yres, uint8_t mirror, uint8_t colfolge);
      void ofsmode(int8_t ofs);
      void version_g();
      void putpixel(int x, int y,uint16_t color);
      void clrscr();   
      void lcd_putchar(char ch);
      void putchar5x7(unsigned char ch);
      void putchar8x8(unsigned char ch);
      void putchar12x16(unsigned char ch);    
      void outtextxy(int x, int y, char *p);
      void setfont(uint8_t nr);
      void gotoxy(unsigned char x, unsigned char y);
      void line(int x0, int y0, int x1, int y1, uint16_t color);    
      void fastxline(uint8_t x1, uint8_t y1, uint8_t x2, uint16_t color);        
      void rectangle(int x1, int y1, int x2, int y2, uint16_t color);    
      void fillrect(int x1, int y1, int x2, int y2, uint16_t color);    
      void ellipse(int xm, int ym, int a, int b, uint16_t color );              
      void fillellipse(int xm, int ym, int a, int b, uint16_t color );         
      void circle(int x, int y, int r, uint16_t color );                         
      void fillcircle(int x, int y, int r, uint16_t color );                    
    
    protected:
  
    private:
      // ------------------------------------
      //            Variable Display-CTRL
      // ------------------------------------    
      
      uint16_t _xres     = 128;
      uint16_t _yres     = 128;
      
      uint8_t colofs     = 0;
      uint8_t rowofs     = 0;
      int8_t _lcyofs     = -32;
      
      uint8_t _mirror    = 0;
      
      #define coladdr      0x2a
      #define rowaddr      0x2b
      #define writereg     0x2c    
      
      void spi_init();
      void spi_lcdout(uint8_t data);
      void wrcmd(uint8_t cmd);
      void wrdata(uint8_t data);
      void wrdata16(int data);
      void set_ram_address (uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
      void setxypos(int x, int y);
      void setcol(int startcol);
      void setpage(int startpage);    
      void putpixeltx(int x, int y, uint16_t color);    
  };
    
#endif
   
