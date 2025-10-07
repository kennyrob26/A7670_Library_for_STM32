# A7670 Library for STM32
A library for the A7670 module that uses the HAL library. This library uses UART communication by sending AT commands.  
## Features and objectives
**AT Commands**
- [X] Send and receive AT commands
  
**GNSS**
- [X] Enable GNSS module
- [X] Use A-GPS server for faster fixation
- [X] Receive data from the GPS module
- [ ] Process NMEA data
  - [X] Latitude
  - [X] Longitude
  - [X] Speed
  - [ ] UTC Time
  - [ ] altitude
  - [ ] course

**MQTT**
- [X] Enable MQTT module
- [X] Acquire Client
- [X] Connect to broker
- [ ] SSL and TSL communication
- [X] Publish MQTT Messages
- [X] Subscribe MQTT Topics
