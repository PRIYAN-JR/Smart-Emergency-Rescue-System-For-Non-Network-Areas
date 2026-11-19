# Smart-Emergency-Rescue-System-For-Non-Network-Areas
Project Overview

The Smart Emergency Rescue System is a portable, hardware-based emergency communication system designed to assist people in remote locations where mobile network and internet connectivity are unavailable.

The system uses GPS positioning, RF wireless communication and heart-rate monitoring to transmit emergency information to a receiver unit.

Project Objectives

• Provide emergency SOS communication in non-network areas.
• Track geographical location using GPS.
• Monitor heart rate using a MAX30102 sensor.
• Transmit emergency information through 433 MHz RF communication.
• Display received emergency information at the receiver unit.

Hardware Components

Component| Purpose
Arduino Nano| Microcontroller
NEO-6M GPS| Location tracking
MAX30102| Heart-rate monitoring
433 MHz RF Transmitter| Wireless data transmission
433 MHz RF Receiver| Data reception
OLED Display| Display emergency information
SOS Push Button| Emergency activation
Buzzer| Audio alert

Block Diagram
<img width="1248" height="449" alt="Block Diagram" src="https://github.com/user-attachments/assets/6e741577-6fe9-4942-96ab-7e7b53d4bb27" />
"System Block Diagram" (Block%20Diagram.png)

Circuit Diagrams

Transmitter Side

"Transmitter Circuit" (Circuit%20Diagram-Transmitter%20Side.png)

Receiver Side

"Receiver Circuit" (Circuit%20Diagram-Receiver%20Side.png)

Working Principle

1. The user presses the SOS push button during an emergency.
2. The NEO-6M GPS module obtains geographical coordinates.
3. The MAX30102 sensor measures the user's heart rate.
4. The Arduino Nano processes the collected information.
5. Emergency information is transmitted through the 433 MHz RF transmitter.
6. The receiver unit receives the transmitted information.
7. The OLED display shows the received GPS coordinates and heart-rate information.
8. The buzzer provides an audio alert when emergency information is received.

Hardware Implementation

The project consists of two main units:

• Transmitter Unit: Collects GPS location and heart-rate information and sends emergency data using RF communication.
• Receiver Unit: Receives emergency information, displays the readings and activates the buzzer.

"Hardware Prototype" (Hardware.jpg)

Applications

• Forest rescue operations
• Highway emergency assistance
• Remote-area safety
• Disaster management
• Emergency communication in isolated locations

Advantages

• Does not depend on mobile network or internet connectivity for RF communication.
• Portable hardware implementation.
• GPS-based location tracking.
• Wireless emergency data transmission.
• Integrated heart-rate monitoring.

Repository Contents

• "Transmitter.ino" – Transmitter Arduino code.
• "Receiver.ino" – Receiver Arduino code.
• "Block Diagram.png" – System block diagram.
• "Circuit Diagram-Transmitter Side.png" – Transmitter circuit.
• "Circuit Diagram-Receiver Side.png" – Receiver circuit.
• "Hardware.jpg" – Hardware prototype.
• "CONNECTIONS.txt" – Hardware connection details.
• "REPORT.pdf" – Complete project report.

Project Domain

Electronics and Communication Engineering (ECE) | Embedded Systems | Wireless Communication | GPS | Sensor Interfacing

Project Status

Hardware prototype and implementation.

---

Developed as an Electronics and Communication Engineering project.
