
/* ----------------------------------------------------------------------
                            cp1_printf_demo.ino

     Demof fuer die Verwendung einer sehr minimalistischen (und im 
     Funktionsumfang eingeschraenkten) Version von printf.

     Benoetigt im Arduino-Sketch eine Funktion:

                   void my_putchar(char c);
                   
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

     28.01.2021   R. Seelig
   --------------------------------------------------------------------- */
   
#include <string.h>
#include "cp1_printf.h"


/* --------------------------------------------------
                       my_putchar
                       
     wird zwingend "cp1_printf.cpp" benoetigt! 
     Ueber diese Funktion erfolgen die Ausgaben von
     printf                       
   -------------------------------------------------- */
void my_putchar(char ch)
{
  Serial.write(ch);
}

uint16_t counter = 0;

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{
  char    str[40];
  float   f;
  int     zahl;
  uint8_t zahl_8;
  int32_t zahl_32;
  
  Serial.begin(38400);
  printf("\n\r --------------------------------");
  printf("\n\r   Demo - printf mit Arduino");
  printf("\n\r --------------------------------\n\r");
  
  strcpy(str,"CP1+ Board und Arduino");
  printf("\n\r Textausgabe         : %s", str);
  zahl= 3289;
  printf("\n\r Zahlenausgabe 16-Bit: %dd = %xh", zahl, zahl);
  zahl_8= 43;
  printf("\n\r Zahlenausgabe 8-Bit : %dd = %xh", zahl_8, zahl_8);
  f= 243.0 / 12.8;
  f= f*1000;
  zahl= f;
  printfkomma= 3;
  printf("\n\r Kommaausgabe 16-Bit : 243.0 / 12.8 = %k", (uint32_t)zahl);
  f= 94.83 * 73.42;
  f= f * 1000;
  zahl_32= f;
  printf("\n\r Kommaausgabe 32-Bit : 94.83 / 73.42 = %k", zahl_32);
  delay(2000);
}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  printf("\n\r Counter: %d     ",counter);
  counter++;
  delay(500);
}
