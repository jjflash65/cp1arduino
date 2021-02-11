/* -----------------------------------------------------
                     cp1_ir_test.ino

     Versuche mit dem HX1838 IR-Receiver

     Board : CP1+
     F_CPU : 8 MHz intern

     28.01.2021        R. Seelig

  ------------------------------------------------------ */


#include "cp1_oled.h"
#include "cp1_printf.h"
#include "cp1_hx1838.h"


oled  tft(2);

/* --------------------------------------------------
                       my_putchar
                       
     wird zwingend "cp1_printf.cpp" benoetigt! 
     Ueber diese Funktion erfolgen die Ausgaben von
     printf                       
   -------------------------------------------------- */
void my_putchar(char ch)
{
  tft.directputchar(ch);
}

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{
  tft.fb_clear();
  tft.clrscr();
  
  hx1838_init();  
}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{ 
  tft.fb_clear();
  tft.clrscr();

  ir_code= 0;
  tft.gotoxy(0,3);
  printf("IR-Code: %x", ir_code);
  ir_newflag= 0;
  while(1)
  {
    if (ir_newflag)
    {
      tft.gotoxy(0,3);
      printf("IR-Code: %x", ir_code);
      ir_newflag= 0;
    }
  }

}
