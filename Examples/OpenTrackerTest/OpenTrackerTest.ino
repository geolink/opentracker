/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */

// Include Scheduler since we want to manage multiple tasks.
#include <Scheduler.h>

// experimental CAN support
#include <due_can.h>
#define TEST_CAN

//Leave defined if you use native port, comment if using programming port
#define Console SerialUSB
#define SerialGPS Serial1
#define SerialGSM Serial2

// the setup routine runs once when you press reset:
void setup() {
  // init serial ports before power on to GSM/GPS
  SerialGSM.begin(115200);
  SerialGPS.begin(9600);

  // initialize the digital pin as an output.
  pinMode(PIN_POWER_LED, OUTPUT);
  digitalWrite(PIN_POWER_LED, HIGH);

  pinMode(PIN_C_OUT_1, OUTPUT);
  digitalWrite(PIN_C_OUT_1, LOW);
  pinMode(PIN_C_OUT_2, OUTPUT);
  digitalWrite(PIN_C_OUT_2, LOW);

  pinMode(PIN_S_DETECT, INPUT);  

  // initialize GSM pins
  pinMode(PIN_C_PWR_GSM, OUTPUT); 
  digitalWrite(PIN_C_PWR_GSM, LOW);

  pinMode(PIN_C_KILL_GSM, OUTPUT); 
  digitalWrite(PIN_C_KILL_GSM, LOW);

  pinMode(PIN_STATUS_GSM, INPUT);  

  pinMode(PIN_RING_GSM, INPUT);  

  // initialize GPS pins
  pinMode(PIN_STANDBY_GPS, OUTPUT); 
  digitalWrite(PIN_STANDBY_GPS, LOW);

  pinMode(PIN_RESET_GPS, OUTPUT); 
  digitalWrite(PIN_RESET_GPS, LOW);

  Console.begin(115200);

  while (true)  
    if (millis() > 3000) break;

#ifdef TEST_CAN
  Console.print("CAN init...");
  CAN.Transceiver.SetRS(PIN_CAN_RS);
  CAN.init(CAN_BPS_500K);
  Console.print("done!");

  //By default there are 7 mailboxes for each device that are RX boxes
  //This sets each mailbox to have an open filter that will accept extended
  //or standard frames
  int filter;
  //extended
  for (filter = 0; filter < 3; filter++) {
	CAN.setRXFilter(filter, 0, 0, true);
  }  
  //standard
  for (filter = 3; filter < 7; filter++) {
	CAN.setRXFilter(filter, 0, 0, false);
  }

#else
  delay(100);
  gsmPower(1); // force ON

#endif

  // Add "loop2" and "loop3" to scheduling.
  // "loop" is always started by default.
  Scheduler.startLoop(loop2);
  Scheduler.startLoop(loop3);
}

void gsmPower(int forceON)
{
  Console.print("GSM power...");
  if (!digitalRead(PIN_STATUS_GSM))
  {
    digitalWrite(PIN_C_PWR_GSM, HIGH); //enable
    for (int i=0; i<15; i++)
    {
      delay(100);
      if (digitalRead(PIN_STATUS_GSM)) break;
    }
    if (!digitalRead(PIN_STATUS_GSM))
    {
      Console.println("GSM Status not going high!");
      for(;;);
    }
    digitalWrite(PIN_C_PWR_GSM, LOW);
    delay(100);
  }
  else if (forceON)
    Console.println("GSM Status already high!");
  else
  {
    digitalWrite(PIN_C_PWR_GSM, HIGH); //disable
    for (int i=0; i<95; i++)
    {
      delay(100);
      if (!digitalRead(PIN_STATUS_GSM)) break;
    }
    if (digitalRead(PIN_STATUS_GSM))
    {
      Console.println("GSM Status not going low!");
      for(;;);
    }
    digitalWrite(PIN_C_PWR_GSM, LOW);
    delay(100);
  }
  if (!digitalRead(PIN_STATUS_GSM))
    Console.println("- GSM OFF!");
  else
    Console.println("- GSM ON!");
}

