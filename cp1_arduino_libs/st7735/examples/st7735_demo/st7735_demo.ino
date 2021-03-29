/* -------------------------------------------------------------
        Demoprogramm fuer ST7735 128x128 Pixel Display
   ------------------------------------------------------------- */

#include <avr/pgmspace.h>
   
#include <string.h>
#include "my_printf.h"
#include "st7735.h"

/*
 Verdrahtung Display:
   da Hardware-SPI verwendet wird sind CLK und DIO des Displays nicht
   waehlbar.
   
   Anschluss CLK:  Arduino D13  =  AVR-PB5
   Anschluss DIO:  Arduino D11  =  AVR-PB3
   
   Anschluesse fuer RST, CE, DC sind frei waehlbar beim Erstellen des
   Objects in der Reihenfolge.
   
   rst= 8   = PB0
   dc=  9   = PB1
   ce=  10  = PB2
*/

st7735 lcd(8, 9, 10);         // Displayobjekt erzeugen


int t_lastx, t_lasty;       // x,y - Positionen der letzten Zeichenaktion von moveto


/* --------------------------------------------------
                       my_putchar
                       
     wird zwingend "cp1_printf.cpp" benoetigt! 
     Ueber diese Funktion erfolgen die Ausgaben von
     printf                       
   -------------------------------------------------- */
void my_putchar(char ch)
{
  lcd.lcd_putchar(ch);
}

/* --------------------------------------------------------
      turtle_moveto

                        Turtlegrafik

      setzt den Ausgangspunkt fuer Turtlegrafiken auf
      die angegebenen Koordinaten

        x,y  : Koordinaten, an der das Zeichnen beginnt
   -------------------------------------------------------- */
void turtle_moveto(int x, int y)
{
 t_lastx= x; t_lasty= y;
}

/* --------------------------------------------------------
     turtle_lineto

                       Turtlegrafik

     zeichnet eine Linie von der letzten Position zur
     angegebenen x,y - Koordinate mit der Farbe col

         x,y  : Position bis zu der eine Linie gezogen
                wird
        col   : 16 - Bit RGB565 Farbwert der gezeichnet
                 werden soll
   -------------------------------------------------------- */
void turtle_lineto(int x, int y, uint16_t col)
{
  lcd.line(x,y, t_lastx, t_lasty,col);
  turtle_moveto(x,y);
}


/* --------------------------------------------------------
                      spiro_generate

       zeichnet einee Spirographen auf das Display
   -------------------------------------------------------- */
void spiro_generate(int inner, int outer, int evol, int resol, int t, uint16_t col)
{
  const int c_width  = 128;
  const int c_height = 128;
  float     inner_xpos, inner_ypos;
  float     outer_xpos, outer_ypos;
  float     j, k;
  int       i;

  inner_xpos = (c_width / 2.0f);
  inner_ypos = (c_height / 2.0f) + inner;

  outer_xpos= inner_xpos;
  outer_ypos= inner_ypos + outer;
  turtle_moveto(outer_xpos, outer_ypos);

  for (i= 0; i< resol + 1; i++)
  {
    j= ((float)i / resol) * (2.0f * M_PI);
    inner_xpos = (c_width / 2.0f) + (inner * sin(j));
    inner_ypos = (c_height / 2.0f) + (inner * cos(j));

    k= j * ((float)evol / 10.0f);

    outer_xpos= inner_xpos + (outer * sin(k));
    outer_ypos= inner_ypos + (outer * cos(k));

    turtle_lineto(outer_xpos, outer_ypos, col);
    delay(t);
  }
}

/* -------------------------------------------------------
                         mandelbrot
                          
                    Mandelbrot-Generator
   ------------------------------------------------------- */
void mandelbrot(uint16_t graphwidth, uint16_t graphheight)
{
  uint16_t k, kt, x, y;
  float    dx, dy, xmin, xmax, ymin, ymax;
  float    jx, jy, wx, wy, tx, ty, r, m;
  uint8_t  cr, cg, cb;

  kt= 100; m= 4.0;

  xmin= 1.7; xmax= -0.8; ymin= -1.0; ymax= 1.0;

//  alternative Zahlenwerte

//  xmin= -0.5328; xmax= -0.2078; ymin= 0.3742; ymax= 0.892;

  dx= (float)(xmax-xmin) / graphwidth;
  dy= (float) (ymax-ymin) / graphheight;

  for (x= 0; x< graphwidth; x++)
  {
    jx= xmin + ((float)x*dx);

    for (y= 0; y< graphheight; y++)
    {
      jy= ymin+((float)y*dy);

      k= 0; wx= 0.0; wy= 0.0;
      do
      {
        tx= wx*wx-(wy*wy+jx);
        ty= 2.0*wx*wy+jy;
        wx= tx;
        wy= ty;
        r= wx*wx+wy+wy;

        k++;
      } while ((r < m) & (k < kt));

      if (k< 3) {cb= k * 20; }
      if ((k> 2) && (k< 35)) { cb= k * 9; cg = k; cr= k*2 ; }
      if (k> 34) { cr= ( 128 ); cg= (k * 3 ); cb= (k); }
      if (k> 90) { cr= 0, cg= 0; cb= 0; }

      lcd.putpixel(x,y,rgbfromvalue(cr, cg, cb));

    }
  }
}

void setup() 
{
  int i;

  lcd.version_g();
  lcd.init(128, 128, 0, _RGB);
  lcd.outmode= 0;  
  
  lcd.bkcolor= rgbfromvalue(0x80, 0x40, 0);  
  lcd.clrscr();

  lcd.line(3,3,120,127, rgbfromega(magenta));
  lcd.textcolor= rgbfromvalue(0, 0xff, 0x80);
  lcd.gotoxy(1,1);
  lcd.setfont(FNT8x8);
  printf("Spirograph");
  lcd.fastxline(0,22,127, rgbfromega(15));
  lcd.rectangle(4,4, 123,123, 0);
  lcd.fillrect(10,25, 60,50, rgbfromega(red));
  lcd.fillcircle(64, 64, 30, rgbfromega(14));
  delay(2000);
}

void loop() 
{
  lcd.bkcolor= 0;
  lcd.clrscr();
    
  spiro_generate(21, 21, 80, 220, 5, rgbfromvalue(50, 255, 50));
  spiro_generate(32, 32, 140,220, 5, rgbfromega(1));
  delay(5000);

  spiro_generate(32, 32, 140,220, 5, rgbfromvalue(0x00,0x20,0x00));;
  spiro_generate(21, 21, 80, 220, 5, rgbfromvalue(0x00,0xcf,0x00));
  delay(5000);

  spiro_generate(32, 32, 140,220, 5, rgbfromega(4));
  spiro_generate(21, 21, 80, 220, 5, rgbfromvalue(0xff,0x80,0x00));
  delay(5000);
  
  mandelbrot(128, 128);
  delay(5000);
}
