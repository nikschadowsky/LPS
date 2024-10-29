# LPS - Local Positioning System
The Local Positioning System (LPS) is a project created for the course Gebäudeautomation at the NORDAKADEMIE.
It stands as a solution for indoor positioning in situations where GPS isn't an option - especially in large building complexes like offices.

# How it works
LPS is based on Bluetooth multilateration. It listens for Bluetooth Low Energy (BLE) Advertisement packages and uses their 
Received Signal Strength Indication (RSSI) for distance estimation. Using four antennae we can calculate the estimated position in 3D-space. 

# But why?
With our system we aim to provide a solution to help firecrews navigate in buildings and rooms full of smoke. It also allows for position history 
in cases were the signal is lost and a firecrew member must be rescued.
Our system is built to be easily installable and configuable and can be used in a variety of different situations.

# Is it already functioning?
Not yet. Since our work on the project just started there are several steps we need to take before we can build a working prototype. A feasibility study 
must be taken to determine the accuracy of our system. Also a distance calibration and threshold analysis have to be completed before we can build the full system prototype.