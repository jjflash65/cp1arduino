/* ----------------------------------------------------------------
                              vports.h

     Header fuer 2 "virtuelle Ports"

     Zusammenfassen beliebiger Portpins zu 2 Pseudoports, die
     unter P1_x und P2_x angesprochen werden koennen. Geschwindig-
     keit der Schaltgeschwindigkeit ist dadurch herabgesetzt, ist
     aber fuer Programme bei denen ein kompletter 8-Bit-Port
     verwendet werden soll oder muss und bei denen die Geschwindig-
     keit eine untergeordnete Rolle spielt, hilfreich und/oder
     sinnvoll.

     MCU: 28 pol. AVR ATmega

     Zuordnung realer Pins zu den virtuellen Ports
                                                    M     M
                                                    O     I     S
                                                    S     S     C
                                                    I     O     K

     virtual P1  :  P1_0  P1_1  P1_2  P1_3  P1_4  P1_5  P1_6  P1_7
     AVR-Portpins:   PD6   PD7   PB0   PB1   PB2   PB3   PB4   PB5
     Adruino     :    6     7     8     9    10    11    12    13

     virtual P2  :  P2_0  P2_1  P2_2  P2_3  P2_4  P2_5  P2_6  P2_7
     AVR-Portpins:   PC0   PC1   PC2   PC3   PD4   PD3   PD2   PD0
     Arduino     :   A0    A1    A2    A3     4     3     2     0

     25.12.2020 R. Seelig
   ---------------------------------------------------------------- */

#ifndef in_vports
  #define in_vports

  #include <util/delay.h>
  #include <avr/io.h>

  #include "avr_gpio.h"

  /* ----------------------------------------------------------
                          Prototypen
     ---------------------------------------------------------- */

  void p1_config(uint8_t dirbyte);
  void p1_bytewrite(uint8_t value);
  void p1_bitwrite(uint8_t bitnr, uint8_t value);
  uint8_t p1_byteread(void);
  uint8_t p1_bitread(uint8_t bitnr);

  void p2_config(uint8_t dirbyte);
  void p2_bytewrite(uint8_t value);
  void p2_bitwrite(uint8_t bitnr, uint8_t value);
  uint8_t p2_byteread(void);
  uint8_t p2_bitread(uint8_t bitnr);


  /* ----------------------------------------------------------
                 Macros fuer virtual Port 1
     ---------------------------------------------------------- */

  #define p1_0_out_init()    PD6_output_init()
  #define p1_0_set()         PD6_set()
  #define p1_0_clr()         PD6_clr()
  #define p1_0_in_init()     PD6_input_init()
  #define is_p1_0()          is_PD6()

  #define p1_1_out_init()    PD7_output_init()
  #define p1_1_set()         PD7_set()
  #define p1_1_clr()         PD7_clr()
  #define p1_1_in_init()     PD7_input_init()
  #define is_p1_1()          is_PD7()

  #define p1_2_out_init()    PB0_output_init()
  #define p1_2_set()         PB0_set()
  #define p1_2_clr()         PB0_clr()
  #define p1_2_in_init()     PB0_input_init()
  #define is_p1_2()          is_PB0()

  #define p1_3_out_init()    PB1_output_init()
  #define p1_3_set()         PB1_set()
  #define p1_3_clr()         PB1_clr()
  #define p1_3_in_init()     PB1_input_init()
  #define is_p1_3()          is_PB1()

  #define p1_4_out_init()    PB2_output_init()
  #define p1_4_set()         PB2_set()
  #define p1_4_clr()         PB2_clr()
  #define p1_4_in_init()     PB2_input_init()
  #define is_p1_4()          is_PB2()

  #define p1_5_out_init()    PB3_output_init()
  #define p1_5_set()         PB3_set()
  #define p1_5_clr()         PB3_clr()
  #define p1_5_in_init()     PB3_input_init()
  #define is_p1_5()          is_PB3()

  #define p1_6_out_init()    PB4_output_init()
  #define p1_6_set()         PB4_set()
  #define p1_6_clr()         PB4_clr()
  #define p1_6_in_init()     PB4_input_init()
  #define is_p1_6()          is_PB4()

  #define p1_7_out_init()    PB5_output_init()
  #define p1_7_set()         PB5_set()
  #define p1_7_clr()         PB5_clr()
  #define p1_7_in_init()     PB5_input_init()
  #define is_p1_7()          is_PB5()

  /* ----------------------------------------------------------
                 Macros fuer virtual Port 2
     ---------------------------------------------------------- */

  #define p2_0_out_init()    PC0_output_init()
  #define p2_0_set()         PC0_set()
  #define p2_0_clr()         PC0_clr()
  #define p2_0_in_init()     PC0_input_init()
  #define is_p2_0()          is_PC0()

  #define p2_1_out_init()    PC1_output_init()
  #define p2_1_set()         PC1_set()
  #define p2_1_clr()         PC1_clr()
  #define p2_1_in_init()     PC1_input_init()
  #define is_p2_1()          is_PC1()

  #define p2_2_out_init()    PC2_output_init()
  #define p2_2_set()         PC2_set()
  #define p2_2_clr()         PC2_clr()
  #define p2_2_in_init()     PC2_input_init()
  #define is_p2_2()          is_PC2()

  #define p2_3_out_init()    PC3_output_init()
  #define p2_3_set()         PC3_set()
  #define p2_3_clr()         PC3_clr()
  #define p2_3_in_init()     PC3_input_init()
  #define is_p2_3()          is_PC3()

  #define p2_4_out_init()    PD4_output_init()
  #define p2_4_set()         PD4_set()
  #define p2_4_clr()         PD4_clr()
  #define p2_4_in_init()     PD4_input_init()
  #define is_p2_4()          is_PD4()

  #define p2_5_out_init()    PD3_output_init()
  #define p2_5_set()         PD3_set()
  #define p2_5_clr()         PD3_clr()
  #define p2_5_in_init()     PD3_input_init()
  #define is_p2_5()          is_PD3()

  #define p2_6_out_init()    PD2_output_init()
  #define p2_6_set()         PD2_set()
  #define p2_6_clr()         PD2_clr()
  #define p2_6_in_init()     PD2_input_init()
  #define is_p2_6()          is_PD2()

  // Hinweis: PD0 ist auch Uart - RxD

  #define p2_7_out_init()    PD0_output_init()
  #define p2_7_set()         PD0_set()
  #define p2_7_clr()         PD0_clr()
  #define p2_7_in_init()     PD0_input_init()
  #define is_p2_7()          is_PD0()

#endif
