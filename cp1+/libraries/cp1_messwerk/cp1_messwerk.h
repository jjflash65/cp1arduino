/* -----------------------------------------------------
                     cp1_messwerk.h

     stellt ein graphisches analoges Messwerk in den
     Dimensionen 128x64 zur Verfügung

     Board : CP1+
     F_CPU : 8 MHz intern

     24.03.2021        R. Seelig
  ------------------------------------------------------ */

#include "Arduino.h"
#include <avr/io.h>
#include "cp1_printf.h"
#include <string.h>  
#include "st7735.h"

extern st7735 lcd;

#ifndef in_messwerk
#define in_messwerk   

#define sinD(a)   (sin(a * M_PI / 180.0))
#define cosD(a)   (cos(a * M_PI / 180.0))

struct krpos
{
  uint16_t x1;
  uint16_t y1;
  uint16_t x2;
  uint16_t y2;
};

struct mw_args
{
  uint16_t xm;         // x-Mittelpunkt des Zeigers
  uint16_t ym;         // dto. y
  uint16_t rad;        // Radius des aeusseren Zeigerkreise
  uint16_t zl;         // zl = Zeigerlaenge
};

struct mw_viscolors
{
  uint16_t bkcol;
  uint16_t skalacol;
  uint16_t zeigercol;
  uint16_t textcol;
  uint16_t framecol;
};

class instrumentA
{
  public:
  
    mw_viscolors mw_col;
    
    uint16_t xofs = 0;
    uint16_t yofs = 0;
    
    instrumentA(uint16_t x, uint16_t y);
    void setcolors(uint16_t frame, uint16_t bk, uint16_t skala, uint16_t zeiger, uint16_t text);
    void drawscreen(char *t_lo, char *t_mid, char *t_hi, char *title);
    void drawzeiger(uint16_t adcw, uint8_t drawmode, char *t_mid, char *title);
    void drawdigital(uint8_t x, uint8_t y, uint32_t adcw, uint32_t maxv, uint16_t col);
 
  private:
    mw_args      mw;  
    
    void dtoa(uint8_t *dstr, int32_t i, char komma);   
    krpos kr_getxy(mw_args mw, uint16_t wi);
    void mw_setargs(uint16_t xm, uint16_t ym, uint16_t rad, uint16_t zl);
        
  protected:
};

#endif  
