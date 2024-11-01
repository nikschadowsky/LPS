#include <Arduino.h>
#include <BLEDevice.h>
#include "LPSDevice.h"

const int SCAN_TIME = 5;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Scanning BLE devices...");
  // initialize the ble environment with an empty device name
  BLEDevice::init("");
  Serial.println("working on it...");

  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(SCAN_TIME, false);

  for (int i = 0; i < results.getCount(); i++) {
    BLEAdvertisedDevice device = results.getDevice(i);

    if (device.haveManufacturerData() && device.getManufacturerData().rfind(LPS_DEVICE_MANUFACTURER_PREFIX, 0) == 0) {
      int rssi = device.getRSSI(); 

      Serial.println(device.toString().c_str());
      Serial.println(device.getManufacturerData().c_str());
      Serial.println(device.getRSSI());
    }
  }
  
  Serial.println("\nDone!");
}

// LPS-Device(ID: , RSSI: )
/*String getDeviceDescription(BLEAdvertisedDevice *device) {
  String result = "LPS-Device(";
  result.concat("ID: ");
  result.concat()
  return "LPS-Device(" + "RSSI: " + device->getRSSI() + ")";
}*/


void loop() {
}
