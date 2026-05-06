/*
   THE DR. STRANGE TANK - RECEIVER CODE (CAR)
   UPDATED FOR ESP32 BOARD VERSION 3.0+
*/

#include <esp_now.h>
#include <WiFi.h>

// --- PIN DEFINITIONS ---

// MOTOR A (Right Wheels)
const int ENA = 13;  
const int IN1 = 12;
const int IN2 = 14;

// MOTOR B (Left Wheels)
const int ENB = 25;  
const int IN3 = 27;
const int IN4 = 26;

// FAN MOTOR (Driver 2)
const int FAN_A = 33;
const int FAN_B = 32;

// --- ESP-NOW STRUCTURE ---
typedef struct struct_message {
  char command;
} struct_message;

struct_message myData;

// --- HELPER FUNCTIONS DECLARATION ---
void moveCar(int in1, int in2, int in3, int in4);
void controlFan(int speed);

// --- CALLBACK FUNCTION ---
void OnDataRecv(const esp_now_recv_info_t * info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  char cmd = myData.command;

  // --- COMMAND HANDLING ---
  switch (cmd) {
    case 'F': // Forward
      moveCar(HIGH, LOW, HIGH, LOW);
      break;
      
    case 'B': // Backward
      moveCar(LOW, HIGH, LOW, HIGH);
      break;
      
    case 'L': // Left Turn
      moveCar(LOW, HIGH, HIGH, LOW);
      break;
      
    case 'R': // Right Turn
      moveCar(HIGH, LOW, LOW, HIGH);
      break;
      
    case 'S': // Stop Car
      moveCar(LOW, LOW, LOW, LOW);
      break;

    // --- FAN COMMANDS ---
    case '1': // Fan Speed Low
      controlFan(150); 
      break;
      
    case '2': // Fan Speed High
      controlFan(255);
      break;
      
    case '3': // Fan OFF
      controlFan(0);
      break;
      
    case '0': // EMERGENCY STOP
      moveCar(LOW, LOW, LOW, LOW);
      controlFan(0);
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Set Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(FAN_A, OUTPUT); pinMode(FAN_B, OUTPUT);

  // Init ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register Callback
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("CAR READY. Waiting for Glove...");
}

void loop() {
  delay(100);
}

// --- HELPER FUNCTIONS ---

void moveCar(int in1, int in2, int in3, int in4) {
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
  digitalWrite(IN3, in3);
  digitalWrite(IN4, in4);
}

void controlFan(int speed) {
  if (speed == 0) {
    digitalWrite(FAN_A, LOW);
    digitalWrite(FAN_B, LOW);
  } else {
    digitalWrite(FAN_A, HIGH);
    digitalWrite(FAN_B, LOW);
  }
}
