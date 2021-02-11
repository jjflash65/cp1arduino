/* --------------------------------------------------------
                     cp1_assemble_line.cpp

     Zeilenassembler: assembliert einen String mit CP1
     Mnemonic in ein Word, das den Opcode und das Datum
     enthaelt
     
     Disassembler: erstellt aus einem gegebenen Programm-
     wort die dazugehoerige Mnemonic

     MCU: ATmega328p

     07.01.2021    R. Seelig
   -------------------------------------------------------- */

#include "kosmos_cp1_v43.h"


const uint8_t mnemset[mnemanz][13] PROGMEM =
{
  { "\xff""db ?"},
  { "\x01""hlt"},
  { "\x02""cdis"},
  { "\x03""cdel ?"},
  { "\x04""mvi a,?"},
  { "\x05""mov a,?"},
  { "\x06""mov ?,a"},
  { "\x13""mov a,@?"},
  { "\x14""mov @?,a"},
  { "\x07""add a,?"},
  { "\x08""sub a,?"},
  { "\x0a""cpe a,?"},
  { "\x0c""cpg a,?"},
  { "\x0d""cpl a,?"},
  { "\x09""jmp ?"},
  { "\x0b""jz ?"},
  { "\x15""jmp @?"},
  { "\x0e""notb a"},
  { "\x0f""andb a,?"},
  { "\x10""inb p1,?"},
  { "\x10""in p1"},
  { "\x11""outb p1,?"},
  { "\x11""out p1"},
  { "\x12""outb p2,?"},
  { "\x12""out p2"},
  { "\x19""djnz a,?"},
  { "\x1a""inc a"},
  { "\x1b""dec a"},

  { "\x1c""mvi b,?"},
  { "\x1d""mvi c,?"},
  { "\x1e""mvi d,?"},
  { "\x1f""mvi e,?"},

  { "\x20""mov a,b"},
  { "\x21""mov a,c"},
  { "\x22""mov a,d"},
  { "\x23""mov a,e"},

  { "\x24""mov b,a"},
  { "\x25""mov c,a"},
  { "\x26""mov d,a"},
  { "\x27""mov e,a"},

  { "\x28""mov b,c"},
  { "\x29""mov b,d"},
  { "\x2a""mov b,e"},

  { "\x2b""mov c,b"},
  { "\x2c""mov c,d"},
  { "\x2d""mov c,e"},

  { "\x2e""not a"},
  { "\x2f""xor a,?"},
  { "\x30""xri a,?"},

  { "\x31""cpi a,?"},
  { "\x32""cmp a,?"},

  { "\x33""jpl ?"},
  { "\x34""jpg ?"},
  { "\x35""jc ?"},

  { "\x36""slc a"},
  { "\x37""src a"},

  { "\x38""and a,?"},
  { "\x39""ani a,?"},
  { "\x3a""or a,?"},
  { "\x3b""ori a,?"},
  { "\x3c""adi a,?"},
  { "\x3d""sbi a,?"},
  { "\x3e""mul a,b"},
  { "\x3f""int ?"},

  { "\x40""call ?"},
  { "\x41""ret"},
  { "\x42"".org ?"}
};

/* --------------------------------------------------
                      str_locase

      konvertier alle Grossbuchstaben eine  Strings
      in Kleinbuchstaben

       Uebergabe:
         str  => Ursprungstring
   -------------------------------------------------- */
void str_locase(uint8_t *str)
{
  uint8_t i;

  for(i = 0; str[i]; i++)
  {
    str[i] = tolower(str[i]);
  }
}

/* --------------------------------------------------
                     str_delch

     loescht ein Zeichen aus einer Zeichenkette

     Uebergabe:   str     => Ursprungstring
                  pos     => Loeschposition
   -------------------------------------------------- */
void str_delch(uint8_t *str, uint8_t pos)
{
  uint8_t l,i;
  uint8_t *hptr;

  l= strlen(str);
  hptr= str+pos;
  for (i= pos+1; i!= l+1; i++)
  {
    *hptr= *(hptr+1);
    hptr++;
  }
}

/* --------------------------------------------------
                     str_findch

      findet das erste Vorkommen eines Zeichens in
      einem String. Stringlaenge darf maximal 254
      Zeichen lang sein (da der Rueckgabewert ein
      uint8_t ist)

      Uebergabe:
        str      : Zeiger auf Quellstring
        ch       : zu findendes Zeichen
      Rueckgabe:
        Position des Zeichens, 0 wenn kein Zeichen
        gefunden wurde
   -------------------------------------------------- */
uint8_t str_findch(uint8_t *str, uint8_t ch)
{
  uint8_t pos;

  pos= 1;
  while (*str)
  {
    if (*str == ch) return pos;
    pos++;
    str++;
  }
  return 0;
}

