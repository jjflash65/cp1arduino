/*  ---------------------------------------------------------
                          charlie20.h

     Header zum Softwaremodul fuer 20 "charliegeplexte"
     LED's
      

     Beim Charlieplexing sind jeweils 2 antiparallel geschaltete Leuchtdioden
     an 2 GPIO-Leitungen angeschlossen. Hieraus ergeben sich 10 Paare zu jeweils
     2 LEDs.

     Bsp.:

       A  o------+-----,         C o------+-----,
                 |     |                  |     |
                ---   ---                ---   ---
              A \ /   / \ B            C \ /   / \ D
              B ---   --- A            D ---   --- C
                 |     |                  |     |
       B  o------+-----'         D o------+-----'
 

       A  B  B  C  C  D  D  E  A  C  C  E  D  B  A  D  A  E  E  B   Linenkombination
       B  A  C  B  D  C  E  D  C  A  E  C  B  D  D  A  E  A  B  E
       ----------------------------------------------------------
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19  LED-Nummern
      
      08.02.2021    R. Seelig                             
    --------------------------------------------------------- */

#ifndef in_charlie20
  #define in_charlie20
  
  #include "Arduino.h" 

  #define charlieA_output()  pinMode(LED_A, OUTPUT)
  #define charlieA_input()   { pinMode(LED_A, INPUT); digitalWrite(LED_A, 0); }
  #define charlieA_set()     digitalWrite(LED_A, 1);
  #define charlieA_clr()     digitalWrite(LED_A, 0);

  #define charlieB_output()  pinMode(LED_B, OUTPUT)
  #define charlieB_input()   { pinMode(LED_B, INPUT); digitalWrite(LED_B, 0); }
  #define charlieB_set()     digitalWrite(LED_B, 1);
  #define charlieB_clr()     digitalWrite(LED_B, 0);
  
  #define charlieC_output()  pinMode(LED_C, OUTPUT)
  #define charlieC_input()   { pinMode(LED_C, INPUT); digitalWrite(LED_C, 0); }
  #define charlieC_set()     digitalWrite(LED_C, 1);
  #define charlieC_clr()     digitalWrite(LED_C, 0);

  #define charlieD_output()  pinMode(LED_D, OUTPUT)
  #define charlieD_input()   { pinMode(LED_D, INPUT); digitalWrite(LED_D, 0); }
  #define charlieD_set()     digitalWrite(LED_D, 1);
  #define charlieD_clr()     digitalWrite(LED_D, 0);
  
  #define charlieE_output()  pinMode(LED_E, OUTPUT)
  #define charlieE_input()   { pinMode(LED_E, INPUT); digitalWrite(LED_E, 0); }
  #define charlieE_set()     digitalWrite(LED_E, 1);
  #define charlieE_clr()     digitalWrite(LED_E, 0);
  
  class charlie20
  {    
    public:
      volatile uint32_t   buffer= 0;        // Buffer in dem ein Bitmuster aufgenommen wird,
                                            // und automatisch ueber den Timer2 Interrupt angezeigt wird
                                                     
      charlie20(uint8_t leda, uint8_t ledb, uint8_t ledc, uint8_t ledd, uint8_t lede);
      void allinput(void);
      void lineset(char nr);
      void plexing(void);
       
    protected:
    
    private:
      const uint16_t cplex[20] =
        // HiByte: definiert, welche der Charlieplexingleitungen Ausgaenge sind
        // LoByte: definiert, welche Bits gesetzt oder geloescht sind
        //
        //    D15 D14 D13 D12 D11 D10 D09 D08 || D07 D06 D05 D04 D03 D02 D01 D00
        //    ------------------------------------------------------------------
        //                  E   D   C   B   A                  E   D   C   B   A
        //
        //                                 BA
        //                       BA    x xx10  
        // Bsp.: 0x0302 = 0000.0011 0000.0010
      
        //   A und B sind Ausgaenge, B= 1, A= 0 => LED BA leuchtet
      
        {
        //  0=AB    1=BA    2=BC    3=CB    4=CD    5=DC    6=DE    7=ED
          0x0301, 0x0302, 0x0602, 0x0604, 0x0c04, 0x0c08, 0x1808, 0x1810,
        //  8=AC    9=CA   10=CE    11=EC  12=DB   13=BD   14=AD   15=DA
          0x0501, 0x0504, 0x1404, 0x1410, 0x0a08, 0x0a02, 0x0901, 0x0908,
        // 16=AE   17=EA   18=EB    19=BE
          0x1101, 0x1110, 0x1210, 0x1202
        };
      uint8_t LED_A, LED_B, LED_C, LED_D, LED_E;      

  };

#endif
