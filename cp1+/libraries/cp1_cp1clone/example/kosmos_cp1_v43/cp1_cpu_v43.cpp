/* --------------------------------------------------------
                       cp1_cpu_v43.cpp

     die virtuelle CPU des Kosmos-CP1

     MCU: ATmega

     01.01.2021    R. Seelig
   ----------------------------------------------- */

#include "kosmos_cp1_v43.h"


/*  ---------------------------------------------------------
                             cpu_reset

                alle Werte der CPU zuruecksetzen
    --------------------------------------------------------- */
void cpu_reset(kcomp *vcp1)
{
  uint16_t i;

  vcp1->pc= 0;
  vcp1->a= 0;
  vcp1->b= 0;
  vcp1->c= 0;
  vcp1->d= 0;
  vcp1->e= 0;
  vcp1->addr= 0;
  vcp1->psw= 0;
  vcp1->sp= 7;

  for (i= 0; i< memsize; i++)
    vcp1->mem[i]= 0;
}

/*  ---------------------------------------------------------
                             cpu_run

      arbeitet den Programmspeicher ab, solange, bis eine
      HLT Anweisung auftritt, oder das Programm mittels
      der STP Taste angehalten wird.

      Uebergabe:
        stepmode == 1 : Einzelschrittabarbeitung
                    0 : Kontinuierlich

        *vcp1    :  Zeiger auf Zustand Gesamtsystem
    --------------------------------------------------------- */
