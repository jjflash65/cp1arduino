/* -------------------------------------------------
                      cp1_uart.cpp

     Softwaremodul zur Verwendung der seriellen
     Schnittstelle von AVR ATmega Controller

     Compiler: AVR-GCC 4.7.2

     MCU:8
           - ATmega8
           - ATmega88 .. ATmega328

     02.01.2021        R. Seelig
  -------------------------------------------------- */


#include <stdio.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>

#include "cp1_uart.h"

/* --------------------------------------------------
                      uart_init

    Initialisierung der seriellen Schnittstelle:

    Uebergabe: baud = Baudrate
    Protokoll: 8 Daten-, 1 Stopbit
   -------------------------------------------------- */
void uart_init(uint32_t baud)
{
  uint16_t ubrr;

  if (baud> 57600)
  {
    baud= baud>>1;
    ubrr= (F_CPU/16/baud);
    ubrr--;
    UCSR0A |= 1<<U2X0;                                  // Baudrate verdoppeln
  }
  else
  {
    ubrr= (F_CPU/16/baud-1);
  }
  UBRR0H = (unsigned char)(ubrr>>8);                    // Baudrate setzen
  UBRR0L = (unsigned char)ubrr;

  #if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__) || defined (__AVR_ATtiny2313__)

    UCSR0B = (1<<RXEN0)|(1<<TXEN0);                       // Transmitter und Receiver enable
    UCSR0C = (3<<UCSZ00);                                 // 8 Datenbit, 1 Stopbit

  #else

    // ATmega8
    UCSR0B |= (1<<RXEN0) | (1<<TXEN0);
    UCSRC  = (1<<URSEL) | (1<<UCSZ01) | (1<<UCSZ00);

  #endif
}

/* --------------------------------------------------
                         uart_deinit

             TxD und RxD wieder freigeben
   -------------------------------------------------- */
void uart_deinit(void)
{
  #if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega168__) || defined (__AVR_ATtiny2313__)

    UCSR0B &= ~(1<<RXEN0) & ~(1<<TXEN0);                       // Transmitter und Receiver enable

  #else

    // ATmega8
    UCSR0B &= ~(1<<RXEN0) & ~(1<<TXEN0);

  #endif
}

/* --------------------------------------------------
                     uart_putchar

     Zeichen ueber die serielle Schnittstelle senden
   -------------------------------------------------- */

void uart_putchar(unsigned char ch)
{
  while (!( UCSR0A & (1<<UDRE0)));                      // warten bis Transmitterpuffer leer ist
  UDR0 = ch;                                            // Zeichen senden
}

/* --------------------------------------------------
                       uart_ischar

     testen, ob ein Zeichen auf der Schnittstelle
     ansteht
   -------------------------------------------------- */

unsigned char uart_ischar( void )
{
  return (UCSR0A & (1<<RXC0));
}

/* --------------------------------------------------
                       uart_getchar

      Zeichen von serieller Schnittstelle lesen
   -------------------------------------------------- */
unsigned char uart_getchar( void )
{
  while(!(UCSR0A & (1<<RXC0)));                         // warten bis Zeichen eintrifft

  #if (echo_enable == 1)

    char ch;
    ch= UDR0;
    uart_putchar(ch);
    return ch;

  #else

    return UDR0;

  #endif
}


/* --------------------------------------------------
                       uart_crlf

     sendet auf der RS-232 ein Linefeed und ein
     Carriage Return
   -------------------------------------------------- */

void uart_crlf()
{
  uart_putchar(0x0a);
  uart_putchar(0x0d);
}


/* --------------------------------------------------
                  uart_putramstring

     gibt einen String aus dem RAM ueber die RS-232
     aus.
   -------------------------------------------------- */

void uart_putramstring(uint8_t *p)
{
  do
  {
    uart_putchar( *p );
  } while( *p++);
}

/* --------------------------------------------------
                  uart_putromstring

     gibt einen String aus dem ROM ueber die RS-232
     aus.

     Bsp.:

          uart_putromstring(PSTR("Hallo Welt")
   -------------------------------------------------- */

void uart_putromstring(const uint8_t *dataPtr)
{
  unsigned char c;

  for (c=pgm_read_byte(dataPtr); c; ++dataPtr, c=pgm_read_byte(dataPtr))
  {
    uart_putchar(c);
  }
}

/*  ---------------------------------------------------------
                            uart_clr

         liest eventuelles Zeichen im Eingangsbuffer
    --------------------------------------------------------- */
void uart_clr(void)
{
  while (uart_ischar()) uart_getchar();
  _delay_ms(1);               // dem quarzlosen Betrieb geschuldet
}

/*  ---------------------------------------------------------
                         uart_readstr

      liest eine vorgegebene max. Anzahl Zeichen auf dem
      UART ein
    --------------------------------------------------------- */
