/* -------------------------------------------------
                        cp1_uart.h

     Header fuer Softwaremodul zur Verwendung der
     seriellen Schnittstelle von AVR ATmega
     Controller

     Compiler: AVR-GCC 4.3.2

     MCU:
           - ATmega8
           - ATmega88 .. ATmega328

     02.01.2021        R. Seelig
  -------------------------------------------------- */


#ifndef in_uart
  #define in_uart

  #define  echo_enable           0       // 0 : es wird bei einer Eingabe kein Echo gesendet
                                         // 1 : Echo wird gesendet
  #include <stdio.h>
  #include <avr/pgmspace.h>
  #include <stdint.h>
  #include <stdlib.h>

  #include "kosmos_cp1_v43.h"

  void uart_init(uint32_t baud);
  void uart_deinit(void);
  void uart_putchar(uint8_t ch);
  uint8_t uart_getchar(void);
  uint8_t uart_ischar(void);
  void uart_crlf();
  void uart_putramstring(uint8_t *p);
  void uart_putromstring(const uint8_t *dataPtr);
   
  void uart_clr(void);
  uint8_t uart_readstr(uint8_t *str, uint8_t chanz);
  uint8_t uart_readu8int(uint8_t *value);
  void uart_uint16out(uint16_t value, uint8_t dp);
  void uart_uint8out(uint8_t value);
  

  #define prints(tx)            uart_putromstring(PSTR(tx))         // Benutzung: prints("Hallo Welt\n\r");

#endif