void cpu_run(kcomp *vcp1, uint8_t stepmode)
{
  uint8_t  i;
  uint16_t memw;
  int      data, akku, opc, pc;
  uint8_t  flag;
  uint8_t  key;

  showmask= 0x07;
  key= 0;
  do
  {
    pc= vcp1->pc;
    if (pc> (memsize-1)) { err= 3; return; }
    memw= vcp1->mem[pc];
    opc= memw >> 8;
    data= memw & 0xff;
    if (data> 255) { err= 4; return; }
    if (opc> opcmax) { err= 4; return; }
    if (opc== 0) { err= 2; return; }     // ein Datum ist kein Code und kann nicht ausgefuehrt werden

    switch (opc)
    {
      case 1:
      // HLT (hlt)
      {
        vcp1->pc++;
//        vcp1->psw= 0;
        err= 0;
        return;
        break;
      }

      // ANZ (cdis : call display)
      case 2 :
      {
        showmask= 0x07;
        setdez(vcp1->a,0);
        vcp1->pc++;
        break;
      }

      // VZG (cdel : call delay)
      case 3 :
      {
        key= cp1_delay(data);
        vcp1->pc++;
        if (key== 0x82)
        {
          err= 255;             // err255 wird missbraucht, um anzuzeigen,
                                // dass Programmlauf gestoppt wurde
          return;
        }
        break;
      }

      // AKO (mvi a,const)
      case 4 :
      {
        vcp1->a= data;
        vcp1->pc++;
        break;
      }

      // LDA (mov a,mem)
      case 5 :
      {
        vcp1->a= vcp1->mem[data];
        vcp1->pc++;
        break;
      }

      // ABS (mov mem,a)
      case 6 :
      {
        vcp1->mem[data]= vcp1->a;
        vcp1->pc++;
        break;
      }

      // ADD (add a,mem)
      case 7 :
      {
        vcp1->psw &= 0xfc;                           // Carry und Zero loeschen
        akku = vcp1->a;
        akku += vcp1->mem[data];

        // beim Ueberlaufen reagiert der originale CP1 mit einer
        // Fehlermeldung. Diese wird durch das Setzen des
        // Carry-Flags ersetzt
        // if (akku > 255) { err= 6; return; }
        if (akku > 255) vcp1->psw |= 0x02;           // Carry bei overflow setzen

        if (vcp1->mem[data] & 0xff00) {err= 5; return; }
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // SUB (sub a,mem)
      case 8 :
      {
        vcp1->psw &= 0xfc;                           // Carry und Zero loeschen
        akku = vcp1->a;
        akku -= vcp1->mem[data];

        // beim Unterschreiten von 0 reagiert der originale CP1 mit einer
        // Fehlermeldung. Diese wird durch das Setzen des
        // Carry-Flags ersetzt
        // if (akku < 0) { err= 6; return; }
        if (akku < 0) vcp1->psw |= 0x02;             // Carry bei underrun setzen

        if (vcp1->mem[data] & 0xff00) {err= 5; return; }
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // SPU (jmp addr)
      case 9 :
      {
        vcp1->psw= 0;
        pc= data;
        if (pc > (memsize-1)) { err= 3; return; }
        vcp1->pc= pc;
        break;
      }

      // VGL (cmpe a,mem :  compare equal)
      case 10:
      {
        vcp1->psw &= 0xfe;
        if (vcp1->a== vcp1->mem[data]) vcp1->psw |= 1;
        vcp1->pc++;
        break;
      }

      // SPB (jz addr)
      case 11:
      {
        if (vcp1->psw & 0x01)
        {
          vcp1->pc= data;
        }
        else
        {
          vcp1->pc++;
        }
        vcp1->psw= 0;
        break;
      }

      // VGR (cmpg a,mem : compare greater)
      case 12:
      {
        vcp1->psw &= 0xfe;
        if (vcp1->a > vcp1->mem[data]) vcp1->psw |= 0x01;
        vcp1->pc++;
        break;
      }

      // VKL (cmpl a,mem :  compare less)
      case 13:
      {
        vcp1->psw &= 0xfe;
        if (vcp1->a < vcp1->mem[data]) vcp1->psw |= 0x01;
        vcp1->pc++;
        break;
      }

      // NEG (notb a : invert Bit0 accu) !!! nur einzelnes Bit auf Bitposition 0
      case 14:
      {
        akku= ~((vcp1->a) & 0x01);
        vcp1->a= akku & 0x01;
        vcp1->pc++;
        break;
      }

      // UND (andb a,mem : and bit0 with mem) !!! nur einzelnes Bit auf Bitposition 0
      case 15:
      {
        data= vcp1->mem[data] & 0x01;
        vcp1->a = (vcp1->a & 0x01) & data;
        vcp1->pc++;
        break;
      }

      // P1E (in p1 / inb p1 : read bit or byte from Port1))
      case 16:
      {
        if (data > 8) { err= 5; return; }    // es gibt keine Klemmennummer groesser als 8
        if (data)                            // einzelnes Bit des Port1 lesen
        {
          // Zaehlung Klemmennummer beginnt mit 1
          if (p1_bitread(data-1)) vcp1->a= 1; else vcp1->a= 0;
        }
        else
        {
          vcp1->a= (p1_byteread());
        }
        vcp1->pc++;
        break;
      }

      // P1A (out p1 / outb p1 : outputs a bit or a byte to Port1 )
      {
      case 17:
        if (data > 8) { err= 5; return; }       // es gibt keine Klemmennummer groesser als 8
        if (data)                               // einzelnes Bit des Port1 setzen
        {
          p1_bitwrite(data-1, vcp1->a & 0x01);  // Klemmennr. beginnt mit 1 (und nicht mit 0)
        }
        else
        {
          p1_config(0xff);
          p1_bytewrite(vcp1->a);
        }
        vcp1->pc++;
        break;
      }

      // P2A (out p2 / outb p1 : outputs a bit or a byte to Port2 )
      {
      case 18:
        if (data > 8) { err= 5; return; }       // es gibt keine Klemmennummer groesser als 8
        if (data)                               // einzelnes Bit des Port2 setzen
        {
          p2_bitwrite(data-1, vcp1->a & 0x01);  // Klemmennr. beginnt mit 1 (und nicht mit 0)
        }
        else
        {
          p2_config(0xff);
          p2_bytewrite(vcp1->a);
        }
        vcp1->pc++;
        break;
      }

      // LIA (mov a,@mem) load indirect
      case 19:
      {
        data= vcp1->mem[data];       // data hat nun die Adresse, die im Memory stand
        if (data> memsize-1) { err= 3; return ; }
        data= vcp1->mem[data];
        if (data> 255) { err= 6; return; }
        vcp1->a= data;
        vcp1->pc++;
        break;
      }

      // AIS (mov @mem,a) store indirect
      case 20:
      {
        data= vcp1->mem[data];       // data hat nun die Adresse, die im Memory stand
        if (data> memsize-1) { err= 3; return ; }
        vcp1->mem[data]= vcp1->a;
        vcp1->pc++;
        break;
      }

      // SIU (jmp @mem) jump indirect
      case 21:
      {
        data= vcp1->mem[data];       // data hat nun die Adresse, die im Memory stand
        if (data> memsize-1) { err= 3; return ; }
        vcp1->pc= data;
        break;
      }

      // nicht originale Opcodes / Erweiterung

      // djnz a,mem
      case 25:
      {
        if (data> memsize-1) { err= 3; return ; }
        vcp1->a--;
        if (vcp1->a) vcp1->pc= data; else vcp1->pc++;
        break;
      }

      // inc a
      case 26:
      {
        vcp1->psw &= ~(0x02);        
        if ((vcp1->a) == 255) { vcp1->psw |= 0x02; }
        vcp1->a++;
        vcp1->pc++;
        break;
      }

      // dec a
      case 27:
      {
        vcp1->psw &= ~(0x02);                
        if ((vcp1->a) == 0) { vcp1->psw |= 0x02; }
        vcp1->a--;
        vcp1->pc++;
        break;
      }

      // mvi b, const
      case 28:

      // mvi c, const
      case 29:

      // mvi d, const
      case 30:

      // mvi e, const
      case 31:
      {
        switch (opc)
        {
          case 28 : vcp1->b= data; break;
          case 29 : vcp1->c= data; break;
          case 30 : vcp1->d= data; break;
          case 31 : vcp1->e= data; break;
        }
        vcp1->pc++;
        break;
      }

      // mov a, b
      case 32:

      // mov a, c
      case 33:

      // mov a, d
      case 34:

      // mov a, e
      case 35:
      {
        switch (opc)
        {
          case 32 : vcp1->a= vcp1->b; break;
          case 33 : vcp1->a= vcp1->c; break;
          case 34 : vcp1->a= vcp1->d; break;
          case 35 : vcp1->a= vcp1->e; break;
        }
        vcp1->pc++;
        break;
      }

      // mov b,a
      case 36:

      // mov c,a
      case 37:

      // mov d,a
      case 38:

      // mov e,a
      case 39:
      {
        switch (opc)
        {
          case 36 : vcp1->b= vcp1->a; break;
          case 37 : vcp1->c= vcp1->a; break;
          case 38 : vcp1->d= vcp1->a; break;
          case 39 : vcp1->e= vcp1->a; break;
        }
        vcp1->pc++;
        break;
      }

      // mov b,c
      case 40:

      // mov b,d
      case 41:

      // mov b,e
      case 42:
      {
        switch (opc)
        {
          case 40 : vcp1->b= vcp1->c; break;
          case 41 : vcp1->b= vcp1->d; break;
          case 42 : vcp1->b= vcp1->e; break;
        }
        vcp1->pc++;
        break;
      }

      // mov c,b
      case 43:

      // mov c,d
      case 44:

      // mov c,e
      case 45:
      {
        switch (opc)
        {
          case 43 : vcp1->c= vcp1->b; break;
          case 44 : vcp1->c= vcp1->d; break;
          case 45 : vcp1->c= vcp1->e; break;
        }
        vcp1->pc++;
        break;
      }

      // not a
      case 46:
      {
        vcp1->a = ~(vcp1->a);
        vcp1->pc++;
        break;
      }

      // xor a,mem
      case 47:
      {
        vcp1->a ^= vcp1->mem[data];
        vcp1->pc++;
      }

      // xri a,const
      case 48:
      {
        vcp1->a ^= data;
        vcp1->pc++;
        break;
      }

      // cpi a,const
      case 49:
      {
        vcp1->psw &= 0xf0;      // zero, carry, less, greater loeschen
        if (vcp1->a == data) vcp1->psw |= 0x01;
        if (vcp1->a < data) vcp1->psw |= 0x04;
        if (vcp1->a > data) vcp1->psw |= 0x08;
        vcp1->pc++;
        break;
      }

      // cmp a,mem : compare akku with memory
      case 50:
      {
        vcp1->psw &= 0xf0;      // zero, carry, less, greater loeschen
        if (vcp1->a == vcp1->mem[data]) vcp1->psw |= 0x01;
        if (vcp1->a < vcp1->mem[data]) vcp1->psw |= 0x04;
        if (vcp1->a > vcp1->mem[data]) vcp1->psw |= 0x08;
        vcp1->pc++;
        break;
      }

      // jpl addr : jump less
      case 51:
      {
        if (vcp1->psw & 0x04)
        {
          vcp1->pc= data;
        }
        else
        {
          vcp1->pc++;
        }
        vcp1->psw= 0;
        break;
      }

      // jpg addr : jump greater
      case 52:
      {
        if (vcp1->psw & 0x08)
        {
          vcp1->pc= data;
        }
        else
        {
          vcp1->pc++;
        }
        vcp1->psw= 0;
        break;
      }

      // jc addr : jump carry
      case 53:
      {
        if (vcp1->psw & 0x02)
        {
          vcp1->pc= data;
        }
        else
        {
          vcp1->pc++;
        }
        vcp1->psw= 0;
        break;
      }
      // slc a : shift left through carry
      case 54:
      {
        vcp1->psw &= ~(0x02);                   // Carry loeschen
        akku= vcp1->a;
        if (akku & 0x80) vcp1->psw |= 0x02;     // nach dem Schieben wird es ein overflow gegeben haben
        akku= akku << 1;
        vcp1->a= akku;
        vcp1->pc++;
        break;
      }

      // src a : shift right through carry
      case 55:
      {
        vcp1->psw &= ~(0x02);                   // Carry loeschen
        akku= vcp1->a;
        if (akku & 0x01) vcp1->psw |= 0x02;     // nach dem Schieben wird es ein underflow gegeben haben
        akku= akku >> 1;
        vcp1->a= akku;
        vcp1->pc++;
        break;
      }

      // and a,mem : akku logical and with memory (a= a & mem)
      case 56 :
      {
        vcp1->psw &= 0xfe;
        akku = vcp1->a;
        akku &= vcp1->mem[data];

        if (vcp1->mem[data] & 0xff00) {err= 5; return; }  // Fehler bei Speicher > 255
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // ani a,const : akku logical and with const (a= a & const)
      case 57 :
      {
        vcp1->psw &= 0xfe;
        akku = vcp1->a;
        akku &= data;

        if (data & 0xff00) {err= 5; return; }  // Fehler bei Datum > 255
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // or a,mem : akku logical or with memory (a= a | mem)
      case 58 :
      {
        vcp1->psw &= 0xfe;
        akku = vcp1->a;
        akku |= vcp1->mem[data];

        if (vcp1->mem[data] & 0xff00) {err= 5; return; }  // Fehler bei Speicher > 255
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // ori a,const : akku logical or with const (a= a | const)
      case 59 :
      {
        vcp1->psw &= 0xfe;
        akku = vcp1->a;
        akku |= data;

        if (data & 0xff00) {err= 5; return; }  // Fehler bei Datum > 255
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // adi a,const : akku immediately add with const: a= a + const
      case 60 :
      {
        vcp1->psw &= 0xfc;                     // Zero und Carry loeschen
        akku = vcp1->a;
        akku += data;

        if (data & 0xff00) {err= 5; return; }  // Fehler bei Datum > 255
        if (akku > 255) vcp1->psw |= 0x02;     // bei Ueberlauf Carry setzen
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // sbi a,const : akku immediately substract with const: a= a - const
      case 61 :
      {
        vcp1->psw &= 0xfc;                     // Zero und Carry loeschen
        akku = vcp1->a;
        akku -= data;

        if (data & 0xff00) {err= 5; return; }  // Fehler bei Datum > 255
        if (akku < 0) vcp1->psw |= 0x02;     // bei Ueberlauf Carry setzen
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // mul a,b : multiplicate akku with register b: a= a * b
      case 62 :
      {
        vcp1->psw &= 0xfc;                     // Zero und Carry loeschen
        akku = vcp1->a;
        akku = akku * vcp1->b;

        if (data & 0xff00) {err= 5; return; }  // Fehler bei Datum > 255
        if (akku > 255) vcp1->psw |= 0x02;     // bei Ueberlauf Carry setzen
        vcp1->pc++;
        vcp1->a = akku;
        break;
      }

      // int const: call a software interrupt with number const
      case 63 :
      {
        key= softw_intr(vcp1, data);
        vcp1->pc++;
        if (key== 0x82)
        {
          err= 255;
          return;
        }
        if (err) return;
        break;
      }

      // call addr: call an address SP= SP-1 PC= addr
      case 64 :
      {
        if (data > (memsize-1)) { err= 3; return; }
        vcp1->stack[vcp1->sp]= vcp1->pc + 1;      // Ruecksprungadresse auf den Stack
        vcp1->sp--;                               // naechste Stackposition festlegen
        if ((vcp1->sp) < 0) { err= 10; return ;}  // Stackoverflow
        vcp1->pc= data;                           // neuen Programmcounter setzen ( = Sprung )
        break;
      }

      // ret: return to calling program PC= stack[sp]; SP = SP+1
      case 65 :
      {
        if ((vcp1->sp) == 7) { err= 11; return;}  // Stack underrun
        vcp1->sp++;                               // Stackpointer aktualisieren
        vcp1->pc= vcp1->stack[vcp1->sp];          // Ruecksprungadresse holen und setzen
      }
    }
    key= readshiftkeys();
  } while (!(stepmode) && (key != 0x82));     // 0x82 = Funktion "STP"
  if (key== 0x82) err= 255;                   // missbrauchter Fehlercode als Kennung dass                                              // Programm mit STP-Taste beendet wurde
}
