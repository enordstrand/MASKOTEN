// Demo: NMEA2000 library. Send main water tank data to the bus.

#include <Arduino.h>
#include <MemoryFree.h>
#define N2k_SPI_CS_PIN 10
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <N2kMessages.h>

#define gndPin 17 // Ground Pin
#define echoPin 16 // Echo Pin
#define trigPin 15 // Trigger Pin
#define voltagePin 14 // vcc Pin
#define LEDPin 13 // Onboard LED

int maximumRange = 500; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration; // Duration used to calculate distance
float distance;

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM={127505L,0};

void setup() {
  Serial.begin (115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(voltagePin, OUTPUT);
  digitalWrite(voltagePin, HIGH);
  pinMode(gndPin, OUTPUT);
  digitalWrite(gndPin, LOW);
  pinMode(LEDPin, OUTPUT); // Use LED indicator (if required)
  
  // Set Product information
  NMEA2000.SetProductInformation("00000003", // Manufacturer's Model serial code
                                 100, // Manufacturer's product code
                                 "ArduWater monitor",  // Manufacturer's Model ID
                                 "1.0.0.0 (2017-04-14)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2016-12-31)" // Manufacturer's Model version
                                 );
  // Set device information
  NMEA2000.SetDeviceInformation(1337, // Unique number. Use e.g. Serial number.
                                150, // Device function=Fluid Leve. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                75, // Device class=Sensor Communication Interface. See codes on  http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20&%20function%20codes%20v%202.00.pdf
                                1337 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf                               
                               );
  // Uncomment 2 rows below to see, what device will send to bus. Use e.g. OpenSkipper or Actisense NMEA Reader                           
  Serial.begin(115200);
  NMEA2000.SetForwardStream(&Serial);
  // If you want to use simple ascii monitor like Arduino Serial Monitor, uncomment next line
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.

  // If you also want to see all traffic on the bus use N2km_ListenAndNode instead of N2km_NodeOnly below
  NMEA2000.SetMode(tNMEA2000::N2km_NodeOnly,23);
  // NMEA2000.SetDebugMode(tNMEA2000::dm_Actisense); // Uncomment this, so you can test code without CAN bus chips on Arduino Mega
  // NMEA2000.EnableForward(false); // Disable all msg forwarding to USB (=Serial)
  NMEA2000.SetN2kCANMsgBufSize(2);
  NMEA2000.SetN2kCANSendFrameBufSize(30);
  // Here we tell library, which PGNs we transmit
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.Open();
}


void loop() {
  SendN2kWaterFluid();
  NMEA2000.ParseMessages();
}

int ReadTankPercent() {
  /* The following trigPin/echoPin cycle is used to determine the
  distance of the nearest object by bouncing soundwaves off of it. */ 
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
 
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
 
  //Calculate the distance (in cm) based on the speed of sound.
  distance = duration/58.2;

  if (distance >= maximumRange || distance <= minimumRange){
    /* Send a negative number to computer and Turn LED ON 
    to indicate "out of range" */
    //Serial.println("-1");
    //digitalWrite(LEDPin, HIGH); 
  } else {
    /* Send the distance to the computer using Serial protocol, and
    turn LED OFF to indicate successful reading. */
    //Serial.println(distance);
    //digitalWrite(LEDPin, LOW); 
  }
  Serial.println(distance);
  // return ((distance*(-10.204))+120.4); // Tank level in % of full tank.
  //return -2;
  return 42;
}

double ReadTankCapacity() {
  return 100; // Tank Capacity in litres
}

#define WindUpdatePeriod 1000

void SendN2kWaterFluid() {
  static unsigned long WindUpdated=millis();
  tN2kMsg N2kMsg;

  if ( WindUpdated+WindUpdatePeriod<millis() ) {
    WindUpdated=millis();
    //Serial.print("freeMemory()=");
    //Serial.println(freeMemory());
    SetN2kFluidLevel(N2kMsg, 1, N2kft_Water, ReadTankPercent(), ReadTankCapacity());
    NMEA2000.SendMsg(N2kMsg);
  }
}

