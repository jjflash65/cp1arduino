/* -----------------------------------------------------
                      cp1_hx1838.h

     Header fuer den Betrieb eines HX1838 IR-Receiver
     am CP1+ Board.

     Aufgrund der Portbelegung, die nicht zum HX1838
     kompatibel ist, muss ein Portpin als 0 geschaltet
     werden um dem HX1838 eine Masseleitung zur
     Verfuegung zu stellen (nicht ganz schoen,
     funktioniert aber)

     Modul verwendet Timer2

     Board : CP1+
     F_CPU : 8 MHz intern

     28.01.2021        R. Seelig

  ------------------------------------------------------ */

#ifndef in_hx1838
  #define in_hx1838

  #include <avr/io.h>
  #include <avr/interrupt.h>
  
  #include "Arduino.h"
  #include "cp1_gpio.h"

  enum { pscale1 = 1, pscale8, pscale32, pscale64, pscale128,  pscale256, pscale1024 };

  extern volatile uint16_t  ir_code;                        // Code des letzten eingegangenen 16-Bit Wertes
  extern volatile uint8_t   ir_newflag;                     // zeigt an, ob ein neuer Wert eingegangen ist
  
  // PD7 (P1.1 des CP1 Boards) als IR-Data-Input (PCINT23)
  #define ir_input_init()    { P1_1_input_init(); P1_1_set(); }
  #define is_irin()          is_P1_1()

  // PD6 (P1.0 des CP1 Boards) als GND-Anschluss
  #define ir_gnd_init()      { P1_0_output_init(); P1_0_clr(); }  

  #define IR_ISR_vect        PCINT2_vect
  #define IR_PCINT           PCINT23
  #define IR_PCMSK           PCMSK2
  #define IR_PCIE            PCIE2

  #define tim2_getvalue()    TCNT2
  #define tim2_clr()         TCNT2= 0;

/* -------------------------------------------------
                        Prototyp
   ------------------------------------------------- */

  void hx1838_init(void);


#endif
