#include <BLEDevice.h>


const String TARGET_DEVICE_NAME = "Tab S9 FE von Nik";

const int SCAN_TIME = 5;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Scanning BLE devices...");
  // initialize the ble environment with an empty device name
  BLEDevice::init("");

  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults* results = scan->start(SCAN_TIME, false);

  for (int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice device = results->getDevice(i);

    if (device.haveName() && device.getName() == TARGET_DEVICE_NAME) {
      int rssi = device.getRSSI(); 

      Serial.println(device.toString().c_str());
      Serial.println("Device: ");
      Serial.println(device.getAddress().toString().c_str());
      Serial.println("RSSi: ");
      Serial.println(rssi);
    }
  }
  
  Serial.println("\nDone!");
}

void loop() {
}
