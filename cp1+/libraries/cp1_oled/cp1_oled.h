/* -----------------------------------------------------
                         cp1_oled.h

    Header fuer das Anbinden eines OLED Displays mit
    SSD1306 Controller und SPI Interface

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
   
#ifndef in_cp1oled
#define in_cp1oled

#include "Arduino.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

enum font { fnt5x7, fnt8x8 };

#define _xres                 128
#define _yres                 64

#define  fb_size              1052               // Framebuffergroesse in Bytes (wenn fb_enable)

#define readarray(arr,ind)       (pgm_read_byte(&(arr[ind])))
    
class oled
{

public:
  uint8_t aktxp= 0;
  uint8_t aktyp= 0;
  
  uint8_t doublechar = 0;
  uint8_t bkcolor    = 0;
  uint8_t invchar    = 0;
  
  uint8_t fontnr     = 1;                               //  0 : 5x7  Font
                                                        //  1 : 8x8  Font
  
  uint8_t fontsizex  = 8;
  uint8_t textsize   = 0;                               // Skalierung der Ausgabeschriftgroesse 
  
  uint8_t vram[fb_size];

  /* -----------------------------------------------------
                        Konstruktor
     ----------------------------------------------------- */
     
  oled(uint8_t port);
  
  /* -----------------------------------------------------
                     Display Funktionen
     ----------------------------------------------------- */

  void gotoxy(uint8_t x, uint8_t y);
  void clrscr(void);
  void directputchar(uint8_t ch);

  /* -----------------------------------------------------
                   Framebuffer Funktionen
     ----------------------------------------------------- */

  void setfont(uint8_t fnr);
  void fb_init(uint8_t x, uint8_t y);
  void fb_clear(void);
  void bmpsw_show(uint16_t ox, uint16_t oy, const unsigned char* const image, uint16_t fwert);
  void fb_show(uint8_t x, uint8_t y);
  void putpixel(uint8_t x, uint8_t y, uint8_t col);
  void line(int x0, int y0, int x1, int y1, uint8_t col);
  void rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t col);
  void ellipse(int xm, int ym, int a, int b, uint8_t col );
  void circle(int x, int y, int r, uint8_t col );
  void fastxline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t col);
  void fillrect(int x1, int y1, int x2, int y2, uint8_t col);
  void fillellipse(int xm, int ym, int a, int b, uint8_t col );
  void fillcircle(int x, int y, int r, uint8_t col );
  void putcharxy(uint8_t x, uint8_t y, uint8_t ch);
  void outtextxy(uint8_t x, uint8_t y, char *p);  

protected:  

private:
  uint8_t rot270     = 1;
  uint8_t oled_port  = 2;  
  uint8_t txoutmode  = 0;
  

  void spi_init(void);
  void spi_out(uint8_t value);
  void setxypos(uint8_t x, uint8_t y);
  void setxybyte(uint8_t x, uint8_t y, uint8_t value);
  uint8_t reversebyte(uint8_t value);
  uint8_t doublebits(uint8_t b, uint8_t nibble);
  
};

/* -----------------------------------------------------
           Pindeklarationen fuer Display an Port1
   ----------------------------------------------------- */

#define p1_mosiinit()         ( DDRD |= (1 << 7) )
#define p1_csinit()           ( DDRB |= (1 << 2) )
#define p1_resinit()          ( DDRB |= (1 << 0) )
#define p1_dcinit()           ( DDRB |= (1 << 1) )
#define p1_sckinit()          ( DDRD |= (1 << 6) )

#define p1_dc_set()           ( PORTB |= (1 << 1) )
#define p1_dc_clr()           ( PORTB &= ~(1 << 1) )
#define p1_ce_set()           ( PORTB |= (1 << 2) )
#define p1_ce_clr()           ( PORTB &= ~(1 << 2) )
#define p1_rst_set()          ( PORTB |= (1 << 0) )
#define p1_rst_clr()          ( PORTB &= ~(1 << 0) )

#define p1_mosi_set()         ( PORTD |= (1 << 7) )
#define p1_mosi_clr()         ( PORTD &= ~(1 << 7) )
#define p1_sck_set()          ( PORTD |= (1 << 6) )
#define p1_sck_clr()          ( PORTD &= ~(1 << 6) )

#define p1_oled_enable()      ( p1_ce_clr() )
#define p1_oled_disable()     ( p1_ce_set() )
#define p1_oled_cmdmode()     ( p1_dc_clr() )      // OLED Kommando
#define p1_oled_datamode()    ( p1_dc_set() )      // OLED   

/* -----------------------------------------------------
         Pindeklarationen fuer Display an Port2
 ----------------------------------------------------- */   

#define p2_mosiinit()         ( DDRC |= (1 << 1) )
#define p2_csinit()           ( DDRD |= (1 << 4) )
#define p2_resinit()          ( DDRC |= (1 << 2) )
#define p2_dcinit()           ( DDRC |= (1 << 3) )
#define p2_sckinit()          ( DDRC |= (1 << 0) )

#define p2_dc_set()           ( PORTC |= (1 << 3) )
#define p2_dc_clr()           ( PORTC &= ~(1 << 3) )
#define p2_ce_set()           ( PORTD |= (1 << 4) )
#define p2_ce_clr()           ( PORTD &= ~(1 << 4) )
#define p2_rst_set()          ( PORTC |= (1 << 2) )
#define p2_rst_clr()          ( PORTC &= ~(1 << 2) )

#define p2_mosi_set()         ( PORTC |= (1 << 1) )
#define p2_mosi_clr()         ( PORTC &= ~(1 << 1) )
#define p2_sck_set()          ( PORTC |= (1 << 0) )
#define p2_sck_clr()          ( PORTC &= ~(1 << 0) )

#define p2_oled_enable()      ( p2_ce_clr() )
#define p2_oled_disable()     ( p2_ce_set() )
#define p2_oled_cmdmode()     ( p2_dc_clr() )      // OLED Kommando
#define p2_oled_datamode()    ( p2_dc_set() )      // OLED   

#endif

