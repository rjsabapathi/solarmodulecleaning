// Function to set motor speeds and save them to EEPROM
void setMaxMotorSpeeds(int motor1Speed, int motor2Speed) {
  motor1MaxSpeed = motor1Speed;
  motor2MaxSpeed = motor2Speed;

  // Save motor speeds to EEPROM
  EEPROM.write(EEPROM_MOTOR1_MAX_SPEED_ADDR, motor1MaxSpeed);
  EEPROM.write(EEPROM_MOTOR2_MAX_SPEED_ADDR, motor2MaxSpeed);
  EEPROM.commit();  // Commit the changes to EEPROM

  Serial.print("Drive Motor Speed set: ");
  Serial.println(motor1MaxSpeed);
  Serial.print("Brush Motor Speed set: ");
  Serial.println(motor2MaxSpeed);
}



// Function to set alarm time and store it in EEPROM
void setAlarmTime(String timeString) {
  alarmTime = timeString;
  saveAlarmToEEPROM(alarmTime);
  Serial.println("Alarm set to: " + alarmTime);
}




// Save alarm time to EEPROM
void saveAlarmToEEPROM(String timeString) {
  for (int i = 0; i < timeString.length(); i++) {
    EEPROM.write(EEPROM_ALARM_ADDR + i, timeString[i]);
  }
  EEPROM.write(EEPROM_ALARM_ADDR + timeString.length(), '\0');  // Null-terminate the string
  EEPROM.commit();  // Commit changes to EEPROM
  Serial.println("Alarm time saved to EEPROM");
}





// Read alarm time from EEPROM
void readAlarmFromEEPROM() {
  char storedAlarm[10];  // Buffer to hold the stored alarm time
  for (int i = 0; i < 9; i++) {  // Limit to 8 characters (HH:MM:SS) + 1 for null terminator
    storedAlarm[i] = EEPROM.read(EEPROM_ALARM_ADDR + i);
  }
  storedAlarm[9] = '\0';  // Null-terminate the string

  alarmTime = String(storedAlarm);
  if (alarmTime != "") {
    Serial.println("Alarm read from EEPROM: " + alarmTime);
  } else {
    Serial.println("No alarm set.");
  }
}




// Reset EEPROM (for debugging purposes)
void resetEEPROM() {
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
  Serial.println("EEPROM has been cleared");
}
