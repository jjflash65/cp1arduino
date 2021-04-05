
/* ----------------------------------------------------------------------
                            uart_in.ino    
                            
          Eingaberoutinen ueber die serielle Schnittstelle      

     26.03.2021   R. Seelig
   --------------------------------------------------------------------- */


/* --------------------------------------------------
                       readfloat
                       
     einlesen eines Floatwertes ueber die serielle                   
     Schnittstelle
   -------------------------------------------------- */
float readfloat(char maxa)
{
  char str[21];
  char ind= 0;
  char dp= 0;
  char ch;

  str[ind]= 0;                   // Endekennung String
  while(1)
  {
    if (Serial.available() > 0) 
    {
      ch = Serial.read();    
      if (ch== 0x0d)
      {
        return atof(str);
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
          if (str[ind]==  '.') dp= 0;
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
          
          // Test, ob es erlaubte Zeichen eines Floats sind
          if (((ch>= '0') && (ch <='9')) || (ch== '.') || (ch== '-'))
          {
            if ((!dp) || (ch != '.')) 
            {
              if ((ch != '-') || (ind== 0))
              {
                str[ind]= ch;                    // eingelesenes Zeichen hinzufügen
                ind++;                           // Index im Stringpuffer erhoehen
                str[ind]= 0;                     // Endekennung setzen
                Serial.write(ch);                // eingegebenes Zeichen anzeigen 
              }  
            }  
            if (ch== '.') dp= 1;              
          } 
        }
      }
    }  
  }
}

/* --------------------------------------------------
                       readint
                       
     einlesen eines Integerwertes ueber die serielle                   
     Schnittstelle
   -------------------------------------------------- */
int32_t readint(char maxa)
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

/*  ---------------------------------------------------------
                             setup
    --------------------------------------------------------- */
void setup() 
{ 
  char ch;
  
  Serial.begin(38400);
  Serial.write("\n\r -----------------------------------");
  Serial.write("\n\r   Einlesen von Zahlenwerten");
  Serial.write("\n\r -----------------------------------\n\r");  

}

/*  ---------------------------------------------------------
                             loop
    --------------------------------------------------------- */
void loop() 
{
  char ch;
  char str[40];
  float f;
  int32_t z;
   
  Serial.write("\n\n\r Floateingabe: ");
  f= readfloat(10);
  Serial.write("\n\r   Eingabe war: ");
  Serial.print(f,2);
  Serial.write("\n\n\r Integereingabe: ");
  z= readint(10);
  Serial.write("\n\r   Eingabe war: ");
  Serial.print(z);
  Serial.write("\n\n\r Zeicheneingabe: ");
  ch= uart_getchar();
  Serial.write("\n\r   Eingabe war: ");  
  Serial.print(ch);  
}  
