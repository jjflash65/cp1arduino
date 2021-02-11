/* -----------------------------------------------------
                      cp1_hx1838.c

     Softwaremodul fuer den Betrieb eines HX1838
     IR-Receiver am CP1+ Board.

     Aufgrund der Portbelegung, die nicht zum HX1838
     kompatibel ist, muss ein Portpin als 0 geschaltet
     werden um dem HX1838 eine Masseleitung zur
     Verfuegung zu stellen (nicht ganz schoen,
     funktioniert aber)

     Modul verwendet Timer2

     Board : CP1+
     F_CPU : 8 MHz intern

     03.02.2021        R. Seelig
  ------------------------------------------------------ */

#include "cp1_hx1838.h"

volatile uint16_t  ir_code;                        // Code des letzten eingegangenen 16-Bit Wertes
volatile uint8_t   ir_newflag;                     // zeigt an, ob ein neuer Wert eingegangen ist

/* -------------------------------------------------------
                        timer2_init

     Initialisiert den Timer2 (8-Bit) mit einer Takt-
     rate von 128u us

     Uebergabe:
        prescale  : Teilerfaktor von F_CPU mit der der
                    Timer getaktet wird.
   ------------------------------------------------------- */
void timer2_init(uint8_t prescale)
{
  /*
  TCCR2B : Timer/Counter Controllregister B
    verantwortlich fuer das Einstellen der Taktversorgung des Zaehlers.
    Bits sind: CS12..CS20
       1 : div(1),   2 : div(8),   3 : div(32),   4 : div(64),
       5 : div(128), 6 : div(256), 7 : div(1024)

    Bsp.:
       F_CPU    = 8 MHz, prescaler 1024 = 7812,5 Hz = 128uS Taktzeit Zaehler
  */
  TCCR2A = 0;                  // Normal Operation, kein PWM, keine Ausgangspins
  TCCR2B = prescale;
  TCNT2  = 0;                  // Zaehler zuruecksetzen
}

/* --------------------------------------------------
                      waittil_hi

     wartet, bis die Signalleitung zu logisch 1
     wird.

     Uebergabe:
        timeout  : Zeit x 100us nach der nach nicht
                   eingehen eines Hi-Pegels ein
                   Timeout ausgeloest wird
   -------------------------------------------------- */
uint8_t waittil_hi(uint16_t timeout)
{
  tim2_clr();
  while ( (!(is_irin() )) && (timeout > tim2_getvalue()) );
  if (timeout > tim2_getvalue()) return 0; else return 1;
}

/* --------------------------------------------------
                      waittil_lo

     wartet, bis die Signalleitung zu logisch 0
     wird.

     Uebergabe:
        timeout  : Zeit x 100us nach der nach nicht
                   eingehen eines Lo-Pegels ein
                   Timeout ausgeloest wird
   -------------------------------------------------- */
uint8_t waittil_lo(uint16_t timeout)
{
  tim2_clr();
  while ( (is_irin() ) && (timeout > tim2_getvalue()) );
  if (timeout > tim2_getvalue()) return 0; else return 1;
}

/* --------------------------------------------------
                      ir_getbit

     liefert je nach Pulselaenge des IR-Receivers 0
     (kurzer Impuls) oder 1 (langer Impuls). Bei
     Timeoutueberschreitung wird 2 zurueck geliefert

     Uebergabe:
        timeout  : Zeit x 100us nach der nach nicht
                   eingehen eines Lo-Pegels ein
                   Timeout ausgeloest wird
     Rueckgabe   : 0,1  Datenbit
                     2  Timeout aufgetreten
   -------------------------------------------------- */
uint8_t ir_getbit(uint16_t timeout)
{
  volatile uint16_t t;

  tim2_clr();
  while ( (is_irin() ) && (timeout > tim2_getvalue()) );
  t= tim2_getvalue();
  if (timeout <= t) return 2;
  if (t< 9) return 0; else return 1;
}

/* --------------------------------------------------
                    pinchange_init
     festlegen, dass Datenanschluss des IR-Receivers
     einen Pinchange Interrupt ausloest
   -------------------------------------------------- */
