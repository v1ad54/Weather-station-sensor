#ifndef MeasurementPower_h
#define MeasurementPower_h

const float InternalReferenceVoltage = 1.096; // as measured

float readVcc() {
  ADCSRA =  bit (ADEN);   // turn ADC on
  ADCSRA |= bit (ADPS0) |  bit (ADPS1) | bit (ADPS2);  // Prescaler of 128
  ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
  
  delay (10);  // let it stabilize
  
  bitSet (ADCSRA, ADSC);  // start a conversion  
  while ( bit_is_set(ADCSRA, ADSC) ){
  }
  
  float results = InternalReferenceVoltage / (float(ADC) + 0.5) * 1024.0; 

  return results;

/*  
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(75); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  #ifdef DEBUG
    Serial.print("low = ");
    Serial.print(low);
    Serial.print("\t high = ");
    Serial.print(high);
    Serial.print("\t result = ");
    Serial.print(result);
    Serial.print("\t return ");
    Serial.println(1125.3 / (float)result);
  #endif
  
  return (1125.3 / (float)result); // Vcc in volts
*/
}



#endif
