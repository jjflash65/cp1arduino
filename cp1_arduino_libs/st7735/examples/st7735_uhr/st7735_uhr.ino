/* -------------------------------------------------------------
        Analoge Uhr auf einem ST7735 128x128 Pixel Display
   ------------------------------------------------------------- */
   
#include <string.h>
#include "my_printf.h"
#include "st7735.h"

/*
 Verdrahtung Display:
   da Hardware-SPI verwendet wird sind CLK und DIO des Displays nicht
   waehlbar.
   
   Anschluss CLK:  Arduino D13  =  AVR-PB5 = P2_7 - CP1+
   Anschluss DIO:  Arduino D11  =  AVR-PB3 = P2_5 - CP1+
   
   Anschluesse fuer RST, CE, DC sind frei waehlbar beim Erstellen des
   Objects in der Reihenfolge.
   
   rst= 8   = PB0
   dc=  9   = PB1
   ce=  10  = PB2
   
 Hinweis:
   Bei Verwendung des internen Oszillators mit 8Mhz (und ohne externen)
   Quarz, laeuft die Uhr bis zu 2.5% falsch (was fuer eine Uhr sehr
   viel ist). Dieses Programm soll demonstrieren, wie eine analog an-
   zeigende Uhr grundsaetzlich aufgebaut sein kann.
   
*/

st7735 lcd(8, 9, 10);         // Displayobjekt erzeugen

/* --------------------------------------------------
           Variable und #define fuer Analoguhr
   -------------------------------------------------- */

#define xuo             2                                   // X-Achse Anzeigeoffset fuer analoge Uhrendarstellung
#define yuo             0                                   // dto. Y-Achse
#define ruhr            53                                  // Radius der analogen Uhrendarstellung
#define stdzeig         24                                  // Laenge Stundenzeiger, je kleiner Zahl umso groesser Zeiger

uint16_t ziffbk;                                            // Hintergrundfarbe des Ziffernblatts

char wtag[7][3] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

volatile int stund = 23;
volatile int minut = 31;
volatile int sek = 50;

volatile int year = 21;
volatile int month= 3;
volatile int day = 22;

volatile int oldstund, oldminut, oldsek;

#define button_rechts   6      // PD6
#define button_links    4      // PD4


#define is_butre()      (!(digitalRead(button_rechts)))
#define is_butli()      (!(digitalRead(button_links)))


/* ----------------------------------------------------------
                   Timing der Bedientasten
   ---------------------------------------------------------- */
#define tasthispeed     70
#define tastlospeed     400

/* --------------------------------------------------
           Variable fuer Turtle-Grafik
   -------------------------------------------------- */

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

/* -----------------------------------------------------------------------------
                                   Analog-Uhr
   ----------------------------------------------------------------------------- */

/* --------------------------------------------------------
                       zeigerpos

  Berechnung der Endkoordinaten eines graphischen Zeigers
  vom Mittelpuntk aus gesehen:

     x,y     : Koordinaten Mittelpunkt
     r       : Radius des Zeigers
     w       : Winkel des Zeigers
     x2,y2   : Koordinaten des Endpunktes desf Zeigers
   -------------------------------------------------------- */
void zeigerpos(int x, int y, int r, int w, int *x2, int *y2)
{
  int w2;
  float a;

  w2= 90 - w;
  a = r * (cos(w2 * (M_PI / 180.0f)));
  *x2= (int)x+a;
  a = r * (sin(w2 * (M_PI / 180.0f)));
  *y2= (int)y-a;
}

/* --------------------------------------------------------
                      drawstdskala

       Zeichnet die Stundeneinteilung einer Uhr
   -------------------------------------------------------- */
void drawstdskala(int x, int y, int r1, int r2, uint16_t col)
{
  int i;
  int zx1,zy1, zx2,zy2;

  for (i= 0; i < 12; i++)
  {
    zeigerpos(x,y,r1,i*30,&zx1,&zy1);
    zeigerpos(x,y,r2,i*30,&zx2,&zy2);
    lcd.line(zx1,zy1,zx2,zy2,col);
  }
}

/* --------------------------------------------------------
                      drawminskala

        Zeichnet die Minuteneinteilung einer Uhr
   -------------------------------------------------------- */
void drawminskala(int x, int y, int r1, int r2, uint16_t col)
{
  int i;
  int zx1,zy1, zx2,zy2;

  for (i= 0; i < 60; i++)
  {
    zeigerpos(x,y,r1,i*6,&zx1,&zy1);
    zeigerpos(x,y,r2,i*6,&zx2,&zy2);
    lcd.line(zx1,zy1,zx2,zy2,col);
  }
}

