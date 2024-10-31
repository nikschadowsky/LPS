#include <BLEDevice.h>

struct LPS_DEVICE {
  
}


const String LPS_DEVICE_MANUFACTURER_PREFIX = "LPS"; // 'LP' are part of the company id while 'S' is the first byte in the data body

//const String LPS_DEVICE_MANUFACTURER_PREFIX  = "7073"; // every LPS ready device must start with this two bytes in its manufacturers data

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

    if (device.haveManufacturerData() && device.getManufacturerData().startsWith(LPS_DEVICE_MANUFACTURER_PREFIX)) {
      int rssi = device.getRSSI(); 

      Serial.println(device.toString().c_str());
      Serial.println(device.getManufacturerData());
      Serial.println(device.getRSSI());

      LPS_DEVICE_MANUFACTURER_PREFIX.getBytes(unsigned char *buf, unsigned int bufsize)
    }
  }
  
  Serial.println("\nDone!");
}

String getDeviceDescription(BLEAdvertisedDevice *device) {

}

void loop() {
}
