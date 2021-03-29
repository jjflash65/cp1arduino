/* ---------------------------------------------------------------------------
                                cp1_i2c.ino

     Demonstriert softwarerealisierte I2C-Funktionen anhand eines
     Busscans

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */  

#include "cp1_printf.h"
#include "cp1_i2c.h"

/* --------------------------------------------------
                       my_putchar
                       
     wird zwingend "cp1_printf.cpp" benoetigt! 
     Ueber diese Funktion erfolgen die Ausgaben von
     printf                       
   -------------------------------------------------- */
void my_putchar(char ch)
{
  Serial.write(ch);
}

// Objekt ableiten: i2c(sda_pin, scl_pin)
swi2c i2c(P1_5, P1_4);

/* --------------------------------------------------
                       i2c_scanbus

      sucht den I2C Bus nach angeschlossenen 
      Devices ab                                         
   -------------------------------------------------- */
void i2c_scanbus(void)
{
  uint8_t cx;
  uint8_t ack;
  
  printf("\n\r --------------------------------");
  printf("\n\r   I2C-Busscan");
  printf("\n\r --------------------------------\n\r");  
  
  for (cx= 0; cx< 254; cx += 2)
  {
    ack= i2c.start(cx);
    delay(1);
    i2c.stop();
    printf("%x\r",cx);
    delay(10);
    if (ack)
    {
      switch (cx)
      {
        case 0x20 :
        case 0x22 : printf("Adr. %xh : RDA5807 UKW-Radio\n\r", cx); break;
        case 0x40 :
        case 0x42 :
        case 0x44 :
        case 0x46 :
        case 0x48 :
        case 0x4A :
        case 0x4C :
        case 0x4E : printf("Adr. %xh : PCF8574 I/O Expander\n\r", cx); break;
        case 0x90 :
        case 0x92 :
        case 0x94 :
        case 0x96 :
        case 0x98 :
        case 0x9A :
        case 0x9C :
        case 0x9E : printf("Adr. %xh : LM75 Temp.-Sensor\n\r", cx); break;
        case 0xA0 :
        case 0xA2 :
        case 0xA4 :
        case 0xA6 :
        case 0xA8 :
        case 0xAA :
        case 0xAC :
        case 0xAE : printf("Adr. %xh : EEProm\n\r", cx); break;
        case 0x78 : printf("Adr. %xh : SSD13016 I2C-OLED Display\n\r", cx); break;
        case 0xC0 : printf("Adr. %xh : TEA5767 UKW-Radio\n\r", cx); break;        
        case 0xD0 : printf("Adr. %xh : RTC - DS1307\n\r", cx); break;

        default   : printf("Adr. %xh : unknown\n\r",cx); break;
      }
    }    
  }  
}

/* --------------------------------------------------
                          setup
   -------------------------------------------------- */
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(38400);
}

/* --------------------------------------------------
                          loop
   -------------------------------------------------- */
void loop() 
{
  uint8_t ch;

  i2c_scanbus();  
  printf("\n\n\rEnd of I2C-bus scanning... \n\n\r");
  printf("Press any key for rescan... \n\r");
  while(Serial.available()== 0);
  ch= Serial.read();
}