/* --------------------------------------------------------
                         drawzweiger

    Zeichnet einen "dicken" Zeiger, vom Mittelpunkt x,y
    mit dem  Radius r und dem Winkel w in der Farbe col
   --------------------------------------------------------*/
void drawzeiger(int x, int y, int r, int w, uint16_t col)
{
  int x2,y2;

  zeigerpos(x,y,r,w, &x2 ,&y2);
  lcd.line(x,y,x2,y2,col);

  zeigerpos(x+1,y,r,w, &x2 ,&y2);
  lcd.line(x+1,y,x2,y2,col);
  zeigerpos(x-1,y,r,w, &x2 ,&y2);
  lcd.line(x-1,y,x2,y2,col);

  zeigerpos(x,y+1,r,w, &x2 ,&y2);
  lcd.line(x,y+1,x2,y2,col);
  zeigerpos(x,y-1,r,w, &x2 ,&y2);
  lcd.line(x,y-1,x2,y2,col);
}


/* --------------------------------------------------------
                         drawsmallzweiger

    Zeichnet einen "duennen" Zeiger, vom Mittelpunkt x,y
    mit dem  Radius r und dem Winkel w in der Farbe col
   --------------------------------------------------------*/
void drawsmallzeiger(int x, int y, int r, int w, uint16_t col)
{
  int x2,y2;

  zeigerpos(x,y,r,w, &x2 ,&y2);
  lcd.line(x,y,x2,y2,col);
}

/* --------------------------------------------------------
                          showzeiger

      zeichnet die Zeiger einer Uhr.

      ziffbk   : Farbwert des Ziffernblatts der Uhr
      clear    : 0 = Zeiger werden gezeichnet
                 1 = Zeiger werden mit Hintergrundfarbe
                     gezeichnet (und somit geloescht)
   -------------------------------------------------------- */
void showzeiger(uint8_t astd, uint8_t amin, uint8_t asek, uint16_t ziffbk, uint8_t clear)
{
  if (astd > 12) astd -= 12;
  if (clear)
  {
    drawzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-20, amin*6, ziffbk);
    drawzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-stdzeig, (astd*30) + (amin / 2), ziffbk);
    drawsmallzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-20, asek*6, ziffbk);
  }
  else
  {
    drawzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-20, amin*6, rgbfromega(red));
    drawzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-stdzeig, (astd*30) + (amin / 2), rgbfromega(lightred));
    drawsmallzeiger(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-20, asek*6, rgbfromega(black));
  }
}


/* -------------------------------------------------------
                         uhrscreen

     zeichnet das Ziffernblatt fuer eine analoge Uhren-
     anzeige
   ------------------------------------------------------- */
void uhrscreen(void)
{
  lcd.textcolor= rgbfromega(7);
  lcd.bkcolor= rgbfromega(0);
  ziffbk= rgbfromvalue(0xaa, 0xaa, 0xff);
  lcd.clrscr();

  // Hintergrund zeichnen
  spiro_generate(32, 32, 140,220, 0, rgbfromega(1));
  // Ziffernblatt der analogen Uhr zeichnen
  lcd.fillcircle(xuo+ruhr+9,yuo+ruhr+9,ruhr-8,rgbfromvalue(0x50, 0x50, 0xff));
  lcd.circle(xuo+ruhr+9,yuo+ruhr+9,ruhr-8,rgbfromvalue(0x00, 0x00, 0x80));
  lcd.fillcircle(xuo+ruhr+9,yuo+ruhr+9,ruhr-12, ziffbk);

  drawminskala(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/10)-16, ruhr-16, rgbfromega(9));
  drawstdskala(xuo+ruhr+9, yuo+ruhr+9, ruhr-(ruhr/5)-16, ruhr-16, rgbfromega(0));

}

/* -------------------------------------------------------
                          putdez2
     zeigt die 2 stellige, vorzeichenbehaftete dezimale
     Zahl in val an.

     mode greift bei Zahlen kleiner 10:

     mode        0  : es wird eine fuehrende 0 ausgegeben
                 1  : es wird anstelle einer 0 ein Leer-
                      zeichen ausgegeben
                 2  : eine fuehrende 0 wird unterdrueckt
   ------------------------------------------------------- */
void putdez2(signed char val, uint8_t mode)
{
  char b;
  if (val < 0)
  {
    my_putchar('-');
    val= -val;
  }
  b= val / 10;
  if (b == 0)
  {
    switch(mode)
    {
      case 0 : my_putchar('0'); break;
      case 1 : my_putchar(' '); break;
      default : break;
    }
  }
  else
    my_putchar(b+'0');
  b= val % 10;
  my_putchar(b+'0');
}

