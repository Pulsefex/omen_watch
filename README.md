# **Pulsefex - Omen Watch**

This project combines embedded systems engineering and BLE communication to create a smart, energy-efficient application for real-time monitoring and communication using the STM32WB microcontroller. The system integrates two key sensors (MAX30102 and TMP102) and employs robust data processing, low-power management, and interactive display features.

---

## **Project Overview**

- **Platform**: STM32WB microcontroller.
- **Sensors**:
  - **MAX30102**: Measures heart rate and SpO₂ levels.
  - **TMP102**: Provides precise temperature readings.
- **Middleware**: STM32_WPAN with BLE 5.4 integration.
- **Display**: SSD1306 OLED for real-time visual feedback.
- **Communication**: BLE using GATT client-server architecture and debugging via UART.

---

## **System Features**

### 1. **Embedded Systems Design**
- **Timers**:
  - TIM16 configured with a **Prescaler** of 20 and **Period** of 63,999 for interrupt-driven data sampling.
  - Precision tuning ensures consistent sensor polling and low-power operation.
- **Interrupts**:
  - Unmasked GPIO interrupts for real-time sensor data acquisition.
  - ISR manages sensor activity flags for efficient task scheduling.
- **Debugging**:
  - **UART**: Displays sensor data and system logs in real-time for debugging and validation.
  - Print statements verify processing flows and enable rapid debugging during development.

### 2. **Sensor Integration**
- **MAX30102**:
  - Configured to deliver SpO₂ and heart rate data via I2C.
  - Data processed through algorithms for accuracy and stability.
- **TMP102**:
  - Temperature data is fetched periodically and displayed.
- **OLED**:
  - Visualizes sensor data dynamically, with clear labels and unit specifications.

### 3. **BLE Communication**
- **Architecture**:
  - **GATT Server**: Exposes sensor data as characteristics.
  - **GATT Client**: Allows remote devices to read or subscribe to updates.
- **Security**:
  - Implements MITM protection and bonding for secure pairing.
- **Dynamic Advertising**:
  - Adjustable advertising intervals for efficient power usage.
- **P2P Protocol**:
  - Ensures seamless bidirectional communication with BLE-enabled clients.

---

## **Development and Debugging**

- **Data Processing**:
  - Sensor data is read using efficient polling mechanisms with flags and interrupts.
  - Data integrity is maintained via masking and unmasking during processing.
- **UART Debugging**:
  - Real-time print statements log sensor values and BLE events.
  - UART interface facilitates seamless testing with the Linux `screen` command.
- **OLED Interface**:
  - Displays debugging states, sensor values, and BLE connection status.
  - Toggle-based screens enable focused visualization.

---

## **BLE Middleware - Theory and Implementation**

- **GATT Roles**:
  - **Server**: Hosts services for MAX30102 and TMP102, allowing characteristic reads and notifications.
  - **Client**: Subscribes to real-time updates.
- **Security**:
  - Leverages fixed and dynamic pins, encryption, and authentication to secure data.
- **L2CAP**:
  - Manages connection parameters to optimize power and performance dynamically.

---

## **Pinout Configuration**
| **Peripheral** | **Pins**       | **Protocol**    | **Details**                  |
|-----------------|---------------|-----------------|------------------------------|
| **MAX30102**    | PC0 (SCL), PC1 (SDA) | I2C (I2C3)     | Shares I2C3 bus with OLED   |
| **TMP102**      | PC0 (SCL), PC1 (SDA) | I2C (I2C3)     | Shares I2C3 bus with OLED   |
| **OLED Display**| PC0 (SCL), PC1 (SDA) | I2C (I2C3)     | Shares I2C3 bus with sensors|
| **UART**        | PB6 (TX), PB7 (RX)   | USART          | UART for debugging/logging  |
| **BLE Antenna** | Internal      | -               | Integrated with STM32WB     |

### Notes:
- All I2C peripherals (MAX30102, TMP102, and OLED) are on the **I2C3 bus** with **PC0 (SCL)** and **PC1 (SDA)**.
- The **USART interface** uses **PB6 (TX)** and **PB7 (RX)** for UART debugging and communication.
- This streamlined pinout ensures efficient use of shared resources while minimizing power consumption.

---

## **Why This Approach?**

1. **Low Power**:
   - Configured timers and dynamic BLE advertising intervals to conserve energy.
2. **Scalability**:
   - Modular design allows easy integration of additional sensors and services.
3. **Reliability**:
   - Real-time debugging ensures quick error resolution during development.
