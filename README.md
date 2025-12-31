# Dual-Tank-Water-Level-Detection-System

A comprehensive water level monitoring and control system that manages two water tanks with automatic pump control, real-time monitoring, and visual/audio alerts.

## Table of Contents
- [Features](#features)
- [System Overview](#system-overview)
- [Hardware Requirements](#hardware-requirements)
- [Circuit Diagrams](#circuit-diagrams)
- [Software Dependencies](#software-dependencies)
- [Installation](#installation)
- [Configuration](#configuration)
- [Operation Logic](#operation-logic)
- [Customization](#customization)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Dual Tank Monitoring**: Real-time water level tracking for both main and secondary tanks
- **Multiple Sensing Methods**: 
  - Ultrasonic sensors for precise distance measurement
  - Contact-based water level sensors for threshold detection
- **Wireless Communication**: ESP-NOW protocol for low-latency data transfer between boards
- **Automatic Pump Control**: Smart logic to maintain optimal water levels
- **Visual & Audio Alerts**: RGB LED indicators and buzzer for status notification
- **Threshold-Based Alerts**: Configurable warning levels for critical situations
- **Low Power Design**: Optimized for continuous operation with minimal power consumption
- **Watchdog Protection**: Built-in safeguards against system lockups

## System Overview

This system consists of two ESP32-based boards working together:

### Main Board
- Monitors the primary water tank
- Controls the water pump via relay
- Receives data from secondary board via ESP-NOW
- Makes pump control decisions based on both tank levels
- Provides visual/audio feedback through RGB LED and buzzer

### Secondary Board
- Monitors the secondary water tank
- Sends water level data to main board via ESP-NOW
- Provides local visual/audio feedback through RGB LED and buzzer
- Operates independently but coordinates with main board

### System Workflow
1. Both boards continuously monitor their respective tank water levels
2. Secondary board sends its water level data to main board every 2 seconds
3. Main board uses data from both tanks to make pump control decisions:
   - **Pump ON** when secondary tank is low (<25%) AND main tank has sufficient water (>25%)
   - **Pump OFF** when secondary tank is full (≥100%) OR main tank is low (≤25%)
4. Visual and audio alerts provide immediate status feedback:
   - **Red LED + Buzzer**: Critical low level (<25%)
   - **Blue LED**: Medium level (50-75%)
   - **Blinking Green LED**: High level (75-100%)
   - **Stable Green LED**: Full tank (100%)

## Hardware Requirements

### Main Board Components
- ESP32 Development Board (NodeMCU-32S recommended)
- HC-SR04 Ultrasonic Sensor
- 5V Relay Module
- Water Pump (12V DC recommended)
- RGB LED (Common Cathode)
- Active Buzzer (5V)
- 4 Water Level Sensor Wires (stainless steel recommended)
- 220Ω Resistor (for RGB LED)
- 10KΩ Resistor (for buzzer)
- Breadboard and jumper wires
- Power supply (12V 2A for pump + ESP32)

### Secondary Board Components
- ESP32 Development Board (NodeMCU-32S recommended)
- HC-SR04 Ultrasonic Sensor
- RGB LED (Common Cathode)
- Active Buzzer (5V)
- 4 Water Level Sensor Wires (stainless steel recommended)
- 220Ω Resistor (for RGB LED)
- 10KΩ Resistor (for buzzer)
- Breadboard and jumper wires
- Power supply (5V 1A)

### Recommended Tools
- Soldering iron and solder
- Wire strippers
- Multimeter
- Water-resistant enclosure boxes
- Heat shrink tubing

## Circuit Diagrams

### Main Board Connections
```
ESP32 (NodeMCU-32S)        Components
GPIO 22 (D22)   ----------> HC-SR04 TRIG
GPIO 23 (D23)   <---------- HC-SR04 ECHO
GPIO 26 (D26)   ----------> Buzzer (+)
GPIO 27 (D27)   ----------> Relay IN
GPIO 16 (D16)   ----------> RGB LED Red (via 220Ω)
GPIO 17 (D17)   ----------> RGB LED Green (via 220Ω)
GPIO 18 (D18)   ----------> RGB LED Blue (via 220Ω)
GPIO 36 (VP)    <---------- Water Sensor 0% (level 1)
GPIO 39 (VN)    <---------- Water Sensor 25% (level 2)
GPIO 34 (S34)   <---------- Water Sensor 50% (level 3)
GPIO 35 (S35)   <---------- Water Sensor 75% (level 4)
GND             ----------> Common Ground
3.3V            ----------> HC-SR04 VCC
5V              ----------> Relay VCC, Buzzer VCC, RGB LED Common Cathode
```

### Secondary Board Connections
```
ESP32 (NodeMCU-32S)        Components
GPIO 23 (D23)   ----------> HC-SR04 TRIG
GPIO 22 (D22)   <---------- HC-SR04 ECHO
GPIO 27 (D27)   ----------> Buzzer (+)
GPIO 18 (D18)   ----------> RGB LED Red (via 220Ω)
GPIO 19 (D19)   ----------> RGB LED Green (via 220Ω)
GPIO 21 (D21)   ----------> RGB LED Blue (via 220Ω)
GPIO 36 (VP)    <---------- Water Sensor 0% (level 1)
GPIO 39 (VN)    <---------- Water Sensor 25% (level 2)
GPIO 34 (S34)   <---------- Water Sensor 50% (level 3)
GPIO 35 (S35)   <---------- Water Sensor 75% (level 4)
GND             ----------> Common Ground
3.3V            ----------> HC-SR04 VCC
5V              ----------> Buzzer VCC, RGB LED Common Cathode
```

### Water Level Sensor Wiring
For both boards, connect stainless steel wires at different heights in the tank:
- **Level 1 (0%)**: Bottom of tank
- **Level 2 (25%)**: 25% height from bottom
- **Level 3 (50%)**: Middle of tank
- **Level 4 (75%)**: 75% height from bottom

> **WARNING**: Use only stainless steel or corrosion-resistant wires. Never use copper wires as they will corrode quickly in water.

## Software Dependencies

- Arduino IDE (v1.8.19 or later)
- ESP32 Board Support (v2.0.0 or later)
- ESP-NOW Library (built-in)

### Library Installation:
1. Open Arduino IDE
2. Go to **File** > **Preferences**
3. Add ESP32 board manager URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Go to **Tools** > **Board** > **Boards Manager**
5. Search for "esp32" and install the latest version

## Installation

### Step 1: Clone Repository
```bash
git clone https://github.com/yourusername/Dual-Tank-Water-Level-Detection-System.git
cd Dual-Tank-Water-Level-Detection-System
```

### Step 2: Configure MAC Addresses
1. Upload the `main_board.ino` sketch to your main ESP32 board
2. Open Serial Monitor (115200 baud) and note the MAC address printed at startup
3. Replace the `broadcastAddress` in `secondary_board.ino` with the main board's MAC address:
   ```cpp
   uint8_t broadcastAddress[] = {0xfc, 0xb4, 0x67, 0xf1, 0xf3, 0x40}; // Replace with your main board's MAC
   ```

### Step 3: Upload Sketches
1. **Main Board**:
   - Connect main ESP32 board
   - Open `main_board/main_board.ino`
   - Select board: **Tools** > **Board** > **ESP32 Arduino** > **NodeMCU-32S**
   - Select port and upload

2. **Secondary Board**:
   - Connect secondary ESP32 board
   - Open `secondary_board/secondary_board.ino`
   - Select board: **Tools** > **Board** > **ESP32 Arduino** > **NodeMCU-32S**
   - Select port and upload

## Configuration

### Key Configuration Parameters

#### In `main_board.ino`:
```cpp
// Tank dimensions (in feet)
const float fullTankDistance = 3.0;   // Distance when tank is full
const float emptyTankDistance = 9.0;  // Distance when tank is empty

// Water level thresholds
const float MIN_LEVEL_THRESHOLD = 25.0;  // Minimum level to keep pump running
const float MAX_LEVEL_THRESHOLD = 100.0; // Maximum level before stopping pump

// Water sensor sensitivity
const int threshold = 300; // Adjust based on your water conductivity
```

#### In `secondary_board.ino`:
```cpp
// Tank dimensions (in feet)
const float fullTankDistance = 3.0;
const float emptyTankDistance = 9.0;

// Water sensor sensitivity
const int threshold = 300; // Same as main board for consistency
```

### Calibration Guide
1. **Ultrasonic Sensor Calibration**:
   - Measure actual distance from sensor to water surface when tank is empty
   - Update `emptyTankDistance` with this value
   - Measure distance when tank is full and update `fullTankDistance`

2. **Water Sensor Calibration**:
   - If sensors trigger falsely: Increase `threshold` value
   - If sensors don't trigger when submerged: Decrease `threshold` value
   - Test with actual water conditions (mineral content affects conductivity)

3. **Pump Control Thresholds**:
   - Adjust `MIN_LEVEL_THRESHOLD` and `MAX_LEVEL_THRESHOLD` based on your tank capacity and usage patterns
   - Lower thresholds for more aggressive pumping
   - Higher thresholds for conservative water usage

## Operation Logic

### Pump Control Algorithm
The main board uses this decision logic:

```cpp
bool secondaryTankLow = (secondaryActiveSensors == 0); // Less than 25%
bool secondaryTankFull = (secondaryActiveSensors >= 4); // 100%
bool mainTankLow = (mainActiveSensors == 0); // Less than 25%

if (secondaryTankLow && !mainTankLow && !pumpRunning) {
  // Start pump: Secondary low but main has water
  digitalWrite(relayPin, HIGH);
  pumpRunning = true;
} 
else if ((secondaryTankFull || mainTankLow) && pumpRunning) {
  // Stop pump: Secondary full OR main tank low
  digitalWrite(relayPin, LOW);
  pumpRunning = false;
}
```

### Alert Levels and Responses

| Water Level | LED Status | Buzzer Status | System Action |
|------------|------------|---------------|---------------|
| 0% (Critical Low) | Blinking Red | Alternating On/Off | Main: Flash red + buzzer, Secondary: Same |
| 25% (Low) | Stable Red | Off | No pump action (main tank only) |
| 50% (Medium) | Stable Blue | Off | Normal operation |
| 75% (High) | Blinking Green | Off | Prepare to stop pump (secondary tank) |
| 100% (Full) | Stable Green | Off | Stop pump immediately |

### Communication Protocol
- **Data Structure**:
  ```cpp
  typedef struct struct_message {
    float waterLevel;        // Percentage (0-100)
    int activeSensors;       // Number of active sensors (0-4)
    bool lowLevelAlert;      // True if level < 25%
  } struct_message;
  ```
- **Update Frequency**: Secondary board sends data every 2 seconds
- **Retry Logic**: ESP-NOW handles packet loss automatically

## Customization

### Adding Additional Sensors
To add more water level sensors:

1. **Hardware**:
   - Use additional ADC pins (GPIO 32, 33 on ESP32)
   - Add more stainless steel sensing wires at desired heights

2. **Software**:
   ```cpp
   // In both boards
   const int levelPins[] = {36, 39, 34, 35, 32, 33}; // Add more pins
   const int numLevelSensors = 6; // Update count

   // Update threshold logic in controlPump() and updateLEDsAndBuzzer()
   ```

### Modifying Alert Behavior
To change LED/buzzer behavior:

```cpp
// In secondary_board.ino - updateLEDsAndBuzzer()
void updateLEDsAndBuzzer() {
  if (activeSensors == 0) {
    // Custom alert pattern
    tone(buzzerPin, 1000, 200); // Short beep
    digitalWrite(redPin, HIGH);
    delay(100);
    digitalWrite(redPin, LOW);
  }
  // ... other conditions
}
```

### Adding WiFi Connectivity
To add WiFi for remote monitoring:

```cpp
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "your_wifi";
const char* password = "your_password";
WebServer server(80);

void setupWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.on("/", handleRoot);
  server.begin();
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Water Level Monitoring</h1>";
  html += "<p>Main Tank: " + String(mainTankLevel) + "%</p>";
  html += "<p>Secondary Tank: " + String(secondaryTankLevel) + "%</p>";
  html += "<p>Pump Status: " + String(pumpRunning ? "ON" : "OFF") + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}
```

## Troubleshooting

### Common Issues and Solutions

| Issue | Symptoms | Solution |
|-------|----------|----------|
| **No Communication** | Secondary data not showing on main board | 1. Check MAC addresses<br>2. Verify ESP-NOW initialization<br>3. Ensure both boards on same WiFi channel |
| **False Water Detection** | Sensors trigger when dry | 1. Increase `threshold` value<br>2. Clean sensor wires<br>3. Check for electrical interference |
| **Pump Not Starting** | Secondary tank low but pump off | 1. Check relay wiring<br>2. Verify pump power supply<br>3. Test relay manually<br>4. Check pump control logic thresholds |
| **Inaccurate Levels** | Ultrasonic readings incorrect | 1. Calibrate tank distances<br>2. Ensure sensor is level<br>3. Check for obstructions in tank<br>4. Add averaging to readings |
| **System Lockups** | Boards stop responding | 1. Add watchdog timer<br>2. Reduce sensor polling frequency<br>3. Check power supply stability<br>4. Add memory management |

### Debugging Tips

1. **Enable verbose logging**:
   ```cpp
   #define DEBUG_LEVEL 2
   
   if (DEBUG_LEVEL >= 1) {
     Serial.print("Distance: ");
     Serial.println(distance);
   }
   ```

2. **Test sensors individually**:
   ```cpp
   void testSensors() {
     for (int i = 0; i < numLevelSensors; i++) {
       Serial.print("Sensor ");
       Serial.print(i);
       Serial.print(": ");
       Serial.println(analogRead(levelPins[i]));
       delay(500);
     }
   }
   ```

3. **Manual pump control**:
   Add this to `main_board.ino` for testing:
   ```cpp
   void setup() {
     // ... existing code
     pinMode(0, INPUT_PULLUP); // Boot button
   }
   
   void loop() {
     if (digitalRead(0) == LOW) { // Button pressed
       digitalWrite(relayPin, !digitalRead(relayPin)); // Toggle pump
       delay(500); // Debounce
     }
     // ... existing code
   }
   ```

## Contributing

Contributions are welcome! Please follow these guidelines:

1. **Fork** the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. Open a **Pull Request**

### Contribution Areas
- **Hardware Improvements**: Better sensor designs, enclosure designs
- **Software Features**: WiFi connectivity, mobile app integration, data logging
- **Documentation**: Circuit diagrams, calibration guides, user manuals
- **Optimizations**: Power management, code efficiency improvements

### Code Style Guidelines
- Follow Arduino coding style
- Use descriptive variable names
- Comment complex logic
- Keep functions focused on single responsibilities
- Test on actual hardware before submitting

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