/* -------------------------------------------------------
                           getwtag

   Wochentagsberechnung nach Carl-Friedrich-Gauss

   liefert den Wochentag zu einem Datum zurueck:
   0 = Sonntag  .. 6 = Samstag
   ------------------------------------------------------- */
char getwtag(int tag, int monat, int jahr)
{
  int w_tag;

  if (monat < 3)
  {
     monat = monat + 12;
     jahr--;
  }
  w_tag = (tag+2*monat + (3*monat+3)/5 + jahr + jahr/4 - jahr/100 + jahr/400 + 1) % 7 ;
  return w_tag;
}

/* -------------------------------------------------------
                         digitalscreen

     zeigt die Uhrzeit und das Datum digital an
   ------------------------------------------------------- */
void digitalscreen(void)
{
  uint8_t temp;

  temp= getwtag(day, month, year);

  lcd.bkcolor= 0;
//  lcd.textcolor= rgbfromvalue(0x60, 0x60, 0x60);
  lcd.textcolor= rgbfromvalue(0xaa, 0xaa, 0xff);
  lcd.gotoxy(4,14);
  putdez2(stund,1); my_putchar(':');
  putdez2(minut,0); my_putchar('.');
  putdez2(sek,0);

  lcd.gotoxy(1,15);
  printf("%c%c  ",wtag[temp][0],wtag[temp][1]);
  putdez2(day,0); my_putchar('.');
  putdez2(month,0); my_putchar('.');
  printf("20");
  putdez2(year,0);
}

/* -------------------------------------------------------
                      butre_counter

      Zaehlt Variable cnt hoch, wenn Taster rechts
      gedrueckt ist. Bleibt Taster rechts laengere Zeit
      gedrueckt, wird die Zaehlfrequenz erhoeht.
      Der Zaehlwert ist das Rueckgabeergebnis

      outx,outy    : Koordinaten auf dem Display an
                     der das Zaehlen angezeigt wird

      maxcnt       : Maximalwert-1, den die Variable cnt
                     erreichen kann.
      addtocnt     : Offsetwert, der zum Zaehlergebnis
                     hinzuaddiert wird
   ------------------------------------------------------ */
uint8_t butre_counter(uint8_t outx, uint8_t outy, uint8_t maxcnt, uint8_t cnt, uint8_t addtocnt)
{
  uint16_t cntspeed;
  uint8_t scnt;

  delay(50);
  cntspeed= tastlospeed;
  scnt= 0;
  while(is_butre())
  {
    if (scnt> 5) cntspeed= tasthispeed;
    cnt++;
    cnt = cnt % maxcnt;
    lcd.gotoxy(outx,outy);
    putdez2(cnt+addtocnt,2);
    my_putchar(' ');
    delay(cntspeed);
    scnt++;
    if (scnt > 100) scnt= 100;
  }
  delay(50);
  return (cnt+addtocnt);
}
/* -------------------------------------------------------
                      stellen_screen

     Bildschirmmaske zum Uhrzeiteinstellen
   ------------------------------------------------------ */
void stellen_screen(void)
{
  lcd.bkcolor= rgbfromega(7);
  lcd.clrscr();
  lcd.fillrect(0,0,127,24,rgbfromega(8));
  lcd.bkcolor= rgbfromega(8);
  lcd.textcolor= rgbfromega(14);
  lcd.gotoxy(1,1);
  printf("Uhr stellen");
  lcd.textcolor= rgbfromega(1);
  lcd.bkcolor= rgbfromega(7);
  lcd.gotoxy(0,4);
  printf("\n\r Stunde :");
  printf("\n\r Minute :");
  printf("\n\r Jahr   :");
  printf("\n\r Monat  :");
  printf("\n\r Tag    :");
}


/* -------------------------------------------------------
                        timer1_init
                        * 
     initialisierung Timer 1 fuer Interrupt-Intervall
     1 Sekunde
   ------------------------------------------------------- */
void timer1_init(void)
{
  
  noInterrupts(); 
  TCCR1A = 0; 
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = F_CPU / 256;                 // Vergleichsregister 16MHz/256/1Hz
  TCCR1B |= (1 << WGM12);        // CTC mode
  TCCR1B |= (1 << CS12);         // 256 prescaler
  TIMSK1 |= (1 << OCIE1A);       // enable timer compare interrupt
  interrupts();                  // Interrups einschalten
}

/* -------------------------------------------------------
                  Timer1 Compare A Interrupt

   ------------------------------------------------------- */