void pinchange_init(void)
{
  IR_PCMSK |= (1 << IR_PCINT);
  PCICR |= (1 << IR_PCIE);

  ir_input_init();
}

/* --------------------------------------------------
                    pinchange_deinit
     Pinchange-Interrupt deaktivieren
  -------------------------------------------------- */
void pinchange_deinit(void)
{
  IR_PCMSK &= ~(1 << IR_PCINT);
  PCICR &= ~(1 << IR_PCIE);

  ir_input_init();
}


/* --------------------------------------------------
                  IR_ISR_vect

     ISR fuer Pinchangevektor Datapin des
     IR-Receivers

     Dauer Lo-Pegel vor Startbit:   9 ms
     Startbit (Hi)              : 4.5 ms
     Datenbit Lo                : 0.6 ms
     Datenbit Hi                : 1.7 ms

     In der Interruptroutine werden die globalen
     Variablen ir_code und ir_newflag geschrieben,
     die in einem Hauptprogramm gepollt werden
     koennen.
   -------------------------------------------------- */
ISR (IR_ISR_vect)
{
  volatile uint8_t cx, b, hw, hw2;
  volatile uint16_t result;
  static   uint16_t intcnt= 0;

  if (!(is_irin()) )                                   // ist der Datenpin des IR-Receivers zu 0 geworden
  {
    pinchange_deinit();                                // Interruptfaehigkeit des Pins aus

    if ( waittil_hi(93) ) goto timeout_err;            // auf Startbit des Frames warten (nach 93*.128us = 12 ms Timeout)
    if ( waittil_lo(48) ) goto timeout_err;            // auf Ende Startbit des Frames warten (nach 6 ms Timeout)
    if ( waittil_hi(24) ) goto timeout_err;            // auf erstes Datenbit warten (nach 3 ms Timeout)


    // erste 8 Bit Frame lesen
    hw= 0;
    for (cx= 0; cx < 8; cx++)
    {
      b= ir_getbit(24);
      if (b == 2) goto timeout_err;
      hw |= (b << cx);
      if ( waittil_hi(24) ) goto timeout_err;          // auf naechstes Datenbit warten
    }
    result= (uint16_t) hw << 8;

    // zweiter 8 Bit Frame lesen (muss den invertierten Wert
    // des ersten Frames haben, ansonsten war ein Fehler aufgetreten
    hw2= 0;
    for (cx= 0; cx < 8; cx++)
    {
      b= ir_getbit(24);
      if (b == 2) goto timeout_err;
      hw2 |= (b << cx);
      if ( waittil_hi(24) ) goto timeout_err;          // auf naechstes Datenbit warten
    }
    if ((hw + hw2) != 0xff) goto timeout_err;          // Fehler aufgetreten

    // Ergebnis zweiten Frames verwerfen

    // dritten 8 Bit Frame lesen
    hw= 0;
    for (cx= 0; cx < 8; cx++)
    {
      b= ir_getbit(24);
      if (b == 2) goto timeout_err;
      hw |= (b << cx);
      if ( waittil_hi(24) ) goto timeout_err;          // auf naechstes Datenbit warten
    }
    result |= hw;

    // vierten 8 Bit Frame lesen
    hw2= 0;
    for (cx= 0; cx < 8; cx++)
    {
      b= ir_getbit(24);
      if (b == 2) goto timeout_err;
      hw2 |= (b << cx);
      if ( waittil_hi(24) ) goto timeout_err;          // auf naechstes Datenbit warten
    }
    if ((hw + hw2) != 0xff) goto timeout_err;          // Fehler aufgetreten

    // ansonsten den 4. Frame verwerfen und das Ergebnis
    // zurueckgeben

    ir_code= result;
    ir_newflag= 1;

    timeout_err:
       pinchange_init();
  }
}

/* -------------------------------------------------
                       hex1838_init
     festlegen, dass Datenanschluss des IR-Receivers
     einen Pinchange Interrupt ausloest
   -------------------------------------------------- */
void hx1838_init(void)
{
  ir_gnd_init();
  timer2_init(pscale1024);
  pinchange_init();
  sei();
}
