/* ----------------------------------------------------------
                            pwm.c

     initialisiert Timer fuer die Verwendung als
     PWM-Generatoren.

     Aktivierung der einzelnen PWM in pwm.h.

         PWM Timer0 PWM Pin = PD5
         PWM Timer1 PWM Pin = PB1
         PWM Timer2 PWM Pin = PD3

     MCU     : ATmegaxx8
     F_CPU   :

     06.01.2021    R. Seelig
   ---------------------------------------------------------- */

#ifndef in_pwm
  #define in_pwm

  #include <stdlib.h>
  #include <avr/io.h>
  #include "avr_gpio.h"

  #define  pwmt0_enable             0   // 0 : verfuegbar, 1 : nicht verfuegbar
  #define  pwmt1_enable             1   // 0 : verfuegbar, 1 : nicht verfuegbar
  #define  pwmt2_enable             1   // 0 : verfuegbar, 1 : nicht verfuegbar


  #define pwmt0_stop()      { TCCR0B = 0; TCCR0A = 0; }
  #define pwmt1_stop()      { TCCR1B = 0; TCCR1A = 0; }
  #define pwmt2_stop()      { TCCR2B = 0; TCCR2A = 0; }

  enum { t01_div1 = 1, t01_div8, t01_div64, t01_div256, t01_div1024 };
  enum { t2_div1 = 1, t2_div8, t2_div32, t2_div64, t2_div128, t2_div256, t2_div1024 };

/* ----------------------------------------------------------
                          Prototypen
   ---------------------------------------------------------- */

   #if (pwmt0_enable == 1)
     uint16_t pwmt0_setfreq(uint16_t freq, uint16_t duty);
     void pwmt0_init(uint8_t prescale, uint8_t tg, uint8_t tp);
   #endif

   #if (pwmt1_enable == 1)
     uint16_t pwmt1_setfreq(uint16_t freq, uint16_t duty);
     void pwmt1_init(uint8_t prescale, uint16_t tg, uint16_t tp);
   #endif

   #if (pwmt2_enable == 1)
     uint16_t pwmt2_setfreq(uint16_t freq, uint16_t duty);
     void pwmt2_init(uint8_t prescale, uint8_t tg, uint8_t tp);
   #endif

#endif