ISR(TIMER1_COMPA_vect) 
{
  sek++;
  sek= sek % 60;
  if (!(sek))
  {
    minut++;
    minut= minut % 60;
    if (!(minut))
    {
      stund++;
      if (stund==24)
      {
        stund= 0;

        // ab hier Kalenderfunktionen
        if (day< 28)                                            // Monatsende nicht erreicht
        {
          day++;
          return;
        }

        if (month== 2)
        {
          if ((year % 4)== 0)                                   // Schaltjahr
          {
            if (day==28)
            {
              day++;
              return;
            }
            else
            {
              month++;
              day= 1;
              return;
            }
          }
          else
          {
            month++;
            day= 1;
            return;
          }
        }

        if ((month==4) || (month==6) || (month== 9) || (month== 11))     // April, Juni, September, November
        {
          if (day< 30)
          {
            day++;
            return;
          }
          else
          {
            day= 1;
            month++;
            return;
          }
        }

        // ab hier alle Monate mit 31 Tagen

        if ((month== 12) && (day== 31))                                // Feuerwerk, es ist Sylvester
        {
          day= 1;
          month= 1;
          year++;
          return;
        }

        if (day< 31)                                                  // Monatsende nicht erreicht
        {
          day++;
          return;
        }

        day= 1;                                                       // monthsende der Monate mit 31 Tagen
        month++;
        return;

      }
    }
  }
}  


void setup() 
{
  int i;
  
  pinMode(button_rechts, INPUT_PULLUP);
  pinMode(button_links, INPUT_PULLUP);
  
  timer1_init();
  
  lcd.version_g();
  lcd.init(128, 128, 0, _RGB);
  lcd.outmode= 0;  
  
  oldstund= stund; oldminut= minut; oldsek= sek;
  uhrscreen();
  showzeiger(stund, minut, sek, ziffbk, 0);  
  digitalscreen();  
}

void loop() 
{
  uint8_t my_std, my_min, my_year, my_month, my_day;
  uint8_t z_year;
    
  // alte Uhrstellung loeschen
  showzeiger(oldstund, oldminut, oldsek, ziffbk, 1);
  oldsek= sek;
  oldminut= minut;
  oldstund= stund;
  // neue Uhrstellung anzeigen
  showzeiger(stund, minut, sek, ziffbk, 0);    
  digitalscreen();
  while(oldsek== sek)
  {
    if (is_butli())
    {
      stellen_screen();
      my_std= stund; my_min= minut; my_year= year; my_month= month; my_day= day;
      lcd.gotoxy(10,5); putdez2(my_std,2); my_putchar(' ');

      delay(50);
      while(is_butli());
      delay(50);

      while(!is_butli())
      {
        if (is_butre())
        {
          my_std= butre_counter(10,5,24, my_std, 0);
        }
      }
      delay(50);
      while(is_butli());
      delay(50);

      lcd.gotoxy(10,6); putdez2(my_min,2); my_putchar(' ');
      while(!is_butli())
      {
        if (is_butre())
        {
          my_min= butre_counter(10,6,60, my_min, 0);
        }
      }
      delay(50);
      while(is_butli());
      delay(50);

      z_year= my_year;
      lcd.gotoxy(10,7); 
      putdez2(z_year,2);
      while(!is_butli())
      {
        if (is_butre())
        {
          z_year= butre_counter(10,7,100, z_year, 0);
        }
      }
      delay(50);
      while(is_butli());
      delay(50);

      my_year= 2000 + z_year;
      delay(50);
      while(is_butli());
      delay(50);

      lcd.gotoxy(10,8); putdez2(my_month,2); my_putchar(' ');
      while(!is_butli())
      {
        if (is_butre())
        {
          my_month= butre_counter(10,8,12, my_month-1, 1);
        }
      }
      delay(50);
      while(is_butli());
      delay(50);

      lcd.gotoxy(10,9); putdez2(my_day,2); my_putchar(' ');
      while(!is_butli())
      {
        if (is_butre())
        {
          if ((my_month == 2) && ((my_year % 4) == 0))           // Februar im Schaltjahr
            my_day= butre_counter(10,9,29, my_day-1, 1);
          else
          if ((my_month == 2) && ((my_year % 4) > 0))            // Februar im Nichtschaltjahr
            my_day= butre_counter(10,9,28, my_day-1, 1);
          else
          if ((my_month==4) || (my_month==6) || (my_month==9) || (my_month== 11))
            my_day= butre_counter(10,9,30, my_day-1, 1);
          else
            my_day= butre_counter(10,9,31, my_day-1, 1);
        }
      }
      delay(50);
      while(is_butli());
      delay(50);
      my_year -= 2000;

      stund= my_std; minut= my_min; sek= 0; oldsek= 61;
      year= my_year; month= my_month; day= my_day;
      uhrscreen();
      digitalscreen();        
    }
  }
}
