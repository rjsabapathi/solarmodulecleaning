
// Function to process JSON input from Serial Monitor
void processJSON(String jsonString) {
  StaticJsonDocument<200> doc;

  // Deserialize the JSON string
  DeserializationError error = deserializeJson(doc, jsonString);

  if (error) {
    Serial.print(F("Failed to parse JSON: "));
    Serial.println(error.c_str());
    return;
  }

  String command = doc["command"];

  if (command == "clock") {
    String timeString = doc["time"];
    changeClockTime(timeString);
  } else if (command == "alarm") {
    String timeString = doc["time"];
    setAlarmTime(timeString);
  } else if (command == "setSpeeds") {
    int motor1Speed = doc["motor1MaxSpeed"];
    int motor2Speed = doc["motor2MaxSpeed"];
    setMaxMotorSpeeds(motor1Speed, motor2Speed);
  } else {
    Serial.println("Invalid command: " + command);
  }
}


