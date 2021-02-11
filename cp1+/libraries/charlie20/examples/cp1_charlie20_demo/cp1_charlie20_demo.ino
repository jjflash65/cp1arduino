/*  -----------------------------------------------------------------------
                          charlie20_demo.ino

      Ansteuerung von 20 LED's mittels Charlieplexing. Hierfuer benoetigte 
      I/O Pins : 5

      Da immer nur eine einzelne LED leuchtet, wird fuer diese Library ein 
      Timerintervall benoetigt, der einzelne Leuchtdiodenlinien im 
      Zeitmultiplex vornimmt. In diesem Demo wird hierfuer Timer2 benutzt

      29.01.2021    R. Seelig                             
    ----------------------------------------------------------------------- */

#include <avr/interrupt.h>
#include "charlie20.h"


#define speed1    50
#define speed2    20

uint32_t  counter = 1;
uint32_t  cx;


// Zurordnung der Leuchtdiodenlinien zu den GPIO-Pins des
// Arduino

// LED-Line               :  A     B     C     D     E
// Benutzte CP1 Pins      : P1.7  P1.6  P1.5  P1.4  P1.3

charlie20 cpx (P1_7,  P1_6,  P1_5,  P1_4,  P1_3);


enum { pscale1 = 1, pscale8, pscale32, pscale64, pscale128,  pscale256, pscale1024 };
/* -------------------------------------------------------
                      timer2_init

     Initialisiert den Timer2 (8-Bit) als Comparetimer.

     Uebergabe:
        prescale  : Teilerfaktor von F_CPU mit der der
                    Timer getaktet wird.
        compvalue : Vergleichswert, bei dem ein Interrupt
                    ausgeloest wird

     Bsp.:
        F_CPU = 8 MHz, prescaler 1024 = 7812,5 Hz = 128uS Taktzeit Zaehler
        OCR0A = 16: nach 128uS * 16 = 2,048 mS wird ein Interupt ausgeloest (bei
                Startwert TCNT0 = 0).
   ------------------------------------------------------- */
void timer2_init(uint8_t prescale, uint8_t compvalue)
{
  TCCR2B = prescale;
  OCR2A = compvalue;
  TCNT2 = 0;                   // Zaehler zuruecksetzen

  TIMSK2 = 1 << OCIE2A;        // if OCR0A == TCNT0 dann Interrupt
  sei();                       // Interrupts grundsaetzlich an

}

/* ------------------------------------------------------
                      TIMER2_COMPA_vect
     ISR fuer Timer0 Compare A match.

     Ruft das Charlieplexing auf
   ------------------------------------------------------ */
ISR (TIMER2_COMPA_vect)
{
  cpx.plexing();                         // ruft das Charlieplexing auf
  TCNT2= 0;                              // Zaehlregister zuruecksetzen
}

/*  ---------------------------------------------------------
                             setup
               wird einmal beim Start durchgefuehrt
    --------------------------------------------------------- */
void setup()
{
  timer2_init(pscale256, 16);           // F_CPU/256 compare 16 loest bei 16MHz alle 0.256ms Interrupt aus   

  // die Variable >buffer< des Objekts cpx nimmt das Leuchtmuster auf,
  // welches angezeigt werden soll
  for (int i= 0; i< 4; i++)
  {
    cpx.buffer= 0xaaaaa;
    delay(300);
    cpx.buffer= 0x55555;
    delay(300);
  }  
}

/*  ---------------------------------------------------------
                             loop
      wird nach setup in einer Endlosschleife durchgefuehrt                             
    --------------------------------------------------------- */
void loop() 
{
  do
  {
    cpx.buffer= counter;
    counter= counter << 1ul;
    delay(speed1);
  } while (counter< 0xfffff);

  counter= counter >> 2;

  do
  {
    cpx.buffer= counter;
    counter= counter >> 1ul;
    delay(speed1);
  } while (counter> 0);


  counter= 1;
  cx= 1;

  do
  {
    cx= cx << 1ul;
    cpx.buffer += cx;
    counter++;
    delay(speed2);
  } while(counter < 20);

  counter= 1;
  cx= 1;

  do
  {
    cpx.buffer -= cx;
    cx= cx << 1ul;
    counter++;
    delay(speed2);
  } while(counter < 20);

  counter= 2;
}
