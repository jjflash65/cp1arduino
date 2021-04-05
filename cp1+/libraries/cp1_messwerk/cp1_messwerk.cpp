/* -----------------------------------------------------
                     cp1_messwerk.c

     stellt ein graphisches analoges Messwerk in den
     Dimensionen 128x64 zur Verfügung

     Board : CP1+
     F_CPU : 8 MHz intern

     24.03.2021        R. Seelig
  ------------------------------------------------------ */
  

#include "cp1_messwerk.h"
  
/* ------------------------------------------------------------
                      instrumentA:instrumentA
                          Konstruktor

     Parameter:
       x,y : Linke obere Ecke, an der das Messinstrument
             ausgegeben werden soll
   ------------------------------------------------------------ */
instrumentA::instrumentA(uint16_t x, uint16_t y)
{
  xofs= x;
  yofs= y;
}

/* ------------------------------------------------------------
                       instrumentA::dtoa
                            
     konvertiert eine dezimale Zahl in einen String. Ist das
     Argument fuer "komma" != 0 wird ein "Kommapunkt" mit 
     ausgegeben.

     Bsp.: printfkomma== 2, dann wird 12345 als 123.45 
     ausgegeben.
     (ermoeglicht Pseudofloatausgaben im Bereich)
   ------------------------------------------------------------ */
void instrumentA::dtoa(uint8_t *dstr, int32_t i, char komma)
{
  typedef enum boolean { FALSE, TRUE }bool_t;

  static uint32_t zz[]  = { 10000000, 1000000, 100000, 10000, 1000, 100, 10 };
  bool_t not_first = FALSE;

  uint8_t zi;
  uint32_t z, b;

  komma= 8-komma;

  if (!i)
  {
    *dstr = '0';
    dstr++;
  }
  else
  {
    if(i < 0)
    {
      *dstr = '-';
      dstr++;
      i = -i;
    }

    for(zi = 0; zi < 7; zi++)
    {
      z = 0;
      b = 0;

      while(z + zz[zi] <= i)
      {
        b++;
        z += zz[zi];
      }

      if(b || not_first)
      {
        *dstr = ('0' + b);
        dstr++;
        not_first = TRUE;
      }

      if (zi+1 == komma)
      {
        if (!not_first) 
        {
          *dstr= '0';
          dstr++;
        }
        *dstr= '.';
        dstr++;
        not_first= TRUE;
      }

      i -= z;
    }
    *dstr= ('0' + i);
    dstr++;
  }
  *dstr= 0;
}

/*  ---------------------------------------------------------
                       instrumentA::kr_getxy
                             
      ermittelt aus der in der Struktur mw angegebenen Werte
      2 Koordinatenpaare, die die Anfangs- und Endkoordinaten
      einer Linie sind.
    --------------------------------------------------------- */
krpos instrumentA::kr_getxy(mw_args mw, uint16_t wi)
{
  krpos k;

  // Berechnung Koordinaten Zeigerende
  k.x1= mw.xm - (cosD(wi) * mw.rad);
  k.y1= mw.ym - (sinD(wi) * mw.rad);

  // Berechnung Koordinaten Zeigeranfang
  k.x2= mw.xm - (cosD(wi) * mw.zl);
  k.y2= mw.ym - (sinD(wi) * mw.zl);
  return k;
}

/*  ---------------------------------------------------------
                      instrumentA::mw_setargs
                             
      setzt die Parameter fuer Radius, Mittelpunkt,
      Mittelpunktposition und Zeigerlaenge des analogen
      Instruments.
    --------------------------------------------------------- */
void instrumentA::mw_setargs(uint16_t xm, uint16_t ym, uint16_t rad, uint16_t zl)
{
  mw.xm= xm;
  mw.ym= ym;
  mw.rad= rad;
  mw.zl= zl;
}

/*  ---------------------------------------------------------
                    instrumentA::setcolors
                             
      setzt die Parameter fuer Radius, Mittelpunkt,
      Mittelpunktposition und Zeigerlaenge des analogen
      Instruments.
    --------------------------------------------------------- */
void instrumentA::setcolors(uint16_t frame, uint16_t bk, uint16_t skala, uint16_t zeiger, uint16_t text)
{
  mw_col.framecol= frame;
  mw_col.bkcol= bk;
  mw_col.skalacol= skala;
  mw_col.zeigercol= zeiger;
  mw_col.textcol= text;
}

/*  ---------------------------------------------------------
                    instrumentA::drawscreen
      zeichnet die Visualisierung des Messinstruments
      
      Parameter:
      *t_lo  : Beschriftungstext fuer linken Anschlag
      *t_mid : Beschriftungstext fuer mittlere Anzeige
      *t_hi  : Beschriftungstext fuer rechten Anschlag
      *title : Beschriftungstext in der Mitte des Instruments
    --------------------------------------------------------- */
