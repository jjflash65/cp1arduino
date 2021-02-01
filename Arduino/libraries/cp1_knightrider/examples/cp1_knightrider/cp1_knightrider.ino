/*  ---------------------------------------------------------
                      cp1_knightrider.ino

      Demo der Leuchtdiodenreihe auf dem CP1+ Board

      27.01.2021    R. Seelig                             
    --------------------------------------------------------- */

void PORT2_byteout(uint8_t value)
{
  if (value & 0x01) digitalWrite(P2_0, HIGH); else digitalWrite(P2_0, LOW);
  if (value & 0x02) digitalWrite(P2_1, HIGH); else digitalWrite(P2_1, LOW);
  if (value & 0x04) digitalWrite(P2_2, HIGH); else digitalWrite(P2_2, LOW);
  if (value & 0x08) digitalWrite(P2_3, HIGH); else digitalWrite(P2_3, LOW);
  if (value & 0x10) digitalWrite(P2_4, HIGH); else digitalWrite(P2_4, LOW);
  if (value & 0x20) digitalWrite(P2_5, HIGH); else digitalWrite(P2_5, LOW);
  if (value & 0x40) digitalWrite(P2_6, HIGH); else digitalWrite(P2_6, LOW);
  if (value & 0x80) digitalWrite(P2_7, HIGH); else digitalWrite(P2_7, LOW);
}

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{
  pinMode(P2_0, OUTPUT);
  pinMode(P2_1, OUTPUT);
  pinMode(P2_2, OUTPUT);
  pinMode(P2_3, OUTPUT);
  pinMode(P2_4, OUTPUT);
  pinMode(P2_5, OUTPUT);
  pinMode(P2_6, OUTPUT);
  pinMode(P2_7, OUTPUT);
}

#define speed   50

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  uint8_t kr;

  kr= 2;

  // links schieben
  while( kr )
  {
    PORT2_byteout(kr);
    delay(speed);
    kr= kr << 1;
  }

  kr = 0x40;
  // rechts schieben
  while( kr )
  {
    PORT2_byteout(kr);
    delay(speed);
    kr= kr >> 1;
  }
  
}

