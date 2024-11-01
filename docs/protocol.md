# Protocols

LPS defines several protocols to ensure correct communication between the devices.


## LPS-Device Find Protocol v0.1
The LPS specifies a required format for BLE Advertisement packets to be recognized by compatible receivers.
In this implementation, the manufacturer data section of the packed begins with the bytes `0x504C53`. It represents the ASCII string `LPS` because the Company Identifier is in little-endian format according to the Bluetooth Core Specification, flipping the first two characters.
The next two bytes specify a numeric value for the **ID** of a device. A device identification ends with another byte equal to `0x23`, indication the end marker of the parsing of an LPS device.

**This protocol does not guarantee that there wont be additional fields added in future revisions.**

## LPS Antenna-Controller Communication Protocol