

# Soil Moisture Monitoring System with Automatic Water Pump

## Project Overview
The Soil Moisture Monitoring System with Automatic Water Pump is an embedded system designed to monitor soil moisture levels and automatically control a water pump for irrigation. The system helps prevent overwatering and underwatering by supplying water only when the soil moisture falls below a predefined threshold.

This project is suitable for smart agriculture, gardening, and small-scale irrigation systems, especially in areas where efficient water usage is important.

---

## Objectives
- Monitor soil moisture levels in real time  
- Automatically activate a water pump when soil is dry  
- Turn off the pump when adequate moisture is detected  
- Reduce human intervention in irrigation  
- Promote efficient water management  

---

## ⚙️ System Components
### Hardware
- ESP32 Development Board  
- Soil Moisture Sensor  
- Relay Module  
- Water Pump  
- Power Supply  
- Breadboard and Jumper Wires  

### Software
- Arduino IDE  
- Embedded C / Arduino Programming Language  

---

## System Architecture
The system consists of a soil moisture sensor connected to the ESP32. The ESP32 reads analog data from the sensor and compares it with a predefined moisture threshold. When the soil is dry, the ESP32 activates the relay module, which turns on the water pump. Once sufficient moisture is detected, the pump is automatically switched off.

---

## System Workflow
1. Soil moisture sensor measures moisture level  
2. ESP32 reads and processes sensor data  
3. Moisture value is compared with threshold  
4. If soil is dry, relay activates the water pump  
5. If soil is wet, relay deactivates the pump  

---

## Features
- Automatic irrigation control  
- Real-time soil moisture monitoring  
- Low power consumption  
- Simple and cost-effective design  
- Scalable for larger irrigation systems  

---

## Applications
- Smart agriculture  
- Home gardening  
- Greenhouse irrigation  
- Crop monitoring systems  

---

## How to Run the Project
1. Connect all hardware components according to the circuit diagram  
2. Install Arduino IDE  
3. Select the correct ESP32 board and COM port  
4. Upload the source code to the ESP32  
5. Power the system and observe automatic pump control  

---

## Future Improvements
- Integration with IoT platforms (Blynk, ThingSpeak, Firebase)  
- Mobile and web dashboard for remote monitoring  
- SMS or email alert system  
- Solar-powered operation  
- Multiple sensor support for large farms  

---

## Author
**Temitope Okubule**  
Computer Scientist  
Embedded Systems and IoT Enthusiast  

---

## License
This project is developed for academic purposes and learning. You are free to modify and improve it with proper attribution.