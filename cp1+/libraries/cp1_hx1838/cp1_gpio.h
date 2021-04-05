/* -----------------------------------------------
                     cp1_gpio.h

     Makros und Defines zum Einfachen Umgang mit
     den GPIO - Portpins des CP1+ Boards

     Wird ein Portpin als Eingang initialisiert
     wird der interne Pop-Up Widerstand einge-
     schaltet !

     02.02.2021  R. Seelig
   ----------------------------------------------- */


// Bitmasken
// ----------------------------------------------
#define MASK0                ( 1 << 0 )
#define MASK1                ( 1 << 1 )
#define MASK2                ( 1 << 2 )
#define MASK3                ( 1 << 3 )
#define MASK4                ( 1 << 4 )
#define MASK5                ( 1 << 5 )
#define MASK6                ( 1 << 6 )
#define MASK7                ( 1 << 7 )


// PortA
// -----------------------------------------------
// PortB
// -----------------------------------------------
#define P1_2_output_init()    ( DDRB |= MASK0 )
#define P1_2_set()            ( PORTB |= MASK0 )
#define P1_2_clr()            ( PORTB &= ~MASK0 )
#define P1_2_input_init()     { DDRB &= ~MASK0; PORTB |= MASK0; }
#define is_P1_2()             ( (PINB & MASK0) >> 0 )

#define P1_3_output_init()    ( DDRB |= MASK1 )
#define P1_3_set()            ( PORTB |= MASK1 )
#define P1_3_clr()            ( PORTB &= ~MASK1 )
#define P1_3_input_init()     { DDRB &= ~MASK1; PORTB |= MASK1; }
#define is_P1_3()             ( (PINB & MASK1) >> 1 )

#define P1_4_output_init()    ( DDRB |= MASK2 )
#define P1_4_set()            ( PORTB |= MASK2 )
#define P1_4_clr()            ( PORTB &= ~MASK2 )
#define P1_4_input_init()     { DDRB &= ~MASK2; PORTB |= MASK2; }
#define is_P1_4()             ( (PINB & MASK2) >> 2 )

#define P1_5_output_init()    ( DDRB |= MASK3 )
#define P1_5_set()            ( PORTB |= MASK3 )
#define P1_5_clr()            ( PORTB &= ~MASK3 )
#define P1_5_input_init()     { DDRB &= ~MASK3; PORTB |= MASK3; }
#define is_P1_5()             ( (PINB & MASK3) >> 3 )

#define P1_6_output_init()    ( DDRB |= MASK4 )
#define P1_6_set()            ( PORTB |= MASK4 )
#define P1_6_clr()            ( PORTB &= ~MASK4 )
#define P1_6_input_init()     { DDRB &= ~MASK4; PORTB |= MASK4; }
#define is_P1_6()             ( (PINB & MASK4) >> 4 )

#define P1_7_output_init()    ( DDRB |= MASK5 )
#define P1_7_set()            ( PORTB |= MASK5 )
#define P1_7_clr()            ( PORTB &= ~MASK5 )
#define P1_7_input_init()     { DDRB &= ~MASK5; PORTB |= MASK5; }
#define is_P1_7()             ( (PINB & MASK5) >> 5 )

// PortC
// -----------------------------------------------
#define P2_0_output_init()    ( DDRC |= MASK0 )
#define P2_0_set()            ( PORTC |= MASK0 )
#define P2_0_clr()            ( PORTC &= ~MASK0 )
#define P2_0_input_init()     { DDRC &= ~MASK0; PORTC |= MASK0; }
#define is_P2_0()             ( (PINC & MASK0) >> 0 )

#define P2_1_output_init()    ( DDRC |= MASK1 )
#define P2_1_set()            ( PORTC |= MASK1 )
#define P2_1_clr()            ( PORTC &= ~MASK1 )
#define P2_1_input_init()     { DDRC &= ~MASK1; PORTC |= MASK1; }
#define is_P2_1()             ( (PINC & MASK1) >> 1 )

#define P2_2_output_init()    ( DDRC |= MASK2 )
#define P2_2_set()            ( PORTC |= MASK2 )
#define P2_2_clr()            ( PORTC &= ~MASK2 )
#define P2_2_input_init()     { DDRC &= ~MASK2; PORTC |= MASK2; }
#define is_P2_2()             ( (PINC & MASK2) >> 2 )

#define P2_3_output_init()    ( DDRC |= MASK3 )
#define P2_3_set()            ( PORTC |= MASK3 )
#define P2_3_clr()            ( PORTC &= ~MASK3 )
#define P2_3_input_init()     { DDRC &= ~MASK3; PORTC |= MASK3; }
#define is_P2_3()             ( (PINC & MASK3) >> 3 )

// PortD
// -----------------------------------------------
#define P2_7_output_init()    ( DDRD |= MASK0 )
#define P2_7_set()            ( PORTD |= MASK0 )
#define P2_7_clr()            ( PORTD &= ~MASK0 )
#define P2_7_input_init()     { DDRD &= ~MASK0; PORTD |= MASK0; }
#define is_P2_7()             ( (PIND & MASK0) >> 0 )

#define P2_6_output_init()    ( DDRD |= MASK2 )
#define P2_6_set()            ( PORTD |= MASK2 )
#define P2_6_clr()            ( PORTD &= ~MASK2 )
#define P2_6_input_init()     { DDRD &= ~MASK2; PORTD |= MASK2; }
#define is_P2_6()             ( (PIND & MASK2) >> 2 )

#define P2_5_output_init()    ( DDRD |= MASK3 )
#define P2_5_set()            ( PORTD |= MASK3 )
#define P2_5_clr()            ( PORTD &= ~MASK3 )
#define P2_5_input_init()     { DDRD &= ~MASK3; PORTD |= MASK3; }
#define is_P2_5()             ( (PIND & MASK3) >> 3 )

#define P2_4_output_init()    ( DDRD |= MASK4 )
#define P2_4_set()            ( PORTD |= MASK4 )
#define P2_4_clr()            ( PORTD &= ~MASK4 )
#define P2_4_input_init()     { DDRD &= ~MASK4; PORTD |= MASK4; }
#define is_P2_4()             ( (PIND & MASK4) >> 4 )

#define P1_0_output_init()    ( DDRD |= MASK6 )
#define P1_0_set()            ( PORTD |= MASK6 )
#define P1_0_clr()            ( PORTD &= ~MASK6 )
#define P1_0_input_init()     { DDRD &= ~MASK6; PORTD |= MASK6; }
#define is_P1_0()             ( (PIND & MASK6) >> 6 )

#define P1_1_output_init()    ( DDRD |= MASK7 )
#define P1_1_set()            ( PORTD |= MASK7 )
#define P1_1_clr()            ( PORTD &= ~MASK7 )
#define P1_1_input_init()     { DDRD &= ~MASK7; PORTD |= MASK7; }
#define is_P1_1()             ( (PIND & MASK7) >> 7 )

