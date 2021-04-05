/* -------------------------------------------------------
                         rtc_demo.c

     Liest DS1307 / DS3231 (Realtime-Clock-Chip) aus und 
     zeigt diese auf der seriellen  Schnittstelle an

     26.03.2021  R. Seelig
   ------------------------------------------------------ */
   
#include "my_printf.h"   
#include "cp1_i2c.h"
#include "cp1_rtc.h"

swi2c i2c(P2_0, P2_1);  // Object Software-I2C : i2c(sda, scl)
realtimeclock rtc;      // Object rtc


/* --------------------------------------------------------
   my_putchar

   wird von my-printf / printf aufgerufen und hier muss
   eine Zeichenausgabefunktion angegeben sein, auf das
   printf dann schreibt !
   -------------------------------------------------------- */
void my_putchar(char ch)
{
  Serial.write(ch);
}

/* --------------------------------------------------
                       readint
                       
     einlesen eines Integerwertes ueber die serielle                   
     Schnittstelle
   -------------------------------------------------- */
int32_t readint(uint8_t maxa)
{
  char str[17];
  char ind= 0;
  char ch;

  str[ind]= 0;                   // Endekennung String
  while(1)
  {
    if (Serial.available() > 0) 
    {
      ch = Serial.read();    
      if (ch== 0x0d)
      {
        return atol(str);
      }      
      
      // Test, ob Taste ein Backspace war (verschiedene Terminalemulationen
      if ((ch== 0x08) || (ch== 0x7f) || (ch== 0xff))
      {  
        
        // ist mindestens ein Zeichen im Puffer?
        if (ind > 0)
        {
          // Anzeige auf dem Terminal loeschen
          for (int i= 0; i < ind; i++)
          {
            Serial.write('\b');
            Serial.write(' ');
            Serial.write('\b');
          }

          // eingegebene Zeichen verringern
          ind--;
          str[ind]= 0;

          // Anzeige des neuen um den letzten Wert entfernten Strings
          Serial.write(str);
        }
      }
      else
      {
        
        // ist maximale Anzahl Zeichen noch nicht erreicht ?
        if (ind< maxa)
        {
          
          // Test, ob es erlaubte Zeichen sind
          if ((ch>= '0') && (ch <='9') || (ch == '-'))
          {
            if ((ch != '-') || (ind== 0))
            {
              str[ind]= ch;                    // eingelesenes Zeichen hinzufügen
              ind++;                           // Index im Stringpuffer erhoehen
              str[ind]= 0;                     // Endekennung setzen
              Serial.write(ch);                // eingegebenes Zeichen anzeigen 
            }                
          }  
        }
      }
    }  
  }
}

char uart_getchar(void)
{
  while (Serial.available() == 0);
  return Serial.read();     
}

/* --------------------------------------------------
     stellen

     die Uhr benutzerabgefragt stellen
   -------------------------------------------------- */
void stellen(void)
{
  int       i;
  uint8_t   b,cx;
  uint8_t   dow;

  do
  {
    printf("\n\rStunde: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 24)));
  date.std= i;

  do
  {
    printf("\n\rMinute: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 60)));
  date.min= i;

  do
  {
    printf("\n\rSekunde: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 60)));
  date.sek= i;

  do
  {
    printf("\n\rTag: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 32)));
  date.tag= i;

  do
  {
    printf("\n\rMonat: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 13)));
  date.monat= i;

  do
  {
    printf("\n\rJahr: ");
    i= readint(2);
  } while ( !((i > -1) && (i < 100)));
  date.jahr= i;

  date.dow= rtc.getwtag();
  rtc.writedate();

}

/* -------------------------------------------------------
                          putdez2
     zeigt die 2 stellige, vorzeichenbehaftete dezimale
     Zahl in val an.

     mode greift bei Zahlen kleiner 10:

     mode        0  : es wird eine fuehrende 0 ausgegeben
                 1  : es wird anstelle einer 0 ein Leer-
                      zeichen ausgegeben
                 2  : eine fuehrende 0 wird unterdrueckt
   ------------------------------------------------------- */
void putdez2(signed char val, uint8_t mode)
{
  char b;
  if (val < 0)
  {
    my_putchar('-');
    val= -val;
  }
  b= val / 10;
  if (b == 0)
  {
    switch(mode)
    {
      case 0 : my_putchar('0'); break;
      case 1 : my_putchar(' '); break;
      default : break;
    }
  }
  else
    my_putchar(b+'0');
  b= val % 10;
  my_putchar(b+'0');
}

/* --------------------------------------------------
     showtime

     zeigt die Uhrzeit an
   -------------------------------------------------- */
void showtime(void)
{
  const char tagnam[7][3] =
  {
    "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"
  };
  
  rtc.readdate();  
  printf("\r ");
  putdez2(date.std ,0); my_putchar('.');
  putdez2(date.min ,0); my_putchar(':');
  putdez2(date.sek ,0); printf("  ");

  date.dow= rtc.getwtag();

  // und Datum anzeigen

  printf("%s ",(&tagnam[date.dow][0]));
  putdez2(date.tag ,0); my_putchar('.');
  putdez2(date.monat ,0); my_putchar('.');
  printf("20");
  putdez2(date.jahr ,0);
}

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{ 
  uint8_t ch;

  
  Serial.begin(38400);
  printf("Uhrzeit in RTC ist:\n\r"); 
  rtc.readdate();
  showtime(); 
  
  printf("\n\n\rUhr stellen (j/n)");

  ch= uart_getchar();
  if (ch == 'j')
  {
    stellen();
  }

}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  static uint8_t oldsek;
    
  delay(100);
  rtc.readdate();
  if (oldsek != date.sek)
  {
    showtime();
    oldsek= date.sek;
  }
}
