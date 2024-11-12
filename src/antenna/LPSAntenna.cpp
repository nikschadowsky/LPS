// #include <Arduino.h>
#include <HardwareSerial.h>
#include <BLEDevice.h>
#include "LPSDevice.h"
#include "LPSScanner.h"
#include "LPSAntennaSender.h"

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Starting");

  // initialize scanner
  LPSANTENNASENDER::initServer();
}

void loop()
{
  LPSANTENNASENDER::handleRequests();
}
