  #define DEBUG 1          //enable debug msg, sent to serial port  
  #define debug_port SerialUSB

  #ifdef DEBUG
    #define debug_print(x)  debug_port.print(x)
  #else
    #define debug_print(x)
  #endif


void setup() {
  
// Ignition detection
    pinMode(PIN_S_DETECT, INPUT); // Initialize pin as input

}

void loop() {
 
// Check If Ignition is on
  if (digitalRead(PIN_S_DETECT) == LOW)
    debug_print(F("Ignition detected!"));
}
