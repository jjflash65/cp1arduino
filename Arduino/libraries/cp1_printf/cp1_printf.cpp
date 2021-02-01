/* ----------------------------------------------------------------------
                            cp1_printf.cpp

     Implementierung einer sehr minimalistischen (und im Funktionsumfang
     eingeschraenkten) Version von printf.

     Benoetigt im Arduino-Sketch eine Funktion:

                   void my_putchar(char c);


     22.04.2016   R. Seelig

     Danke hier an www.mikrocontroller.net (Dr.Sommer, beric)

     letzte Aenderung:
     ---------------------------------------------------

     Angabe der Nachkommastellen bei Verwendung des %k Platzhalters
     
     (Implementation der globalen Variable printfkomma und veraenderte
     < putint > Funktion

     29.08.2018    R. Seelig

   --------------------------------------------------------------------- */

#include "cp1_printf.h"

char printfkomma = 1;


/* ------------------------------------------------------------
                            putint
                            
     gibt einen Integer dezimal aus. Ist Uebergabe
     "komma" != 0 wird ein "Kommapunkt" mit ausgegeben.

     Bsp.: 12345 wird als 123.45 ausgegeben.
     (ermoeglicht Pseudofloatausgaben im Bereich)
   ------------------------------------------------------------ */
void putint(int32_t i, char komma)
{
  typedef enum boolean { FALSE, TRUE }bool_t;

  static uint32_t zz[]  = { 10000000, 1000000, 100000, 10000, 1000, 100, 10 };
  bool_t not_first = FALSE;

  uint8_t zi;
  uint32_t z, b;

  komma= 8-komma;

  if (!i)
  {
    my_putchar('0');
  }
  else
  {
    if(i < 0)
    {
      my_putchar('-');
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
        my_putchar('0' + b);
        not_first = TRUE;
      }

      if (zi+1 == komma)
      {
        if (!not_first) my_putchar('0');
        my_putchar('.');
        not_first= TRUE;
      }

      i -= z;
    }
    my_putchar('0' + i);
  }
}

/* ------------------------------------------------------------
                       hexnibbleout
                       
     gibt die unteren 4 Bits eines chars als Hexaziffer aus.
     Eine Pruefung ob die oberen vier Bits geloescht sind
     erfolgt NICHT !
  -------------------------------------------------------------  */
void hexnibbleout(uint8_t b)
{
  if (b< 10) b+= '0'; else b+= 55;
  my_putchar(b);
}

/* ------------------------------------------------------------
                            puthex
                            
     gibt einen Integer hexadezimal aus. Ist die auszugebende
     Zahl >= 0xff erfolgt die Ausgabe 2-stellig, ist sie
     groesser erfolgt die Ausgabe 4-stellig.

     Ist out16 gesetzt, erfolgt ausgabe immer 4 stellig
   ------------------------------------------------------------ */
void puthex(uint16_t h, char out16)
{
  uint8_t b;

  if ((h> 0xff) || out16)                    // 16 Bit-Wert
  {
    b= (h >> 12);
    hexnibbleout(b);
    b= (h >> 8) & 0x0f;
    hexnibbleout(b);
  }
  b= h;
  b= (h >> 4) & 0x0f;
  hexnibbleout(b);
  b= h & 0x0f;
  hexnibbleout(b);
}


void putstring(char *p)
{
  while(*p)
  {
    my_putchar( *p );
    p++;
  }
}


/* ------------------------------------------------------------
                           own_printf
                           
     alternativer Ersatz fuer printf.

     Aufruf:

         my_printf("Ergebnis= %d",zahl);

     Platzhalterfunktionen:

        %s     : Ausgabe Textstring
        %d     : dezimale Ausgabe (16-Bit)
        %l     : dezimale Ausgabe (32-Bit)
        %x     : hexadezimale Ausgabe (16-Bit)
                 ist Wert > 0xff erfolgt 4-stellige
                 Ausgabe
                 is Wert <= 0xff erfolgt 2-stellige
                 Ausgabe
        %k     : 32-Bit Integerausgabe als Pseudokommazahl
                 12345 wird als 123.45 ausgegeben (bei
                 printfkomma = 2)
        %c     : Ausgabe als Asciizeichen

   ------------------------------------------------------------ */

void own_printf(const uint8_t *s,...)
{
  int32_t   arg1;
  int16_t   arg16;
  uint32_t  xarg1;
  char     *arg2;
  char      ch;
  va_list   ap;
  uint8_t   token;

  va_start(ap,s);
  do
  {
    ch= pgm_read_byte(s);
    if(ch== 0) return;

    if(ch=='%')            // Platzhalterzeichen
    {
      s++;
      token= pgm_read_byte(s);
      switch(token)
      {
        case 'l':          // dezimale Ausgabe (32-Bit)
        {
          arg1= va_arg(ap,int32_t);
          putint(arg1,0);
          break;
        }
        case 'd':          // dezimale Ausgabe (16-Bit)
        {
          arg16= va_arg(ap,int16_t);
          putint(arg16,0);
          break;
        }
        case 'x':          // hexadezimale Ausgabe
        {
          xarg1= va_arg(ap,uint16_t);
          puthex(xarg1, 0);
          break;
        }
        case 'k':
        {
          arg1= va_arg(ap,int32_t);
          putint(arg1,printfkomma);     // Integerausgabe mit Komma: 12896 zeigt 12.896 an
          break;
        }
        case 'c':          // Zeichenausgabe
        {
          arg1= va_arg(ap,int);
          my_putchar(arg1);
          break;
        }
	case '%':
        {
          my_putchar(token);
          break;
        }
        case 's':
        {
          arg2= va_arg(ap,char *);
          putstring(arg2);
          break;
        }
      }
    }
    else
    {
      my_putchar(ch);
    }
    s++;
  }while (ch != '\0');
}
