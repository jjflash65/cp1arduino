/*  ---------------------------------------------------------
                          charlie20.cpp

     Softwaremodul fuer 20 "charliegeplexte" LED's      

     Beim Charlieplexing sind jeweils 2 antiparallel geschaltete Leuchtdioden
     an 2 GPIO-Leitungen angeschlossen. Hieraus ergeben sich 10 Paare zu jeweils
     2 LEDs.

     Bsp.:

       A  o------+-----,         C o------+-----,
                 |     |                  |     |
                ---   ---                ---   ---
              A \ /   / \ B            C \ /   / \ D
              B ---   --- A            D ---   --- C
                 |     |                  |     |
       B  o------+-----'         D o------+-----'
 

       A  B  B  C  C  D  D  E  A  C  C  E  D  B  A  D  A  E  E  B   Linenkombination
       B  A  C  B  D  C  E  D  C  A  E  C  B  D  D  A  E  A  B  E
       ----------------------------------------------------------
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19  LED-Nummern
      
      08.02.2021    R. Seelig                             
    --------------------------------------------------------- */
    
#include "charlie20.h"
    
/* ------------------------------------------------------
                     charlie20:charlie20
                      Konstruktor

     initialisiert die I/O Leitungen des Charlieplexings
     und setzt alle Leitungen auf 1.
     
     Initialisiert Timer2 fuer Interrupt
   ------------------------------------------------------ */
charlie20::charlie20(uint8_t leda, uint8_t ledb, uint8_t ledc, uint8_t ledd, uint8_t lede)
{
  LED_A= leda; LED_B= ledb; LED_C= ledc; LED_D= ledd; LED_E= lede;
  
  allinput();

  charlieA_set();
  charlieB_set();
  charlieC_set();
  charlieD_set();
  charlieE_set();
}


/* ------------------------------------------------------
                   charlie20::allinput
                   
     schaltet alle am Charlieplexing beteiligten I/O
     Leitungen als Eingang
   ------------------------------------------------------ */
void charlie20::allinput(void)
{
  charlieA_input();
  charlieB_input();
  charlieC_input();
  charlieD_input();
  charlieE_input();
}

/* ------------------------------------------------------
                    charlie20::lineset

     schaltet eine einzelne LED an.

     nr:   Nummer der einzuschaltenden LED (0..19)
           nr== 20 => alle LED's werden ausgeschaltet
   ------------------------------------------------------ */
void charlie20::lineset(char nr)
{
  uint8_t   bl, bh;
  uint16_t  b;

  allinput();
  if (nr== 20) return;                               // 20 = alle LED aus

  b= cplex[nr];
  
  bh= b >> 8;
  bl= b & 0xff;

  if (bh & 0x01)
  {
    charlieA_output();
    if (bl & 0x01) { charlieA_set(); } else { charlieA_clr(); }
  }
  if (bh & 0x02)
  {
    charlieB_output();
    if (bl & 0x02) { charlieB_set(); } else { charlieB_clr(); }
  }
  if (bh & 0x04)
  {
    charlieC_output();
    if (bl & 0x04) { charlieC_set(); } else { charlieC_clr(); }
  }
  if (bh & 0x08)
  {
    charlieD_output();
    if (bl & 0x08) { charlieD_set(); } else { charlieD_clr(); }
  }
  if (bh & 0x10)
  {
    charlieE_output();
    if (bl & 0x10) { charlieE_set(); } else { charlieE_clr(); }
  }
}

/* ------------------------------------------------------
                     charlie20::plexing

     Hier werden 20 Bits des Buffers charlie20.buffer
     seriell nacheinander dargestellt. Aufruf dieser
     Methode sollte ueber einen Timerinterrupt erfolgen
   ------------------------------------------------------ */
void charlie20::plexing(void)
{
  static volatile uint8_t cpx_cnt = 0;

  charlie20::lineset(20);                               // alle LED's us
  if (charlie20::buffer & ((1ul << cpx_cnt)) )          // Bit in charlie20.buffer gesetzt ?
  {
    charlie20::lineset(cpx_cnt);                        // dann diese LED einschalten
  }
  cpx_cnt++;
  cpx_cnt= cpx_cnt % 20;
}
    
