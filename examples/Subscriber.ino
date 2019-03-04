/************************************************************************
 * Subscriber example
 */
#include <CommunicationManager.h>

#define CAN_ID 200

#define LED_PIN 13

uint16_t  sensorValue;
uint8_t   rxFlagSensorValue;

void setup() {
  // Configure Pin
  pinMode (LED_PIN, OUTPUT);

  // Turn LED off
  digitalWrite(LED_PIN, LOW);

  // Initialize CommunicationManager with 500kBit/s
  CommunicationManager::GetInstance()->Initialize(500000);

  // Subscribe to CAN Messages with ID 200
  CommunicationManager::GetInstance()->Subscribe(&sensorValue, sizeof(sensorValue), CAN_ID, &rxFlagSensorValue);
}

void loop() {
  // Update CommunicationManager
  CommunicationManager::GetInstance()->Update();

  // Check if value was received
  if(1 == rxFlagSensorValue) {

    // Switch LED
    if(sensorValue > 100) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }

    // Reset flag
    rxFlagSensorValue = 0;
  }
}
