  #include <avr/dtostrf.h>  
  
void setup() {  
  
  //setup led pin  
  pinMode(PIN_POWER_LED, OUTPUT); // Set LED as Output  
  digitalWrite(PIN_POWER_LED, LOW); // Set LED initially off  
}  
  
void loop() {  
  
  // Switch the Power LED    
  digitalWrite(PIN_POWER_LED, HIGH);  
  delay(800);  
  digitalWrite(PIN_POWER_LED, LOW);  
  delay(800);   
  
}  