/* --------------------------------------------------
                   str_delallch

     loescht alle angegebenen Zeichen aus einem
     String

      Uebergabe:
        str     : Zeiger auf Quellstring
        ch      : zu loeschende Zeichen
   -------------------------------------------------- */
void str_delallch(uint8_t *str, uint8_t ch)
{
  uint8_t spos, slen;
  uint8_t *hptr;
  uint8_t cnt= 0;

  hptr= str;
  spos= str_findch(str,ch);
  if (!spos) return;
  do
  {
    spos= str_findch(str,ch);
    if (spos)
    {
      str_delch(str,spos-1);
    }
    cnt++;
  } while((spos) && (cnt< 255));
}

/* --------------------------------------------------
                   str_replacech

     ersetzt das erste in Zeichen dest in *str mit
     dem Zeichen in dest

      Uebergabe:
        str     : Zeiger auf Quellstring
        ch      : zu loeschende Zeichen
   -------------------------------------------------- */
void str_replacech(uint8_t *str, uint8_t dest, uint8_t src)
{
  uint8_t spos;

  spos= str_findch(str,src);
  if (spos)
  {
    str += spos-1;
    *str= dest;
  }
}

/* --------------------------------------------------
                      str_insert

       fuegt einen String in einen anderen ein.

       VORSICHT: Es wird keine Pruefung vorgenommen,
       ob der Bufferspeicher ausreichend ist. In
       dem rudimentaeren Fall hier sind 20 Bytes
       ausreichend. Fuer andere Anwendungen ist
       die Speichergroesse ggf. anzupassen oder
       dynamisch zu alozieren.

       Uebergabe:
         *dest : Zielstring, in den eingefuegt wird
         *src  : der einzufuegende String
         pos   : Position, an der eingefuegt wird
   -------------------------------------------------- */
void str_insert(uint8_t *dest, uint8_t *src, uint8_t pos)
{

  uint8_t len, len2;

  len= strlen(dest);
  len2 = strlen(src);

  uint8_t buffer[20] = "";

  strcpy(buffer, dest+pos-1);
  strcpy(dest + pos-1, src);
  strcpy(dest + strlen(dest), buffer);
  return;
}

/* --------------------------------------------------
                   str_delcomment

     loescht einen mit ";" eingeleiteten Kommentar
     aus einem String

      Uebergabe:
        str  : Zeiger auf Quellstring
   -------------------------------------------------- */
void str_delcomment(uint8_t *str)
{
  uint8_t spos;

  spos= str_findch(str, ';');
  if (spos)
  {
    str += spos-1;
    *str= 0;
  }
}

/* --------------------------------------------------
                   str_split2asm

      zerlegt einen praeparierten Assemblerstring
      in seine max. 3 Bestandteile.

      String muss im Format "mnem,op1,op2" sein

      Bsp.: "mvi,a,123"

      liefert: mnem= "mvi"
               op1 = "a"
               op2 = "123"

      Uebergabe:
        str          : Zeiger auf Quellstring
        mnem,op1,op2 : siehe obiges Beispiel

      Rueckgabe      : Anzahl Bestandteile
   -------------------------------------------------- */
uint8_t str_split2asm(uint8_t *str, uint8_t *mnem, uint8_t *op1, uint8_t *op2)
{
  uint8_t *ptr;
  uint8_t manz = 0;

  ptr = strtok(str, ",");

  while (ptr)
  {
    manz++;
    switch (manz)
    {
      case 1 : memcpy(mnem, ptr, strlen(ptr)+1); break;
      case 2 : memcpy(op1, ptr, strlen(ptr)+1); break;
      case 3 : memcpy(op2, ptr, strlen(ptr)+1); break;
      default : break;
    }
    // Test ob strtok funktioniert wie gewuenscht
//    if (ptr)  printf("%s\n\r", ptr);
    ptr = strtok(NULL, ",");
  }
  return manz;
}

/* --------------------------------------------------
                       u8_atoi

     konvertiert einen String, der einen 8-Bit
     Integer repraesentiert in einen uint8_t. Ist
     der Zahl ein "0x" vornangestellt, wird der
     String als hexadezimale Zahl behandelt

     Uebergabe:
       *src   : String der die Zahl als Text
                beinhaltet

       *atch  : haelt fest, ob im String ein
                '@'-Zeichen vorhanden ist

       *err   : Fehlercode
                  0: Konvertierung erfolgreich
                  1: Alphanumerisches Zeichen
                     enthalten
                  2: Zahl groesser als 255
  -------------------------------------------------- */
