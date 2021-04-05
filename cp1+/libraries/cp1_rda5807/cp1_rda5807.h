/* ---------------------------------------------------------------------------
                               cp1_rda5807.h

     rudimentaere Funktionen zum UKW - Empfaengerchip RDA5807

     01.03.2021    R. Seelig
   --------------------------------------------------------------------------- */

#include <avr/io.h>
#include <util/delay.h>

#include "Arduino.h"
#include "cp1_i2c.h"

// Software - I2C : i2c(sda, scl)
extern swi2c i2c;

class rda5807
{
  public:

    #define  rda5807_adrs    0x20           // I2C-addr. fuer sequientielllen Zugriff
    #define  rda5807_adrr    0x22           // I2C-addr. fuer wahlfreien Zugriff
    #define  rda5807_adrt    0xc0           // I2C-addr. fuer TEA5767 kompatiblen Modus

    #define fbandmin         870            // 87.0  MHz unteres Frequenzende
    #define fbandmax         1080           // 108.0 MHz oberes Frequenzende
    #define sigschwelle      72             // Schwelle ab der ein Sender als "gut empfangen" gilt

    uint16_t aktfreq =   1018;              // Startfrequenz ( 101.8 MHz )
    uint8_t  aktvol  =   3;                 // Startlautstaerke
    
    volatile uint16_t reg[7];

    uint16_t tmpfreq;
    uint8_t  tmpvol;

    uint8_t cx;

    const uint16_t regdef[7] = {
                0x0758,                     // 00 default ID
                0x0000,                     // 01 reserved
                0xF009,                     // 02 DHIZ, DMUTE, BASS, POWERUPENABLE, RDS
                0x0000,                     // 03 reserved
                0x1400,                     // 04 SOFTMUTE
                0x84DF,                     // 05 INT_MODE, SEEKTH=0110, unbekannt, Volume=15
                0x4000 };                   // 06 OPENMODE=01


    rda5807();
    void writereg(uint8_t ind);
    void write(void);
    void reset(void);
    void poweron(void);    
    int setfreq(uint16_t channel);
    void setvol(uint8_t vol);
    void setmono(void);
    void setstereo(void);
    uint8_t getsig(void);
    void scandown(void);
    void scanup(void);

  protected:

  private:
    uint8_t sda, scl;
   
};