void instrumentA::drawscreen(char *t_lo, char *t_mid, char *t_hi, char *title)
{
  krpos    k;
  uint16_t i, wi;
  uint8_t  l;
  
  // Parameter Skala des Messwerks: xm, ym, rad, zl
  mw_setargs(64, 87, 80, 76);

  lcd.fillrect(xofs, yofs, xofs+127, yofs+63, mw_col.bkcol);
  
  lcd.rectangle(xofs,yofs, xofs+127, yofs+63, mw_col.framecol);

  // Skaleneinteilung zeichnen
  for (i= 45; i< 136; i+= 9)
  {    
    k= kr_getxy(mw, i);
    lcd.line(xofs+k.x1, yofs+k.y1, xofs+k.x2, yofs+k.y2, mw_col.zeigercol);
  }
  lcd.textcolor= mw_col.textcol;
  lcd.setfont(FNT5x7);
  lcd.outtextxy(xofs+6,yofs+40, t_lo);
  lcd.outtextxy(xofs+58,yofs+14, t_mid);
  lcd.outtextxy(xofs+112,yofs+40, t_hi);
  l= strlen(title);
  lcd.textcolor= mw_col.textcol;
  lcd.outtextxy(xofs+64 - (l * 3), yofs+28, title);

  mw_setargs(64, 87, 74, 32);
  for (wi= 45; wi< 136; wi++)
  {
    k= kr_getxy(mw, wi);  
    lcd.line(xofs+ mw.xm, yofs+ 62, xofs+k.x2, yofs+k.y2-1, mw_col.framecol);
    lcd.line(xofs+1+ mw.xm, yofs+ 62, xofs+1+k.x2, yofs+k.y2-1, mw_col.framecol);
    lcd.line(xofs+2+ mw.xm, yofs+ 62, xofs+1+k.x2, yofs+k.y2-1, mw_col.framecol);
  }  
  mw_setargs(64, 87, 74, 35);
}


/*  ---------------------------------------------------------
                    instrumentA::drawzeiger
      zeichnet den Zeiger auf einen Messwert der Skala
      
      Parameter:
        adcw     : Anzuzeigender 10-Bit Messwert
        drawmode : 0=> Zeiger wird geloescht, 1=> Zeiger wird gezeichnet
        *title   : Beschriftungstext in der Mitte des Instruments
    --------------------------------------------------------- */
void instrumentA::drawzeiger(uint16_t adcw, uint8_t drawmode, char *t_mid, char *title)
{
  krpos    k;
  uint16_t i, wi;
  uint16_t l;
  uint16_t col;
 
  l= strlen(title);

  if (drawmode)
  {
    col= mw_col.zeigercol;
  }
  else
  {
    col= mw_col.bkcol;
  }

  // Umrechnen 10-Bit ADC-Wert in einen Winkel
  wi = (uint16_t)((float)(adcw) / (1024.0 / 90.0));
  wi += 45;
  k= kr_getxy(mw, wi);
  
  // Messinstrumentenzeiger zeichnen / loeschen  
  lcd.line(xofs+k.x1, yofs+k.y1, xofs+k.x2, yofs+k.y2, col);
//  lcd.line(xofs+k.x1+1, yofs+k.y1+1, xofs+k.x2+1, yofs+k.y2+1, col); 
  
  if (drawmode == 0)
  {
    lcd.textcolor= mw_col.textcol;    
    lcd.setfont(FNT5x7);    
    lcd.outtextxy(xofs+64 - (l * 3), yofs+28, title);
    lcd.outtextxy(xofs+58,yofs+14, t_mid);    
  }

}

/*  ---------------------------------------------------------
                 instrumentA::drawdigital
                 
    gibt einen 10-Bit ADC Wert als umgerechneten Wert auf
    dem Display aus.
    
    Parameter:
      x,y    : Ausgabeposition auf dem Display
      adcw   : 10-Bitwert
      maxv   : der Wert * 100, der der maximalen Aufloesung
               des ADC's entspricht.
               Bsp.: maxv = 20000;
                     adcw = 512;
                     
                     Ausgabe: 10.00;
    --------------------------------------------------------- */

void instrumentA::drawdigital(uint8_t x, uint8_t y, uint32_t adcw, uint32_t maxv, uint16_t col)
{
  uint32_t messw;
  
  lcd.textcolor= mw_col.textcol;
  // umrechnen des ADC-Wertes in analogen Messwert

  messw= (adcw * maxv / 1024l);

  lcd.textcolor= col; 
  lcd.setfont(FNT8x8);   
  
  printfkomma= 2;
  lcd.gotoxy(x,y); 
  printf("%k ", messw);
}
  