uint8_t u8_atoi(uint8_t *src, uint8_t *atch, uint8_t *err)
{
  uint16_t result;
  uint8_t *ptr;
  uint8_t *hexmark = "0x";

  result= 0;
  *atch= 0;
  *err= 0;

  if (*src == 0) { *err= 2; return 0; }
  if (*src == '@')
  {
     *atch= 1;
     src++;
  }
  if (*src == 0) { *err= 2; return 0; }
  ptr = strstr(src, hexmark);
  if (!(ptr== NULL))
  {
    // Zahlenstring hat Hexadezimalkennung
    src += 2;
    ptr= src;
  }
  if (ptr == src)
  {
    // hexadezimale Konvertierung
    while(*src)
    {
      if (((*src + 1 - '0' > 0 ) && (*src - '0' < 10)) || ((*src + 1 - 'a' > 0 ) && (*src - 'a' < 16)))
      // gueltige Ziffer
      {
        if (*src <= '9')
        {
          result= (result*16) + *src - '0';
        }
        else
        {
          result= (result*16) + *src - 'a' + 10;
        }
        if (result> 0xff)
        {
          *err= 2;         // Zahlenbereich uint8_t nur bis 255
          return 0;
        }
      }
      else
      // keine Ziffer
      {
        *err= 1;
        return 0;
      }
      src++;
    }
  }
  else
  {
    // dezimale Konvertierung
    while(*src)
    {
      if ((*src + 1 - '0' > 0 ) && (*src - '0' < 10))
      // gueltige Ziffer
      {
        result= (result*10) + *src - '0';
        if (result> 0xff)
        {
          *err= 2;         // Zahlenbereich uint8_t nur bis 255
          return 0;
        }
      }
      else
      // keine Ziffer
      {
        *err= 1;
        return 0;
      }
      src++;
    }
  }
  return result;
}


/* --------------------------------------------------
                           u8toa

      rudimentaeres uin8_t zu String-Konverter

      Uebergabe:
        *s  : Zeiger auf String, der die Konver-
              tierung aufnimmt
        val : zu konvertierender 8-Bit Integer
   -------------------------------------------------- */
void u8toa(uint8_t *s, uint8_t val)
{
  uint8_t b;

  *s= (val / 100)+'0';
  b= (val / 100);
  val-= b*100;
  s++;
  *s= (val / 10)+'0';
  b= (val / 10);
  val-= b*10;
  s++;
  *s= val+'0';
  s++;
  *s= 0;
}

/* --------------------------------------------------
                  mnem_getsearchstring

     ermittelt aus einem String einen Suchstring mit
     dem im Array nach gueltigen Mnemonics gesucht
     werden kann und liefert das zu der Mnemonic
     gehoerende Datum zurueck.

     Uebergabe:
       *dest  : Zeiger der den Suchstring aufnimmt
       *src   : zu analysierender Code
       *err   : Fehlercode

     Rueckgabe :
                einzufuegendes Datum
                
     Errorcodes:
        0     : regulaere Mnemonic
        2     : ungueltiger 8-Bit Wert
        3     : 2 Integerangaben
        6     : regulaerer Kommentar                
  -------------------------------------------------- */
uint8_t mnem_getsearchstring(uint8_t *dest, uint8_t *src, uint8_t *error)
{
  uint8_t mnem[10];
  uint8_t op1[10];
  uint8_t op2[10];
  uint8_t opanz;
  uint8_t err;
  uint8_t data, data2, atch;

  data= 0; data2= 0;
  *error= 0;
  // fuehrende Leerzeichen loeschen
  do
  {
    if (*src== ' ') str_delch(src,0);
  } while (*src== ' ');
  
  if (*src == ';') { *error= 6; return 0; }  
  
  str_replacech(&src[0],',',' ');
  str_locase(&src[0]);                    // und alles nach Kleinbuchstaben konvertieren
  str_delcomment(&src[0]);                // evtl. Kommentar abschneiden

  // alle Leerzeichen, CR- und LF- Zeichen entfernen
  str_delallch(&src[0], 13);
  str_delallch(&src[0], 10);
  str_delallch(&src[0], ' ');
  // String liegt jetzt im Format:  "mov,a,123" vor

  op1[0]= 0; op2[0]= 0;
  opanz= str_split2asm(&src[0], &mnem[0], &op1[0], &op2[0]);  // zerlegt Assemblerbefehl in Bestandteile
  strcpy(dest, mnem);

  // String wieder zusammen setzen
  if (opanz> 1)
  {
    data= u8_atoi(&op1[0], &atch, &err);
    if (!err) strcpy(op1, "?");
    if ((!err) && (atch)) strcpy(op1, "@?");
    if (err== 2) { *error= 2; return 0; }
    strcat(dest, " ");
    strcat(dest, op1);
  }
  if (opanz== 3)
  {
    data2= u8_atoi(&op2[0], &atch,  &err);
    if ((data> 0) && (data2> 0)) { *error= 3; return 0; }     // es koenen keine 2 Zahlenwerte angegeben sein
    if (!err) strcpy(op2, "?");
    if ((!err) && (atch)) strcpy(op2, "@?");
    if (err== 2) { *error= 2; return 0; }
    strcat(dest, ",");
    strcat(dest, op2);
  }
  if (data> 0) return data;
  if (data2> 0) return data2;
  return 0;
}

