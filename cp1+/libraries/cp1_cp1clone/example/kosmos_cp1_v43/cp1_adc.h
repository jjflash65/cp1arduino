/* ------------------------------------------------------------------
                             cp1_adc.h

     Header zum Softwaremodul: Ansprechen des internen AD-Wandlers

     MCU   : ATmega328

     25.11.2019 R. Seelig
   ------------------------------------------------------------------ */

#ifndef in_single_adc
  #define in_single_adc

  #include <avr/io.h>
  #include <util/delay.h>
  #include <stdio.h>
  #include <stdint.h>


  enum { adc_ref_ext, adc_ref_avcc, adc_ref_intrn= 3};
  enum { adc_in_pc0, adc_in_pc1, adc_in_pc2, adc_in_pc3, adc_in_pc4, adc_in_pc5, adc_in_adc6, adc_in_adc7 };

  unsigned int getadc_10bit (void);
  void adc_init(uint8_t vref, uint8_t channel);

#endif
