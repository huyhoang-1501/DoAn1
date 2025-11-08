<div align="center">
  <h1>Smart Alarm Clock & Weather Station</h1>
  <p>
    <img src="https://img.shields.io/badge/Microcontroller-ESP8266-blueviolet?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP8266">
    <img src="https://img.shields.io/badge/RTC-DS1307-green?style=for-the-badge&logo=clock&logoColor=white" alt="RTC DS1307">
    <img src="https://img.shields.io/badge/Sensor-DHT22-orange?style=for-the-badge&logo=temperature&logoColor=white" alt="DHT22">
    <img src="https://img.shields.io/badge/Display-LCD%20I2C%2020x4-blue?style=for-the-badge&logo=display&logoColor=white" alt="LCD">
    <img src="https://img.shields.io/badge/Cloud-Firebase-critical?style=for-the-badge&logo=firebase&logoColor=white" alt="Firebase">
    <img src="https://img.shields.io/badge/Language-C%2B%2B-blue?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++">
  </p>

  ---
  <p>
    <a href="#-overview">Overview</a> •
    <a href="#-key-features">Key Features</a> •
    <a href="#-project-structure">Project Structure</a> •
    <a href="#-setup-guide">Setup Guide</a> •
    <a href="#-wiring-diagram">Wiring Diagram</a> •
    <a href="#-web-dashboard">Web Dashboard</a> •
    <a href="#-demo-video">Demo Video</a>
  </p>
  ---
</div>

<br>

## Overview

**`Smart_Alarm_Clock`** is a **smart alarm clock + mini weather station**, integrating:

- **Real-time clock display** (hour, minute, second, day, month, year) using **RTC DS1307**
- **Temperature & humidity measurement** via **DHT22**
- **5 independently configurable alarms** (day/month/hour/minute)
- **Control via 4 push buttons** (Up, Down, Menu, Alarm Off)
- **Data upload to Firebase** (temperature, humidity, alarms)
- **Real-time Web Dashboard** (React + Chart.js)

> **Use case**: Multifunctional desk clock with environmental monitoring.

<br>

## Key Features

| Feature | Description |
|--------|-------------|
| **Accurate Clock** | RTC DS1307 keeps time even when powered off |
| **5 Independent Alarms** | Set day, time, and enable/disable individually |
| **DHT22 Sensor** | Measures temperature (±0.5°C) and humidity (±2%) |
| **20x4 I2C LCD** | Clear display of time, sensor data, and menu |
| **Push-button Control** | Up/Down/Menu/Alarm Off for intuitive navigation |
| **Firebase Realtime DB** | Uploads data every 30s, syncs alarm settings |
| **Web Dashboard** | View live charts, configure alarms remotely |

<br>

## Setup Guide

### Software Requirements
- **Arduino IDE** (recommended)
- **Libraries** (install via Arduino IDE → Library Manager):
  ```bash
  - ESP8266WiFi
  - FirebaseESP8266
  - RTClib (by Adafruit)
  - DHT sensor library for ESPx (by beegee-tokyo)
  - LiquidCrystal_PCF8574

## Demo Video
[![Demo Video](https://github.com/user-attachments/assets/6b9afca4-bb96-449a-a765-82d8ad1d4fc7)](https://youtu.be/4SuPDmZ4MYU)


<div align="center">
  <p><strong>© 2025 – HCMUTE Senior Project Team</strong></p>
  <p><i>Nguyễn Phạm Huy Hoàng - 22161125</i> | <i>Trần Nguyễn Gia Huy - 22161129</i></p>
  <p><em>Precision time. Environmental awareness. Smarter living.</em></p>
</div>