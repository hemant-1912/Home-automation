# Home-automation
# Home Automation with ESP32 and React Native

This project allows you to control home devices (Cooler, Fan, Light, Heater) remotely using a mobile app built with React Native and an ESP32 microcontroller. Communication between the app and ESP32 is handled via MQTT for real-time updates and control.

## Features

- Control Cooler, Fan, Light, and Heater from your phone
- Real-time device status updates using MQTT
- Easy to set up and extend for more devices

## Project Structure

| File/Folder         | Description                                   |
|---------------------|-----------------------------------------------|
| `devices.tsx`       | React Native app code for device control      |
| `esp32/` or `esp32_code.ino` | ESP32 firmware for device management |
| `.env.example`      | Example environment variables (no secrets)    |
| `README.md`         | Project documentation                         |

## Getting Started

### Prerequisites

- Node.js and npm
- React Native CLI
- ESP32 board and USB cable
- MQTT broker (e.g., Mosquitto)
- WiFi credentials

### ESP32 Setup

1. Open the ESP32 code (`esp32_code.ino`) in Arduino IDE or PlatformIO.
2. Enter your WiFi and MQTT broker details in the code.
3. Upload the code to your ESP32 board.

### React Native App Setup

1. Copy `devices.tsx` into your React Native project.
2. Install dependencies:
3. Update MQTT broker details in the app as needed.
4. Run the app:
or

### MQTT Broker

- Set up your MQTT broker and make sure both the ESP32 and the app can connect to it.
- Update the broker address, port, username, and password in both the ESP32 code and the app.

## Usage

- Use the app to turn devices ON or OFF.
- Device states update in real time as messages are sent and received via MQTT.

## Contributing

Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

---
