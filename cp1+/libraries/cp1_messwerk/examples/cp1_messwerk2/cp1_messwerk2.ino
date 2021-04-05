/* -----------------------------------------------------
                     cp1_messwerk2.ino

     Demoprogramm fuer ein graphisches analoges Messwerk
     auf einem 128x128 Farbdisplay

     Board : CP1+
     F_CPU : 8 MHz intern

     24.03.2021        R. Seelig

  ------------------------------------------------------ */
  

#include "st7735.h"
#include "cp1_printf.h"
#include "cp1_messwerk.h"

/*
 Verdrahtung Display:
   da Hardware-SPI verwendet wird sind CLK und DIO des Displays nicht
   waehlbar.
   
   Anschluss CLK:  Arduino D13  =  AVR-PB5 = P1_7-CP1
   Anschluss DIO:  Arduino D11  =  AVR-PB3 = P1_5 CP1
   
   Anschluesse fuer RST, CE, DC sind frei waehlbar beim Erstellen des
   Objects in der Reihenfolge.
   
      Arduino    AVR     CP1
   
   rst=   8   =  PB0  =  P1_2
   dc=    9   =  PB1  =  P1_3
   ce=   10   =  PB2  =  P1_4
*/
st7735 lcd(P1_2, P1_3, P1_4);  // Displayobjekt erzeugen

instrumentA  mw(0,0);         // Messwerkobjekt erzeugen


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

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{
  analogReference(DEFAULT);

  lcd.ofsmode(-32);
  lcd.version_g(); 
  lcd.init(128, 128, 0, _RGB);
  lcd.outmode= 0;    
  lcd.clrscr();  
  
  mw.setcolors(rgbfromvalue(0x80, 0x80, 0), rgbfromega(15), rgbfromega(8), rgbfromega(0), rgbfromega(1));   
  mw.drawscreen("0", "2.5", "5", "Volt");

}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{ 
  uint16_t adcw;

  adcw= analogRead(P2_2);
  mw.drawzeiger(adcw, 1, "2.5", "Volt");
  mw.drawdigital(1,9, adcw, 500, 0xffff);
  delay(100);
  mw.drawzeiger(adcw, 0, "2.5", "Volt");
}
