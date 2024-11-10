# Serialization

## Serialization of LPS devices
When communication between the LPS controller and its antennae device data must be serialized.
This documentation serves to provide a clear instruction on how the LPS antenna serializes the data of a single LPS device.

An LPS device contains information about its ID (16-bit unsigned number) and its RSSI (8-bit signed number). The LPS standard specifies that a serialized LPS device consists of three byte of information:
1. upper 8 bit of ID
2. lower 8 bit of ID
3. all 8 bit of RSSI

This standard ensures correct interpretation of the transmitted data and serves as a basis for the deserialization process.

**This standard does not guarantee that there wont be additional fields added in future revisions.**