<div align="center">
  <h1>Smart Alarm Clock & Weather Station</h1>
  <p>
    <img src="https://img.shields.io/badge/Microcontroller-ESP8266-blueviolet?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP8266">
    <img src="https://img.shields.io/badge/RTC-DS1307-green?style=for-the-badge&logo=clock&logoColor=white" alt="DS1307">
    <img src="https://img.shields.io/badge/Sensor-DHT22-orange?style=for-the-badge&logo=temperature&logoColor=white" alt="DHT22">
    <img src="https://img.shields.io/badge/Display-LCD%2020x4%20I2C-blue?style=for-the-badge&logo=display&logoColor=white" alt="LCD I2C">
    <img src="https://img.shields.io/badge/Control-4%20Push%20Buttons-teal?style=for-the-badge&logo=keyboard&logoColor=white" alt="Buttons">
    <img src="https://img.shields.io/badge/Buzzer-Active%20Buzzer-red?style=for-the-badge&logo=speaker&logoColor=white" alt="Buzzer">
    <img src="https://img.shields.io/badge/Cloud-Firebase-critical?style=for-the-badge&logo=firebase&logoColor=white" alt="Firebase">
    <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++">
  </p>

  ---
  <p>
    <a href="#-overview">Overview</a> •
    <a href="#-key-features">Key Features</a> •
    <a href="#-setup-guide">Setup Guide</a> •
    <a href="#-web-dashboard">Web Dashboard</a> •
    <a href="#-screenshots">Screenshots</a> •
    <a href="#-development-team">Development Team</a>
  </p>
  ---
</div>

<br>

## Overview

**Smart Alarm Clock & Weather Station** is a multifunctional IoT device that combines a **highly accurate alarm clock** with a **mini weather station**. Built on the **ESP8266 (NodeMCU)**, it operates completely offline while offering full two-way synchronization with the cloud.

**Real-time functions include:**
- Precise timekeeping (date + time down to seconds) using **DS1307 RTC** with battery backup
- Temperature & humidity measurement via **DHT22**
- **Five fully independent alarms** (each with day/month/hour/minute + enable/disable)
- Intuitive navigation using **4 physical push buttons**
- Live data streaming & alarm synchronization with **Firebase Realtime Database**
- Clear 20×4 I²C LCD display showing all information at a glance
- Remote control and monitoring via a responsive **Web Dashboard**

> **Target Application**: Smart bedside/desk clock, personal reminder system, indoor environmental monitoring.

<br>

## Key Features

| Feature                            | Description                                                                                                 |
|------------------------------------|-------------------------------------------------------------------------------------------------------------|
| **Ultra-Accurate Clock**           | DS1307 RTC + CR2032 battery → keeps perfect time even during power outages                                 |
| **5 Independent Date-Based Alarms**| Each alarm can be set to a specific date (day/month) + time, perfect for birthdays, appointments, etc.     |
| **Two-Way Firebase Sync**          | Change alarms on the web → device updates in < 30 s (and vice versa)                                         |
| **DHT22 Environmental Sensor**    | Measures temperature (±0.5 °C) and humidity (±2 %) every 5 s, uploads to Firebase every 30 s               |
| **20×4 I²C LCD Display**           | Clean 4-line interface: Date / Time / Temperature / Humidity                                               |
| **4-Button Intuitive Control**     | Up / Down / Menu (Select/Save) / Alarm Off                                                                  |
| **Buzzer Alarm**                   | Loud continuous buzzer when alarm triggers; instantly silenced with dedicated button                       |
| **Auto Reconnect System**          | Wi-Fi & Firebase automatically reconnect every 10 s if connection drops                                    |
| **Web Dashboard**                  | Real-time charts + full remote alarm configuration from any device                                          |

<br>

## Setup Guide

