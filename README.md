# Telemtetry for the Formula Student
## Telemetry
Telemetry is a technology that enables remote measurement and monitoring.This technology is interesting for a race vehicle as it allows live readings from the car's sensors to be monitored directly from the side of the track. With such a system, the data from all the sensors are easily accessible, and the engineers can adjust the car's parameters during the test sessions to increase the car's performance. A telemetry system is also helpful in improving the driver's skills, providing measurements such as GPS, speed, pedal level, steering angle, etc. Direct visualization of measurements also allows problems to be identified before they can cause an accident or damage the car.
## Bachelor's Project
This project aims to develop and test a telemetry system for the Formula Student car of the HES-SO Valais-Wallis. This thesis deals with the embedded part of the system and the communication with the remote PC. This project was carried out in collaboration with a Business Information Technology student working on the software to display the telemetry system's data.
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
The realeae branch contains the project's outcome.

The software folder contains the code of the nRF5340, the busmaster scirpts used for the tests and a configuration file example. The hardware folder contains the Altium projects, the Bill of material and the Fusion 360 file for the antenna holder. The Report folder contains the report of the project. The Miscellaneous folder contains the presentations, data and directives
