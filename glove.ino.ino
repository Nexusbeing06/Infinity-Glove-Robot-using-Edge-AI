/*
   THE INFINITY GLOVE - FINAL MASTER CODE (TRANSMITTER)
   Features:
   1. Normal Mode: Tilt to Drive
   2. Fan Mode: Tilt to Spin Fan
   3. Magic Mode: Draw Circle -> 360 Spin (Edge AI)
*/

// --- 1. YOUR AI LIBRARY ---
#include <allrounder165-project-1_inferencing.h>

#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// --- 2. ROBOT ADDRESS (REPLACE WITH YOUR CAR'S MAC) ---
uint8_t broadcastAddress[] = {0x20, 0xE7, 0xC8, 0x67, 0x63, 0x2C}; 

Adafruit_MPU6050 mpu;
const int BTN_PIN = 4;      // Button Pin
const int MODE_LED = 2;     // Blue LED Pin

// Variables
bool carMode = false;       // False = Fan Mode, True = Car Mode
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE]; // Buffer for AI data
size_t feature_ix = 0;

// ESP-NOW Structure
typedef struct struct_message { char command; } struct_message;
struct_message myData;
esp_now_peer_info_t peerInfo;

// Helper to send data
void sendCommand(char cmd) {
  myData.command = cmd;
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(MODE_LED, OUTPUT);
  
  if (!mpu.begin()) {
    Serial.println("MPU6050 Not Found!");
    while (1);
  }
  
  // MPU Settings (Must match what you used for training)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Setup ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("GLOVE READY. Hold Button to switch modes.");
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // --- BUTTON LOGIC ---
  if (digitalRead(BTN_PIN) == LOW) {
    unsigned long startPress = millis();
    while(digitalRead(BTN_PIN) == LOW); // Wait for release
    
    // Long Press (> 1s) = Switch Mode
    if (millis() - startPress > 1000) {
      carMode = !carMode;
      digitalWrite(MODE_LED, carMode ? HIGH : LOW); // LED ON = Car Mode
      sendCommand('S'); // Stop everything
      Serial.println(carMode ? "MODE: CAR (AI Ready)" : "MODE: FAN");
      feature_ix = 0; // Reset AI buffer
    } 
    // Short Press = Stop
    else {
      sendCommand('0'); // '0' is usually mapped to stop/reset
      Serial.println("SNAP! (Stop)");
    }
  }

  // --- AI MAGIC DETECTION (Only in Car Mode) ---
  if (carMode) {
    // 1. Fill the buffer with sensor data
    features[feature_ix++] = a.acceleration.x;
    features[feature_ix++] = a.acceleration.y;
    features[feature_ix++] = a.acceleration.z;
    features[feature_ix++] = g.gyro.x;
    features[feature_ix++] = g.gyro.y;
    features[feature_ix++] = g.gyro.z;

    // 2. If buffer is full, run the AI
    if (feature_ix >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        
        signal_t signal;
        int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
        ei_impulse_result_t result = { 0 };
        err = run_classifier(&signal, &result, false);

        if (err == 0) {
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                String label = String(result.classification[ix].label);
                
                // CHECK FOR MAGIC SPELL (Confidence > 80%)
                if ((label == "circle_data" || label == "CIRCLE_DATA") && result.classification[ix].value > 0.8) {
                    Serial.println("!!! MAGIC DETECTED: CIRCLE !!!");
                    
                    // --- THE PULSE SPIN (Safe for Battery) ---
                    for(int i=0; i<3; i++) {
                      sendCommand('R'); 
                      delay(800); 
                      sendCommand('S'); 
                      delay(200);
                    }
                    sendCommand('S'); // Final Stop
                }
            }
        }
        // Reset buffer to start listening again
        feature_ix = 0; 
    }
  }

  // --- MANUAL DRIVING ---
  if (carMode) {
     // Car Mode Driving
     if (a.acceleration.y < -5.0) sendCommand('F');      // Forward
     else if (a.acceleration.y > 5.0) sendCommand('B');  // Backward
     else if (a.acceleration.x > 6.0) sendCommand('R');  // Right
     else if (a.acceleration.x < -6.0) sendCommand('L'); // Left
     else { 
       sendCommand('S'); // Stop if hand is flat
     }
  } 
  else {
    // Fan Mode Controls
    if (a.acceleration.x > 7.0) { sendCommand('1'); delay(100); } // Fan Speed 1
    else if (a.acceleration.x < -7.0) { sendCommand('2'); delay(100); } // Fan Speed 2
    else if (a.acceleration.y > 6.0) { sendCommand('3'); delay(500); } // Toggle Fan
  }

  delay(10); // Run loop at ~100Hz
}