uint8_t uart_readstr(uint8_t *str, uint8_t chanz)
{
  uint8_t cnt;
  uint8_t ch;
  uint8_t *hptr;

  hptr= str;
  *hptr= 0;
  cnt= 0;
  do
  {

//    ch= uart_getchar();
    do
    {
      ch= 0;
      if (uart_ischar()) ch= uart_getchar();
      if (readshiftkeys() == 0x82) ch= 0xfe;         // STP-Taste auf Tastatur beendet Terminal
    } while (ch== 0);
    if (ch== 0xfe) return 0xfe;

    // kein Return und kein ESC
    if ((ch != 0x1b) && (ch != 0x0d))
    {
      // kein Backspace - Delete
      if ((ch != 0x08) && (ch != 0x7f))
      {
        if (cnt < chanz+1)                 // max. Stringlaenge noch nicht erreicht
        {
          uart_putchar(ch);                // Tastendruck anzeigen
          *hptr= ch;                       // neues Zeichen eintragen
          hptr++;
          *hptr= 0;                        // und Stringende markieren
          cnt++;
        }
      }
      else
      // letztes Zeichen loeschen
      {
        if (hptr > str)                    // ist ueberhaupt ein Zeichen vorhanden
        {
          hptr--;
          cnt--;
          *hptr= 0;                        // neues Stringende

          // letzes Zeichen im Terminal loeschen
          uart_putchar(0x08);
          uart_putchar(' ');
          uart_putchar(0x08);
        }
      }
    }

  } while((ch != 0x1b) && (ch != 0x0d));   // wiederholen bis ESC oder Return
  return ch;
}

/* --------------------------------------------------
                     uart_readu8int

     liest einen Integerzahlenwert ueber den UART
     ein.
     Eine Ueberpruefung auf den Wertebereich findet
     statt !
   -------------------------------------------------- */

uint8_t uart_readu8int(uint8_t *value)
{
  uint8_t  ch, cnt, i;
  int16_t val;

  cnt= 0;
  do
  {
    val= 0;
    do
    {
//      ch= uart_getchar();
      do
      {
        ch= 0;
        if (uart_ischar()) ch= uart_getchar();
        if (readshiftkeys() == 0x82) ch= 0x82;         // STP-Taste auf Tastatur beendet Terminal
      } while (ch== 0);

      if (ch== 0x82) return 0x82;

      if ((ch>= '0') && (ch<= '9'))
      {
        if (cnt < 3)
        {
          val= (val*10) +(ch - '0');
          cnt++;
          uart_putchar(ch);        // Echo des eingebenen Zeichens
        }
      }
      if ((ch== 8) || (ch== 127))  // manche Terminals senden Ascii 127 fuer Delete-Taste
                                   // ansonsten ist 8 der Code fuer Backspace
      {
        if (cnt> 0)
        {
          uart_putchar(8);
          uart_putchar(' ');
          uart_putchar(8);
          val /= 10;
          cnt--;
        }
      }
    } while (ch != 0x0d);        // wiederholen bis Returnzeichen eintrifft
    if (val> 255)
    {
      for (i= cnt; i> 0; i--)
      {
        uart_putchar(8);         // gemachte, fehlerhafte Eingabe durch Backspace
        uart_putchar(' ');       // loeschen
        uart_putchar(8);
      }
      cnt= 0;                    // und Zaehler zuruecksetzen
    }
  } while (val> 255);            // Zahl muss im 8 Bit Integerbereich liegen
  *value= val;
  return 0;
}

/*  ---------------------------------------------------------
                         uart_uint16out

      gibt einen 16-Bit Integer dezimal auf dem UART aus

      Uebergabe
        value : auszugebender Zahlenwert
        dp    : Position, an der ein Dezimalpunkt ausge.
                geben wird, 0 es wird kein Dezimalpunkt
                ausgegeben
    --------------------------------------------------------- */
void uart_uint16out(uint16_t value, uint8_t dp)
{
  uint16_t teiler = 10000;
  uint16_t z;
  uint8_t i;

  for (i= 0; i< 5; i++)
  {
    if ((dp> 0) && (i== dp)) uart_putchar('.');
    z= value / teiler;
    uart_putchar(z + '0');
    value -= z * teiler;
    teiler /= 10;
  }
}

/*  ---------------------------------------------------------
                         uart_uint8out

      gibt einen 8-Bit Integer dezimal auf dem UART aus
    --------------------------------------------------------- */
void uart_uint8out(uint8_t value)
{
  uint16_t teiler = 100;
  uint8_t i, z;

  for (i= 0; i< 3; i++)
  {
    z= value / teiler;
    uart_putchar(z + '0');
    value -= z * teiler;
    teiler /= 10;
  }
}
