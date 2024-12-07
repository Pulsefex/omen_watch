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

### **Component & Driver Breakdown**

This section provides a detailed explanation of the implementation, focusing on the **MAX30102**, **TMP102**, and **SSD1306 OLED** peripherals. It emphasizes the design principles, system architecture, and direct code references to explain the underlying functionality. The architecture is centered around a **bare-metal scheduler**, interrupt-driven design, and precise timer configurations.

---

### **1. Bare-Metal Scheduler**

#### **What is a Bare-Metal Scheduler?**
A lightweight, deterministic task manager without an RTOS. It operates by cycling through tasks based on flags set by ISRs or periodic timer events.

#### **Advantages Over FreeRTOS:**
- **Low Overhead:** Avoids context switching and reduces complexity.
- **Deterministic Timing:** Ensures predictable task execution, ideal for real-time requirements.
- **Direct Hardware Control:** Allows tight integration with peripherals.

#### **Code Snippet: Scheduler in Action**
```c
while (1) {
    SCH_Run(~0);         // Runs all scheduled tasks
    vReadSensorData();   // Processes sensor data
}
```

Here, `SCH_Run(~0)` manages scheduled tasks, and `vReadSensorData()` ensures periodic sensor data processing.

---

### **2. Interrupt-Driven Architecture**

#### **What are ISRs?**
Interrupt Service Routines (ISRs) are triggered by hardware events to handle critical tasks immediately. They often set flags for the main loop to process.

#### **Key ISRs and Code References:**

1. **Timer ISR (`HAL_TIM_PeriodElapsedCallback`):**
   - **Trigger:** Timer 16 periodic interrupt.
   - **Action:** Sets `ucSensorReadFlag` to signal sensor reading.
   - **Code:**
     ```c
     void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
         if (htim->Instance == TIM16) {
             ucSensorReadFlag = 1; // Signals the main loop to read sensor data
         }
     }
     ```

2. **GPIO ISR (`HAL_GPIO_EXTI_Callback`):**
   - **Trigger:** MAX30102’s interrupt pin signals new data.
   - **Action:** Sets a flag to process the data.
   - **Code:**
     ```c
     void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
         if (GPIO_Pin == MAX30102_INT_Pin) {
             ecgFIFOIntFlag = 1; // Signals data readiness
         }
     }
     ```

3. **ADC ISR (`HAL_ADC_ConvCpltCallback`):**
   - **Trigger:** Completion of ADC conversion for TMP102.
   - **Action:** Converts raw ADC data to temperature values.
   - **Code:**
     ```c
     void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
         uhADCxConvertedData = HAL_ADC_GetValue(hadc);
         usADCxConvertedData_Voltage_mVolt = __ADC_CALC_DATA_VOLTAGE(VDDA_APPLI, uhADCxConvertedData);
         vSetGSRAnalogValue(uhADCxConvertedData);
     }
     ```

---

### **3. Timer Configuration**

Timers are the backbone of scheduling, ensuring precise timing for sensor polling and OLED updates.

#### **Timer Parameters and Calculations:**

1. **TIM16: Periodic Sensor Polling**
   - **Prescaler:** `20` → \( 32 $\text{MHz}$ $\div$ (20+1) $\approx$ 1.52 $\text{MHz}$ \)
   - **Period:** `63999` → \( 1.52 $\text{MHz}$ $\div$ 64000 $\approx$ 42 $\text{ms}$ \)
   - **Code:**
     ```c
     htim16.Init.Prescaler = 20;
     htim16.Init.Period = 63999;
     HAL_TIM_Base_Init(&htim16);
     ```

2. **TIM1: OLED Brightness Control**
   - **Prescaler:** `9` → \( 32 $\text{MHz}$ $\div$ (9+1) = 3.2 $\text{MHz}$\)
   - **Period:** `97` → \( 3.2 $\text{MHz}$ $\div$ 97 $\approx$ 33 $\text{MHz}$ )
   - **Code:**
     ```c
     htim1.Init.Prescaler = 9;
     htim1.Init.Period = 97;
     HAL_TIM_Base_Init(&htim1);
     ```

---

### **4. I2C Communication**

#### **Shared I2C3 Bus for Peripherals:**
- **SCL:** `PC0`
- **SDA:** `PC1`
- **Timing:** `0x0040040C` for 100 kHz standard mode.

#### **Peripheral Breakdown:**
1. **MAX30102:**
   - Uses I2C interrupts for asynchronous data transfer.
   - **Code for Initialization:**
     ```c
     void vMax30102Init(void) {
         // Configure I2C for MAX30102
         HAL_I2C_Init(&hi2c3);
     }
     ```

2. **TMP102:**
   - Polled periodically using I2C.
   - **Code for Reading Data:**
     ```c
     void vReadTmp102Data(void) {
         uint8_t data[2];
         HAL_I2C_Master_Receive(&hi2c3, TMP102_ADDR, data, 2, HAL_MAX_DELAY);
     }
     ```

3. **SSD1306 OLED:**
   - I2C commands update the display.
   - **Code for Display Update:**
     ```c
     void vOledBlePrintData(void) {
         SSD1306_UpdateScreen();
     }
     ```

---

### **5. GPIO and External Interrupts**

#### **GPIO Pin Roles:**
1. **MAX30102 Data Ready:**
   - GPIO pin detects rising edge for new data availability.
   - **Code:**
     ```c
     GPIO_InitStruct.Pin = MAX30102_INT_Pin;
     GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
     HAL_GPIO_Init(MAX30102_INT_GPIO_Port, &GPIO_InitStruct);
     ```

2. **OLED Power Control:**
   - GPIO pin toggles OLED power.
   - **Code:**
     ```c
     HAL_GPIO_WritePin(DISP_VDD_GPIO_Port, DISP_VDD_Pin, GPIO_PIN_SET);
     ```

---

### **6. ADC Configuration: TMP102**

#### **ADC Parameters:**
- **Prescaler:** `ADC_CLOCK_ASYNC_DIV1` → Maximum speed.
- **Resolution:** `8-bit` for fast sampling.
- **Sampling Time:** `640 Cycles` for stable readings.

#### **Code for TMP102 Data Processing:**
```c
sConfig.Channel = ADC_CHANNEL_3;
sConfig.Rank = ADC_REGULAR_RANK_1;
sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
HAL_ADC_ConfigChannel(&hadc1, &sConfig);
```

---

### **7. UART Communication for OLED**

#### **Parameters:**
- **Baud Rate:** `115200`.
- **Mode:** Full-duplex TX/RX.

#### **OLED Data Transmission:**
- Uses UART interrupts to ensure non-blocking communication.
- **Code:**
  ```c
  HAL_UART_Transmit(&huart1, (uint8_t*)data, strlen(data), HAL_MAX_DELAY);
  ```

---

### **Why This Architecture Works**

- **Deterministic Scheduling:** Timers and ISRs ensure predictable, real-time performance.
- **Efficient Resource Usage:** Minimal overhead with bare-metal implementation.
- **Scalability:** Allows integration of additional peripherals with minimal changes.

This architecture balances simplicity, efficiency, and reliability, delivering a robust system tailored for embedded applications.


## **BLE Communication**
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
