/* ------------------------------------------------------------------
                           kosmos_cp1_v41.h

                        Header zum CP1-Clone

     MCU  :   ATmega328p
     Takt :

     02.01.2021 R. Seelig
   ------------------------------------------------------------------ */

#ifndef in_cp1_config
  #define in_cp1_config

  #include <string.h>
  #include <stdint.h>
  #include <ctype.h>
  #include <avr/io.h>
  #include <avr/interrupt.h>
  #include <util/delay.h>

  #include "avr_gpio.h"
  #include "cp1_uart.h"
  #include "cp1_tm1637.h"
  #include "cp1_adc.h"
  #include "cp1_eeprom_i2c.h"
  #include "cp1_vports.h"
  #include "cp1_pwm.h"
  
  #define  baudrate            38400


  #define  own_ub100mv         43         // eigene Betriebsspannung in 100mV

  #define  opcmax              65
  
  // Mnemonics in der Assemblerliste
  #define  mnemanz             67  

  #define  memsize             256        // in Words
  
  #define  eep_memsize         16384       // in Bytes

  typedef struct kcomp
  {
    uint8_t pc;
    uint8_t a;
    int8_t sp;
    uint8_t b, c, d, e;

    // PSW-Bits  0: zero, 1: carry, 2: less, 3: greater
    uint8_t psw;
    uint8_t addr;
    uint16_t stack[8];
    uint16_t mem[memsize];    
  } kcomp;
  
  extern uint8_t   err;
  
  extern const uint8_t mnemset[mnemanz][13] PROGMEM;
  
  /*  ---------------------------------------------------------
                             Prototypen
      --------------------------------------------------------- */
  // in cp1_cpu.c      
  void cpu_reset(kcomp *vcp1);
  void cpu_run(kcomp *vcp1, uint8_t stepmode);
  
  // in assemble_line.c
  uint8_t assemble_line(uint8_t *src, uint16_t *prgword, kcomp *vcp1);
  void disassemble_prgword(uint8_t *dest, uint16_t prgword);
  
  void str_locase(uint8_t *str);
  void str_delch(uint8_t *str, uint8_t pos);
  uint8_t str_findch(uint8_t *str, uint8_t ch);
  void str_delallch(uint8_t *str, uint8_t ch);
  void str_replacech(uint8_t *str, uint8_t dest, uint8_t src);
  void str_insert(uint8_t *dest, uint8_t *src, uint8_t pos);  
  
  // in kosmos_cp1_v41.c
  extern uint8_t cp1_delay(uint32_t dtime);
  extern uint8_t softw_intr(kcomp *vcp1, int data);
  extern void debugprint(uint16_t val);
  

#endif