/* --------------------------------------------------
                    assemble_line

     assembliert eine Zeile und ermittelt den
     Opcode sowie das dazugehoerende Datum. Ist die
     Zeile kein regulaerer Ausdruck, wird als Rueck-
     gabewert der Fehlercode zurueckgegeben.
     Ist kein Fehler aufgetreten wird 0 zurueckge-
     geben.

     Hinweis: Der Originalstring wird zerstoert

     Uebergabe:
       *src     : zu assemblierende Assemblerzeile
       *prgword : nimmt das Programmword (Opcode
                  + Datum auf)

     Rueckgabe Fehlercode:
        0     : regulaere Mnemonic
        2     : ungueltiger 8-Bit Wert
        3     : 2 Integerangaben
        5     : ungueltige Mnemonic
        6     : regulaerer Kommentar    
  -------------------------------------------------- */
uint8_t assemble_line(uint8_t *src, uint16_t *prgword, kcomp *vcp1)
{
  uint8_t  mnemstr[15];
  uint8_t  tmpstr[15];
  uint8_t  *str;
  uint8_t  opc, data, err;
  uint8_t  i, b, scan;

  err= 0;
  data= mnem_getsearchstring(&mnemstr[0], src, &err);
  if (err) return err;

  for (i= 0; i< mnemanz; i++)
  {

    // String aus PROGMEM in ein Array ins Ram umkopieren...
    str= &tmpstr[0];
    // ... und diesen String mit der Mnemonicliste vergleichen
    scan= 0;
    do
    {
      b= pgm_read_byte(&mnemset[i][scan]);
      *str= b;
      str++;
      scan++;
    } while(b);

    if ( (strstr(tmpstr, mnemstr)) && ((strlen(tmpstr)-1 == strlen(mnemstr))) )
    {
      opc=tmpstr[0];
      if (opc== 255) opc= 0;
      if (opc != 0x42)                  // .org
      {
        *prgword= (opc*1000)+data;
        return err;
      }
      else
      {
        err= 255;                       // als Kennung, dass prgword nur die zu
                                        // setzende Addresse beinhaltet
        *prgword= data;
        return err;                                
      }
    }
  }
  err= 5;
  *prgword= 0x7fff;

  return err;
}

/* --------------------------------------------------
                  disassemble_prgword

     disassembliert ein Programmword in seine
     Mnemonic

      Uebergabe:
        *dest   : Zeiger auf Speicher, der den
                  Mnemonicstring aufnimmt
        prgword : zu disassemblierendes Programmword
   -------------------------------------------------- */
void disassemble_prgword(uint8_t *dest, uint16_t prgword)
{
  uint8_t  tmpstr[20];
  uint8_t  dezs[4];
  uint8_t  *str;
  uint8_t  opc, data, err;
  uint8_t  i, b, scan;

  *dest= 0;
  opc= prgword >> 8; data= prgword & 0xff;
  if (opc== 0) opc= 0xff;

  for (i= 0; i< mnemanz; i++)
  {

    // String aus PROGMEM in ein Array ins Ram umkopieren
    str= &tmpstr[0];
    scan= 0;
    do
    {
      b= pgm_read_byte(&mnemset[i][scan]);
      *str= b;
      str++;
      scan++;
    } while(b);

    if (tmpstr[0]== opc)
    {
      if ((opc> 15) && (opc< 19))             // Opcodes 16,17,18 gibt es jeweils 2 mal, je fuer
                                              // Bit und Byteoperation
      {
        switch (opc)
        {
          case 16 :
          {
            if (data) strcpy(tmpstr, "\x10""inb p1,?");
                 else strcpy(tmpstr, "\x10""in p1");
            break;
          }

          case 17 :
          {
            if (data) strcpy(tmpstr, "\x11""outb p1,?");
                 else strcpy(tmpstr, "\x11""out p1");
            break;
          }

          case 18 :
          {
            if (data) strcpy(tmpstr, "\x12""outb p2,?");
                 else strcpy(tmpstr, "\x12""out p2");
            break;
          }
          default : break;
        }
      }
//      puts_rom("   ");
      str_delch(&tmpstr[0],0);
      b= str_findch(&tmpstr[0],'?');
      if (b)                                   // Datum einfuegen ?
      {
        u8toa(&dezs[0], data);
        str_insert(&tmpstr[0], &dezs[0], b);
        str_delch(&tmpstr[0], b+2);
      }
//      puts_ram(&tmpstr[0]);
      strcpy(dest, tmpstr);
      i= mnemanz;                              // Suche abbrechen, da gefunden
    }
  }
}

