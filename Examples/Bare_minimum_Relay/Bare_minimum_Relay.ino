void setup() {
    // Relay output
    pinMode(PIN_C_OUT_1, OUTPUT); // Initialize pin as output
    digitalWrite(PIN_C_OUT_1, LOW); // Set PIN LOW
    pinMode(PIN_C_OUT_2, OUTPUT); // Initialize pin as output
    digitalWrite(PIN_C_OUT_2, LOW); // Set PIN LOW
}

void loop() {

  digitalWrite(PIN_C_OUT_1, HIGH);   // switch the relay on
  digitalWrite(PIN_C_OUT_2, HIGH);   //  switch the relay on

  delay(3000);               // wait 

  digitalWrite(PIN_C_OUT_1, LOW);   // switch the relay off
  digitalWrite(PIN_C_OUT_2, LOW);   // switch the relay off

  delay(3000);               // wait 
}
