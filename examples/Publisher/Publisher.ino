/************************************************************************
 * Publisher example
 */
#include <CommunicationManager.h>

#define CAN_ID 200

#define SENSOR_PIN 23

uint16_t  sensorValue;
uint8_t   txFlagSensorValue;

void setup() {
  // Configure Pin
  pinMode (SENSOR_PIN, INPUT);

  // Initialize CommunicationManager with 500kBit/s
  CommunicationManager::GetInstance()->Initialize(500000);

  // Publish Sensor value every 40ms
  CommunicationManager::GetInstance()->Publish(&sensorValue, sizeof(sensorValue), CAN_ID, &txFlagSensorValue, CYCLE_40);

  // Initialize Sensor Value
  readSensor();
}

void readSensor() {
  sensorValue = analogRead(SENSOR_PIN);
}

void loop() {
  // Update CommunicationManager
  CommunicationManager::GetInstance()->Update();

  // Check if value was sent
  if(1 == txFlagSensorValue) {
    // Read new sensor value
    readSensor();

    // Reset flag
    txFlagSensorValue = 0;
  }
}
