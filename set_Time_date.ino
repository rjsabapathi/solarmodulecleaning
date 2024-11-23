// Function to change clock time based on the input string "YYYY-MM-DD HH:MM:SS"
void changeClockTime(String timeString) {
  int year = timeString.substring(0, 4).toInt();
  int month = timeString.substring(5, 7).toInt();
  int day = timeString.substring(8, 10).toInt();
  int hour = timeString.substring(11, 13).toInt();
  int minute = timeString.substring(14, 16).toInt();
  int second = timeString.substring(17, 19).toInt();

  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("Clock set to: " + timeString);
}
// Function to change clock time based on the input string "YYYY-MM-DD HH:MM:SS"
void setRTCTime(String realtimeString) {
Serial.println("saba");
Serial.print(realtimeString);

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        return;
    }
    
    // Check if the RTC lost power and if so, reset it
    if (rtc.lostPower()) {
        Serial.println("RTC lost power, resetting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    int hour = realtimeString.substring(0, 2).toInt();
    int minute = realtimeString.substring(3, 5).toInt();
    int second = realtimeString.substring(6, 8).toInt();

    // Get current date to retain while updating time
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));

    Serial.print("RTC time set to: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
}