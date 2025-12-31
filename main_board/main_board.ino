#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Pin definitions for main board
const int trigPin = 22;    // TRIG pin for ultrasonic
const int echoPin = 23;    // ECHO pin for ultrasonic
const int buzzerPin = 26;  // Buzzer pin
const int relayPin = 27;   // Relay pin for pump
const int redPin = 16;     // RGB LED - Red
const int greenPin = 17;   // RGB LED - Green  
const int bluePin = 18;    // RGB LED - Blue

// Water level sensor pins (ADC pins)
const int levelPins[] = {36, 39, 34, 35}; // 0%, 25%, 50%, 75% levels
const int numLevelSensors = 4;

const int threshold = 300; // ADJUST this if the wires falsly detect water or dont detect water at all. (lower threshold means false positive and higher means false negative)

// Water level parameters
const float fullTankDistance = 3.0;   // 3 feet = full tank (100%)
const float emptyTankDistance = 9.0;  // 9 feet = empty tank (0%)

// Threshold levels
const float MIN_LEVEL_THRESHOLD = 25.0;  // 25% minimum level
const float MAX_LEVEL_THRESHOLD = 100.0; // 100% maximum level

// Global variables for water levels
float mainTankLevel = 0.0;
float secondaryTankLevel = 0.0;
int mainActiveSensors = 0;
int secondaryActiveSensors = 0;
bool secondaryLowAlert = false;
bool pumpRunning = false;

// Structure for receiving data via ESP-NOW
typedef struct struct_message {
  float waterLevel;
  int activeSensors;
  bool lowLevelAlert;
} struct_message;

struct_message receivedData;

// Callback when data is received
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  secondaryTankLevel = receivedData.waterLevel;
  secondaryActiveSensors = receivedData.activeSensors;
  secondaryLowAlert = receivedData.lowLevelAlert;
  
  Serial.print("Secondary Tank Level: ");
  Serial.print(secondaryTankLevel);
  Serial.print("%, Active Sensors: ");
  Serial.println(secondaryActiveSensors);
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // Initialize water level sensor pins (Pulled UP)
  for (int i = 0; i < numLevelSensors; i++) {
    pinMode(levelPins[i], INPUT_PULLUP);
  }
  
  // Turn off relay initially (pump off)
  digitalWrite(relayPin, LOW);
  
  // Set up WiFi in station mode
  WiFi.mode(WIFI_STA);

  Serial.print("[DEFAULT] ESP32 Board MAC Address: ");	// use this mac address in the secondary baord code for esp now communication 
  readMacAddress();
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // receive callback
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
  
  Serial.println("Receiver started");
}

float getUltrasonicDistance() {
	
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  float duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  
  // cm to feet conversion
  float distance = duration * 0.034 / 2; 
  distance = distance / 30.48; 
  
  // Constrain distance to valid range
  distance = constrain(distance, fullTankDistance, emptyTankDistance);
  
  return distance;
}

float calculateWaterLevel(float distance) {
  // Calculate percentage: 100% at fullTankDistance, 0% at emptyTankDistance
  float percentage = ((emptyTankDistance - distance) / (emptyTankDistance - fullTankDistance)) * 100.0;
  return constrain(percentage, 0, 100);
}

int countActiveSensors() {
  int count = 0;
  for (int i = 0; i < numLevelSensors; i++) {
    if (analogRead(levelPins[i]) > threshold) {
      count++;
    }
  }
  return count;
}

void updatePheripherals() {
  // Turn off all LEDs first
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);
  
  if (mainActiveSensors == 0) {
    // Less than 25% - flash red and buzzer on
    // digitalWrite(redPin, HIGH);
    // digitalWrite(buzzerPin, HIGH);
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(redPin, !digitalRead(redPin));
      digitalWrite(buzzerPin, !digitalRead(buzzerPin));
      lastBlink = millis();
    }
  } 
  else if (mainActiveSensors == 1) {
    // 25-50% - stable red, buzzer off
    digitalWrite(redPin, HIGH);
    digitalWrite(buzzerPin, LOW);
  } 
  else if (mainActiveSensors == 2) {
    // 50-75% - blue on
    digitalWrite(bluePin, HIGH);
    digitalWrite(buzzerPin, LOW);
  } 
  else if (mainActiveSensors == 3) {
    // 75-100% - blink green
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(greenPin, !digitalRead(greenPin));
      lastBlink = millis();
    }
    digitalWrite(buzzerPin, LOW);
  } 
  else if (mainActiveSensors >= 4) {
    // 100% - stable green
    digitalWrite(greenPin, HIGH);
    digitalWrite(buzzerPin, LOW);
  }
}

void controlPump() {
  // Pump logic:
  // Turn on pump if secondary tank is low (<25%) AND main tank has enough water (>25%)
  // Turn off pump if secondary tank is full (>=100%) OR main tank is low (<=25%)
  
  bool secondaryTankLow = (secondaryActiveSensors == 0); // Less than 25%
  bool secondaryTankFull = (secondaryActiveSensors >= 4); // 100%
  bool mainTankLow = (mainActiveSensors == 0); // Less than 25%
  
  if (secondaryTankLow && !mainTankLow && !pumpRunning) {
    // Start pump
    digitalWrite(relayPin, HIGH);
    pumpRunning = true;
    Serial.println("Pump started - secondary tank low");
  } 
  else if ((secondaryTankFull || mainTankLow) && pumpRunning) {
    // Stop pump
    digitalWrite(relayPin, LOW);
    pumpRunning = false;
    Serial.println("Pump stopped - secondary tank full or main tank low");
  }
}

void loop() {
  
  float distance = getUltrasonicDistance();
  mainTankLevel = calculateWaterLevel(distance);
  mainActiveSensors = countActiveSensors();
  updatePheripherals();
  
  // Control pump based on both tank levels
  controlPump();
  
  // Print status for debugging
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    Serial.print("Main Tank: ");
    Serial.print(mainTankLevel);
    Serial.print("% (");
    Serial.print(mainActiveSensors);
    Serial.print(" sensors), Secondary Tank: ");
    Serial.print(secondaryTankLevel);
    Serial.print("% (");
    Serial.print(secondaryActiveSensors);
    Serial.print(" sensors), Pump: ");
    Serial.println(pumpRunning ? "ON" : "OFF");
    lastPrint = millis();
  }
  
  delay(100); // Small delay to prevent watchdog reset
}