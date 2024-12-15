# Protocols Prototype v1

LPS defines several protocols to ensure correct communication between the devices.


## LPS-Device Find Protocol
The LPS specifies a required format for BLE Advertisement packets to be recognized by compatible receivers.
In this implementation, the manufacturer data section of the packed begins with the bytes `0x504C53`. It represents the ASCII string `LPS` because the Company Identifier is in little-endian format according to the Bluetooth Core Specification, flipping the first two characters.
The next two bytes specify a numeric value for the **ID** of a device. A device identification ends with another byte equal to `0x23`, indication the end marker of the parsing of an LPS device.

**This protocol does not guarantee that there wont be additional fields added in future versions and revisions.**

## LPS Antenna-Controller Communication Protocol
The LPS specifies a required format for communication between an LPS antenna and its corresponding controller. 
Every LPS controller creates a private wifi network which all antennae are configured to connect to on startup. 
Every LPS antenna provides a webserver on port 80.
This port is used to conduct all communication with an antenna. 
Each antenna serves four endpoints:
- `/api/scan`:
... an HTTP GET request to this endpoint starts the scan of the surrounding LPS BLE devices and returns a status 200 octet stream containing a serialized version of each device. 
The serialized data contains at least 3 bytes and always ends on 3 NULL bytes `0x000000`.
Each LPS device is serialized according to the [Serialization standard](serialization.md). The total serialized and, ultimately, transmitted bytestream consists of the serialized representation for each device concatenated and terminated by three NULL bytes:
1. ``0..n`` device data serialized (3 bytes each)
2. `0x000000`
An HTTP GET request to this endpoint will fail with a status 403 if and only if this antenna is in configuration mode

- `/api/config/enable`:
This endpoint serves to allow for enabling the configuration mode of an antenna. Under normal operation and on startup the antenna is in OPERATION mode. This enables scanning of nearby LPS devices. Requesting an HTTP POST on this endpoint enables the configuration mode and returns a status 202. An indication LED can be installed on GPIO pin 23 and will be lit if the device is in configuration mode.

- `/api/config/disable`:
This endpoint serves to allow for disabling the configuration mode of an antenna. Requesting an HTTP POST on this endpoint disables the configuration mode and returns a status 202. An indication LED can be installed on GPIO pin 23 and will be lit if the device is in configuration mode.

- `/api/config`:
An HTTP GET request on this endpoint will return a basic HTML representation of the current state of the configuration mode. This will return a status 200. 

**Any other HTTP request on any other endpoint will result in an HTTP 403 response.**

**This protocol does not guarantee that there wont be additional fields added in future versions and revisions.**

## LPS Communication with other Systems
The LPS allows communication between external systems and the LPS. The communication is always LPS controlled. That means a so called LPS action gets initiated by the controller and has to be handled accordingly. The LPS defines five actions in its current revision:
- `ESP_CONFIG_START`
This action signals to the external system that the LPS controller initiated a (re)configuration. This action serves just as information to the external system and does not need further handling.
- `ESP_CONFIG_REQ`
This action signals to the external system that the LPS controller requires an interation. It requests a ASCII letter A-D that specifies which corner of the room a certain antenna is located at. The antenna will light up through its integrated LED. The response requires a single byte to be transmitted. This action does only occur in a (re)configuration.
- `ESP_POS_DATA_START`
This action signals to the external system that the LPS controller is beginning to send serialized position data. This data is termined by an ending sequence `POS_DATA_END`. This action does not need further handling.
- `ESP_CONFIG_DIST1`
This action signals to the external system that the LPS controller requires an interation. It requests a floating point value in big endian notation. This value corresponds with the distance between the two lit up LEDs. This response requires four bytes to be transmitted. This action does only occur in a (re)configuration
- `ESP_CONFIG_DIST2`
This action signals to the external system that the LPS controller requires an interation. It requests a floating point value in big endian notation. This value corresponds with the distance between the two lit up LEDs. This response requires four bytes to be transmitted. This action does only occur in a (re)configuration.

Both `ESP_CONFIG_DISTX` actions do not share the same action since they need to be distinguishable for the LPS controller. 

The baudrate of this communication is defined to be 115200.

**This protocol does not guarantee that there wont be additional fields added in future versions and revisions.**