### Hardware Requirements
- ESP8266 NodeMCU (or any ESP8266 development board)
- DS1307 RTC module + CR2032 coin cell battery
- DHT22 (AM2302) temperature & humidity sensor
- 20×4 I²C LCD (address 0x27)
- 4× push buttons (with internal pull-ups)
- Active buzzer
- Stable 5 V power supply

### Pinout Configuration (NodeMCU ESP8266)

| Component                  | ESP8266 Pin   | Note                          |
|----------------------------|---------------|-------------------------------|
| DHT22 Data                 | D0 (GPIO16)   |                               |
| Buzzer                     | D7 (GPIO13)   |                               |
| Button UP                  | D4 (GPIO2)    | Internal pull-up              |
| Button DOWN                | D3 (GPIO0)    | Internal pull-up              |
| Button MENU / SELECT       | D5 (GPIO14)   | Internal pull-up              |
| Button ALARM OFF           | D6 (GPIO12)   | Internal pull-up              |
| LCD & DS1307 I²C SDA       | D2 (GPIO4)    | Shared I²C bus                |
| LCD & DS1307 I²C SCL       | D1 (GPIO5)    | Shared I²C bus                |

### Required Arduino Libraries
- `ESP8266WiFi` (built-in)
- `Firebase-ESP8266` by Mobizt
- `RTClib` by Adafruit
- `DHT sensor library for ESPx` by beegee-tokyo
- `LiquidCrystal_PCF8574` by mathertel

### Firebase & Web Setup
1. Create a Firebase project → enable **Realtime Database** (legacy token mode)
2. Copy `Database URL` and `Database Secret` into the Arduino sketch
3. Structure in Firebase:

esp8266_data/
├── sensors {temperature, humidity, timestamp}
└── alarms/
├── alarm1 {day, month, hour, minute, enabled}
├── alarm2 ...
└── alarm5

4. Deploy the web dashboard using Firebase Hosting, Vercel, Netlify, or simply open `index.html` locally

<br>

## Web Dashboard
- Clean, responsive design (mobile + desktop)
- Real-time temperature & humidity line charts (Chart.js)
- Full control over all 5 alarms (set date/time + enable/disable)
- Live sync with the physical device

<br>

## Screenshots

### Main Screen
![Main Screen](https://github.com/user-attachments/assets/3bc66a63-5bc0-4181-91ff-cfbda36b9b04)

### Main menu interface
![Main menu interface](https://github.com/user-attachments/assets/2d91fd80-8b07-4b9c-a16d-0db6761a2f87)

### Menu Date Setting
![Menu Date Setting](https://github.com/user-attachments/assets/7165d6d2-985f-4515-829a-999ce1823294)

### Menu Time Setting
![Menu Time Setting](https://github.com/user-attachments/assets/246aa5b9-9f61-4bd4-85f0-849f7eaf18c9)

### Alarm Configuration
![Alarm Configuration](https://github.com/user-attachments/assets/f8bd0aa9-ca5f-459f-a6b3-f9c68133d76a)

### Alarm Triggered
![Alarm Active](https://github.com/user-attachments/assets/97a1a2a3-9f8e-4e46-b096-8c9642ec9986)

### Actual Product
![Web Dashboard](https://github.com/user-attachments/assets/e10c0a1a-a4e0-49a7-ae36-266e36503126)

### Web Dashboard (Desktop & Product)
![Web Dashboard](https://github.com/user-attachments/assets/8ceb3cfd-90eb-44b3-89a8-9b394851385c)

### Demo Video
[![Demo Video](https://img.youtube.com/vi/RZyvXtFgk40/0.jpg)](https://www.youtube.com/shorts/RZyvXtFgk40)

<br>

<div align="center">

**© 2025 – Ho Chi Minh City University of Technology and Education (HCMUTE)**  
**Electronics & Communication Engineering Technology**

**Nguyễn Phạm Huy Hoàng – 22161125**  
**Trần Nguyễn Gia Huy – 22161129**

<em>“Precise time. Environmental awareness. Smarter mornings.”</em>

</div>