void gsmLoop()
{
  // read from port 1, send to port 0:
  if (SerialGSM.available()) {
    int inByte = SerialGSM.read();
    Console.write(inByte); 
  }
  
  // read from port 0, send to port 1:
  if (Console.available()) {
    int inByte = Console.read();
    SerialGSM.write(inByte); 
    if (inByte == '.')
      gsmPower(0);
    if (inByte == '\'')
    {
      digitalWrite(PIN_C_KILL_GSM, HIGH);
      pinMode(PIN_C_KILL_GSM, OUTPUT);
      delay(1000);
      digitalWrite(PIN_C_KILL_GSM, LOW);
    }
    
    if (inByte == '\\')
    {
      digitalWrite(PIN_STANDBY_GPS, !digitalRead(PIN_STANDBY_GPS));
    }
    if (inByte == '|')
    {
      digitalWrite(PIN_RESET_GPS, HIGH);
      pinMode(PIN_RESET_GPS, OUTPUT);
      delay(100);
      digitalWrite(PIN_RESET_GPS, LOW);
    }
  }

  if (!digitalRead(PIN_RING_GSM))
    digitalWrite(PIN_POWER_LED, HIGH);   // turn the LED off
  else
    digitalWrite(PIN_POWER_LED, LOW);    // turn the LED on
}

void gpsLoop()
{
  // read from port 1, send to port 0:
  if (SerialGPS.available()) {
    int inByte = SerialGPS.read();
    Console.write(inByte); 
  }
#if 0
  // read from port 0, send to port 1:
  if (Console.available()) {
    int inByte = Console.read();
    SerialGPS.write(inByte); 
  }
#endif
}

void printFrame(CAN_FRAME &frame)
{
   Console.print("ID: 0x");
   Console.print(frame.id, HEX);
   Console.print(" Len: ");
   Console.print(frame.length);
   Console.print(" Data: 0x");
   for (int count = 0; count < frame.length; count++) {
       Console.print(frame.data.bytes[count], HEX);
       Console.print(" ");
   }
   Console.print("\r\n");
}

// Task no.1: blink LED with 1 second delay.
void loop()
{
  digitalWrite(PIN_POWER_LED, HIGH);

  // IMPORTANT:
  // When multiple tasks are running 'delay' passes control to
  // other tasks while waiting and guarantees they get executed.
  delay(500);

  digitalWrite(PIN_POWER_LED, LOW);
  delay(500);
}

// Task no.2: blink LED with 0.1 second delay.
void loop2()
{

  digitalWrite(PIN_C_OUT_1, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(PIN_C_OUT_2, LOW);   // turn the LED on (HIGH is the voltage level)

  delay(3000);               // wait for a second

  digitalWrite(PIN_C_OUT_1, LOW);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(PIN_C_OUT_2, HIGH);   // turn the LED on (HIGH is the voltage level)

  delay(3000);               // wait for a second
}

int sensorValue = 0;        // value read from the pot
float outputValue = 0;

// Task no.3: accept commands from Serial port
// 'v' read input voltage
void loop3() {
#ifndef TEST_CAN
  gsmLoop();
  gpsLoop();
  yield();
  return;
#endif

#ifdef TEST_CAN
  if (CAN.rx_avail()) {
    CAN_FRAME incoming;
    CAN.get_rx_buff(incoming); 
    printFrame(incoming);
  }
#endif  
  if (Console.available()) {
    char c = Console.read();
#ifdef TEST_CAN
    if (c == 'c') {
      CAN_FRAME frame;
      frame.id = 0x123;
      frame.length = 8;
      frame.data.low = 0xB8C8A8E8;
      frame.data.high = 0x01020304;
      frame.extended = 0;
      
      CAN.sendFrame(frame);
    }
#endif  
    if (c == 'v') {
      // read the analog in value:
      sensorValue = analogRead(AIN_S_INLEVEL);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      Console.print("VIN = " );
      Console.print(outputValue);
      Console.print("V (");
      Console.print(sensorValue);
      Console.println(")");
    }
    if (c == '1') {
      // read the analog in value:
      sensorValue = analogRead(AIN_EXT_IN1);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      Console.print("IN1 = " );
      Console.print(outputValue);
      Console.print("V (");
      Console.print(sensorValue);
      Console.println(")");
    }
    if (c == '2') {
      // read the analog in value:
      sensorValue = analogRead(AIN_EXT_IN2);
      // map it to the range of the analog out:
      outputValue = sensorValue * (242.0f / 22.0f * ANALOG_VREF / 1024.0f);
    
      // print the results to the serial monitor:
      Console.print("IN2 = " );
      Console.print(outputValue);
      Console.print("V (");
      Console.print(sensorValue);
      Console.println(")");
    }
  }
  if (digitalRead(PIN_S_DETECT) == LOW)
    Console.println("Ignition detected!");
  // IMPORTANT:
  // We must call 'yield' at a regular basis to pass
  // control to other tasks.
  yield();
}

