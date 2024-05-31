# Telemetry for the Formula Student
## Telemetry
Telemetry is a technology that enables remote measurement and monitoring.This technology is interesting for a race vehicle as it allows live readings from the car's sensors to be monitored directly from the side of the track. With such a system, the data from all the sensors are easily accessible, and the engineers can adjust the car's parameters during the test sessions to increase the car's performance. A telemetry system is also helpful in improving the driver's skills, providing measurements such as GPS, speed, pedal level, steering angle, etc. Direct visualization of measurements also allows problems to be identified before they can cause an accident or damage the car.
## Bachelor's Project
This project aimed to develop and test a telemetry system for the Formula Student car of the HES-SO Valais-Wallis. This thesis deals with the embedded part of the system and the communication with the remote PC. This project was carried out in collaboration with a Business Information Technology student working on the software to display the telemetry system's data.
## MSE PA - Optimizations
This project aimed to enhance the existing system by incorporating additional functionalities
and improving its performance. The objectives of the project are as follows :
- **Addition of a Configurable Filter for the CAN Bus:** The objective of this
section is to implement a filter on the CAN input to allow the passage of other
messages without compromising the performance of the telemetry system.
- **Telemetry System Control via CAN Bus:**
 Currently, recording is initiated and
stopped using a button connected to the system. The objective of this section
is to add the capability to control these functions via the CAN bus.
- **Integration of Time and Date:** The objective of this section is to replace the
current timestamp (relative time since recording started) with absolute time.
- **System Optimization to Enable the Recording of More Data at a Higher
Frequency:** The goal of this section is to push the limits of the number of CAN
messages the system can handle and increase the recording frequency on the SD
card.

The first two points will be implemented on the existing hardware. This is because
the VRT team will need these modifications for the summer 2024 races, and these
optimisations do not require new hardware. For the last two points, the same hardware
is reused, but it is duplicated in order to have one device for the transmission and the
other for the recording.
## Project summary
The telemetry device is connected to the sensors on the car's CAN bus. The
device also includes a GPS module. The CAN bus and GPS module data are
transmitted to the base station via Wi-Fi. The data are also saved on an SD card.
The system is designed to be generic and completely configurable. New sensors
can easily be added to the system by only changing the configuration file.
The device has been successfully developed and tested, and all the objectives
have been reached. The tests showed that the system can transmit data over a
750 meters distance.
## Git Content
The realease branch contains the project's outcome.

- The software folder contains the code of the nRF5340, the busmaster scirpts used for the tests and a configuration file example. The telemetry_system folder contains the code for the system using one device. The telemetry_system_recorder contains the code of the recoridng device for the splitted system and the telemetry_system_transmitter contains the code of the transmitting device for the splitted system. 

- The hardware folder contains the Altium projects, the Bill of material and the Fusion 360 file for the antenna holder.
- The Report folder contains the report of the project. 
- The Miscellaneous folder contains an example of the configuration file, a test and measurement file etc...
