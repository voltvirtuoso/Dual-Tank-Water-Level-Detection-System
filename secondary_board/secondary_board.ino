#include <WiFi.h>
#include <esp_now.h>

// Pin definitions
const int trigPin = 23;
const int echoPin = 22;
const int buzzerPin = 27;
const int redPin = 18;
const int greenPin = 19;
const int bluePin = 21;

// Water level sensor pins (ADC pins)
const int levelPins[] = {36, 39, 34, 35};
const int numLevelSensors = 4;
const int threshold = 300; // ADJUST this if the wires falsly detect water or dont detect water at all. (lower threshold means false positive and higher means false negative)

// Water level parameters
const float fullTankDistance = 3.0;
const float emptyTankDistance = 9.0;

float waterLevelPercentage = 0.0;
int activeSensors = 0;

// Structure for data
typedef struct struct_message {
  float waterLevel;
  int activeSensors;
  bool lowLevelAlert;
} struct_message;

struct_message myData;
uint8_t broadcastAddress[] = {0xfc, 0xb4, 0x67, 0xf1, 0xf3, 0x40};	// add main board's mac address here
esp_now_peer_info_t peerInfo;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: Print status (but initialize it in setup if you want to use that in future)
}

void setup() {
  Serial.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  for (int i = 0; i < numLevelSensors; i++) {
    pinMode(levelPins[i], INPUT); 
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_send_cb(esp_now_send_cb_t(onDataSent));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

float getUltrasonicDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // timeout of 30000 microseconds (30ms)
  long duration = pulseIn(echoPin, HIGH, 30000); 

  if (duration == 0) {
    return 0.0; 
  }

  float distanceCm = duration * 0.034 / 2;
  float distanceFeet = distanceCm / 30.48;

  return distanceFeet;
}

float calculateWaterLevel(float distance) {
  if (distance == 0.0) return 0.0; // Handle error case
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

// Global blink state variables to persist across loops
bool blinkState = false;
unsigned long lastBlinkTime = 0;

void updateLEDsAndBuzzer() {
  bool lowLevelAlert = false;
  unsigned long currentMillis = millis();

  // Toggle blink state every 500ms
  if (currentMillis - lastBlinkTime > 500) {
    blinkState = !blinkState;
    lastBlinkTime = currentMillis;
  }

  if (activeSensors == 0) {
    // 0%: Blink RED
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    
    digitalWrite(redPin, blinkState ? HIGH : LOW);
    digitalWrite(buzzerPin, blinkState ? HIGH : LOW);
    lowLevelAlert = true;
  } 
  else if (activeSensors == 1) {
    // 25%: Stable RED
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    digitalWrite(buzzerPin, LOW);
  } 
  else if (activeSensors == 2) {
    // 50%: Stable BLUE
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    digitalWrite(buzzerPin, LOW);
  } 
  else if (activeSensors == 3) {
    // 75%: Blink GREEN
    digitalWrite(redPin, LOW);
    digitalWrite(bluePin, LOW);
    
    digitalWrite(greenPin, blinkState ? HIGH : LOW);
    digitalWrite(buzzerPin, LOW);
  } 
  else if (activeSensors >= 4) {
    // 100%: Stable GREEN
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
    digitalWrite(buzzerPin, LOW);
  }

  myData.waterLevel = waterLevelPercentage;
  myData.activeSensors = activeSensors;
  myData.lowLevelAlert = lowLevelAlert;
}

void loop() {
  float distance = getUltrasonicDistance();
  
  // Debug print
  Serial.print("Dist (ft): ");
  Serial.print(distance);
  Serial.print(" | Sensors Active: ");
  Serial.println(activeSensors);

  waterLevelPercentage = calculateWaterLevel(distance);
  activeSensors = countActiveSensors();
  
  updateLEDsAndBuzzer();

  static unsigned long lastSend = 0;
  if (millis() - lastSend > 2000) {
    esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
    lastSend = millis();
  }
  
  delay(100); 
}