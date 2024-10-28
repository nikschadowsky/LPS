#include <BLEDevice.h>

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Scanning BLE devices...");
  // initialize the ble environment with an empty device name
  BLEDevice::init("");

  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults* results = scan->start(20, false);

  for(int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice device = results->getDevice(i);
    int rssi = device.getRSSI();

    Serial.println(device.toString().c_str());
    Serial.println("Device: ");
    Serial.println(device.getAddress().toString().c_str());
    Serial.println("RSSi: ");
    Serial.println(rssi);
  }
}

void loop() {

}
