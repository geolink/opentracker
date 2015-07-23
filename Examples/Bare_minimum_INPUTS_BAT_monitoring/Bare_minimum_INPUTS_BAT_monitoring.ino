
  #define DEBUG 1          //enable debug msg, sent to serial port  
  #define debug_port SerialUSB

  #ifdef DEBUG
    #define debug_print(x)  debug_port.print(x)
  #else
    #define debug_print(x)
  #endif

// Variables will change:
int outputValue;
int sensorValue;


void setup() {
  // put your setup code here, to run once:

}

void loop() {
// Read VIN Value
      // read the analog in value:
      sensorValue = analogRead(AIN_S_INLEVEL);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      debug_print(F("VIN = " ));
      debug_print(outputValue);
      debug_print(F("V ("));
      debug_print(sensorValue);
      debug_print(F(")"));
      debug_port.println(" ");

      
// Read IN1 Value
      // read the analog in value:
      sensorValue = analogRead(AIN_EXT_IN1);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      debug_print(F("IN1 = " ));
      debug_print(outputValue);
      debug_print(F("V ("));
      debug_print(sensorValue);
      debug_print(F(")"));
      debug_port.println(" ");

// Read IN2 Value
      // read the analog in value:
      sensorValue = analogRead(AIN_EXT_IN2);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      debug_print(F("IN2 = " ));
      debug_print(outputValue);
      debug_print(F("V ("));
      debug_print(sensorValue);
      debug_print(F(")"));
      debug_port.println(" ");
      
      delay(1000);
}
