/* ----------------------------------------------------------------------
                            cp1_printf.cpp

     Implementierung einer sehr minimalistischen (und im Funktionsumfang
     eingeschraenkten) Version von printf.

     Benoetigt im Arduino-Sketch eine Funktion:

                   void my_putchar(char c);


     22.04.2016   R. Seelig

     Danke hier an www.mikrocontroller.net (Dr.Sommer, beric)

     letzte Aenderung:
     ---------------------------------------------------

     Angabe der Nachkommastellen bei Verwendung des %k Platzhalters
     
     (Implementation der globalen Variable printfkomma und veraenderte
     < putint > Funktion

     29.08.2018    R. Seelig

   --------------------------------------------------------------------- */

#ifndef in_myprintf
  #define in_myprintf

  #include <avr/io.h>
  #include <avr/pgmspace.h>
  #include <stdarg.h>

  extern char printfkomma;

  void my_putchar(char c);
  void putint(int32_t i, char komma);
  void hexnibbleout(uint8_t b);
  void puthex(uint16_t h);
  void my_putramstring(uint8_t *p);

  void own_printf(const uint8_t *s,...);

  #define tiny_printf(str,...)  (own_printf(PSTR(str), ## __VA_ARGS__))
  #define printf                tiny_printf


#endif


