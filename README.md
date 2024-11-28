
# Sensor Calibration System

This project is designed for managing and calibrating multiple sensors connected to an Arduino device.
It includes functionality for reading raw sensor data, calibrating it, and saving the calibration data to EEPROM.

## Features

- **Calibration Support**:
  - Add calibration points for sensors with linear interpolation and extrapolation.
  - Reset calibration data when needed.
  - Store calibration data persistently in EEPROM.

- **Multi-Sensor Management**:
  - Manage multiple sensors with individual calibration data.
  - Dynamically display sensor data (raw and calibrated values).

- **Command-Based Control**:
  - Calibrate sensors via serial commands.
  - Reset individual sensors using predefined commands.

## Serial Commands

### Calibration Commands
- **Add Calibration Point**:

Example: cal1 500 1000

  - Adds a calibration point for Sensor 1. Replace `1` with `2` for Sensor 2.

### Reset Commands
- **Reset Calibration Data**:

Example: reset1

  - Resets all calibration points for Sensor 1. Replace `1` with `2` for Sensor 2.

## Usage

### Setup
1. Connect sensors to the appropriate analog pins (`A0`, `A1`, etc.).
2. Upload the code to your Arduino board.
3. Open the Serial Monitor in the Arduino IDE at a baud rate of `9600`.

### Workflow
1. Use the `cal` commands to add calibration points for your sensors.
2. View raw and calibrated values in the Serial Monitor.
3. Reset calibration data if needed using the `reset` commands.
