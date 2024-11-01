#include <Arduino.h>
#include <BLEDevice.h>
#include "LPSDevice.h"
#include "LPSScanner.h"

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Starting");
  // initialize scanner
  init("");
  std::vector<LPSDEVICE> devices = scan();

  for(LPSDEVICE device : devices) {
    Serial.println(getDeviceFormatted(device).c_str());
  }

  Serial.println("\nDone!");
}



void loop() {
}
