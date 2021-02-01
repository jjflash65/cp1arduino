/*  ---------------------------------------------------------
                          blink.ino

      Einfaches Blinkprogramm auf Port 2

      29.01.2021    R. Seelig                             
    --------------------------------------------------------- */

#define LED     P2_0
#define speed   500

/*  ---------------------------------------------------------
                             setup
               wird einmal beim Start durchgefuehrt
    --------------------------------------------------------- */
void setup()
{
  pinMode(LED, OUTPUT);    
}
/*  ---------------------------------------------------------
                             loop
      wird nach setup in einer Endlosschleife durchgefuehrt                             
    --------------------------------------------------------- */
void loop() 
{
  digitalWrite(LED, HIGH);           // schaltet LED an
  delay(speed);                      // Wartezeit wie im "#define speed" vorgegeben
  digitalWrite(LED, LOW);            // und LED wieder aus
  delay(speed);                      // und wieder warten
}
