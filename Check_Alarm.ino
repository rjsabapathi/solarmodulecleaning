
// Function to check if the current time matches the alarm time
void checkAlarm() {
  DateTime now = rtc.now();
  
  // Extract hours, minutes, and seconds from current RTC time
  String currentTime = "";
  if (now.hour() < 10) currentTime += "0";  // Pad single digits with a leading zero
  currentTime += String(now.hour()) + ":";
  
  if (now.minute() < 10) currentTime += "0";
  currentTime += String(now.minute()) + ":";

  if (now.second() < 10) currentTime += "0";
  currentTime += String(now.second());

  // Compare with alarm time
  if (currentTime == alarmTime) {
    // Display on LCD at row 0, column 3 "Time matches"
    lcd.setCursor(0, 3);
    lcd.print("Time matches      ");
    
    // Also display on the Serial Monitor
    Serial.println("Time matches the alarm!");

    Robot_Status = 1; // time matches so robot ststus set 1 forward
  
  }

}