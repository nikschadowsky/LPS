# Protocols

LPS defines several protocols to ensure correct communication between the devices.


## LPS-Device Find Protocol v0.1
The LPS specifies a required format for BLE Advertisement packets to be recognized by compatible receivers.
In this implementation, the manufacturer data section of the packed begins with the bytes `0x504C53`. It represents the ASCII string `LPS` because the Company Identifier is in little-endian format according to the Bluetooth Core Specification, flipping the first two characters.
The next two bytes specify a numeric value for the **ID** of a device. A device identification ends with another byte equal to `0x23`, indication the end marker of the parsing of an LPS device.

**This protocol does not guarantee that there wont be additional fields added in future versions and revisions.**

## LPS Antenna-Controller Communication Protocol
The LPS specifies a required format for communication between an LPS antenna and its corresponding controller. 
Every LPS controller creates a private wifi network which all antennae are configured to connect to on startup. 
Every LPS antenna provides a webserver on port 80.
This port is used to conduct all communication with an antenna. 
Each antenna serves two endpoints:
- `/api/scan`:
... an HTTP GET request to this endpoint starts the scan of the surrounding LPS BLE devices and returns a status 200 octet stream containing a serialized version of each device. 
The serialized data contains at least 3 bytes and always ends on 3 NULL bytes `0x000000`.
Each LPS device is serialized according to the [Serialization standard](serialization.md). The total serialized and, ultimately, transmitted bytestream consists of the serialized representation for each device concatenated and terminated by three NULL bytes:
1. ``0..n`` device data serialized (3 bytes each)
2. `0x000000`
An HTTP GET request to this endpoint will fail with a status 403 if and only if this antenna is in configuration mode

- `/api/config`:
This endpoint serves to allow for toggling the configuration mode of an antenna. Under normal operation and on startup the antenna is in OPERATION mode. This enables scanning of nearby LPS devices. Requesting an HTTP POST on this endpoint toggles the configuration mode and returns a status 202. An indication LED can be installed on GPIO pin 23 and will be lit if the device is in configuration mode. An HTTP GET request on this endpoint will return a basic HTML representation of the current state of the configuration mode. This will return a status 200. 

**Any other HTTP request on any other endpoint will result in an HTTP 403 response.**

**This protocol does not guarantee that there wont be additional fields added in future versions and revisions.**