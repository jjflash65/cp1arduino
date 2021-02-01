Add the CP1+ hardware to the Arduino Framework

Das Package beinhaltet eine Boarddefinition fuer das CP1+ Board mit Auswahlmoeglichkeiten
fuer ATmega168 / 328p Controller die wahlweise mit 8 MHz internem Takt oder 16 MHz externem
Quarz betrieben werden werden können.

In den Libraries sind grundsätzlich die Definitionen für Port1 und Port2 enthalten, so dass
I/O Pins Beispielsweise auch mittels digitalWrite(P2_1, HIGH) angesprochen werden koennen.

Desweiteren sind Libraries zum Betrieb eines OLED SPI-Display mit 1306 Controller sowie eine
Library zum Ansprechen des 7-Segment- und Tastencontrollers TM1637 enthalten.

Eine ausfuehrlichere Dokumentation hierueber werde ich (hoffentlich) noch erstellen, vorab
aber erst einmal die Boarddefinitionen.

Im Ordner ./Arduino/bootloaders sind die Bootloader fuer einen Betrieb mit 38400 Bd 
und internem Taktgenerator fuer 8 MHz enthalten.

