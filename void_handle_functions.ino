

//******************************************************************** Function to handle GET status requests ************************************************************************************************

void handleGetStatus() {  // This functin get all data from robot send througn the server

  String Robotstatus;
  //String ALARM_TIME = alarmTime;

  if (Robot_Status == 1) {

    Robotstatus = "FORWARD";
  } 
  else if (Robot_Status == 2) {

   Robotstatus = "REVERSE";
  } 
  else if (Robot_Status == 3 || Robot_Status == 0) {  // Assuming 0 means not set or null

    Robotstatus = "STOP";
  } 
        

    // Fetch current time from RTC
    DateTime now = rtc.now();
    String formattedTime = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

   //Temperature get from RTC ds3232
    float Temp = rtc.getTemperature();

         String tem = String(Temp);


    // Create the JSON response
    String jsonResponse = "{";
    jsonResponse += "\"time\":\"" + formattedTime + "\",";
    jsonResponse += "\"voltage\":\"" + String(busVoltage, 2) + "\",";
    jsonResponse += "\"current\":\"" + String(shuntVoltage, 2) + "\",";
    jsonResponse += "\"robotStatus\":\"" + Robotstatus + "\","; // Convert Robot_Status to String
    jsonResponse += "\"AlarmTime\":\"" + alarmTime + "\","; // alarem time
    jsonResponse += "\"Motor1MaxSpeed\":\"" + String(motor1MaxSpeed) +"\",";// Drive motor speed
    jsonResponse += "\"Motor2MaxSpeed\":\"" + String(motor2MaxSpeed) +"\",";// Brush motor speed
    jsonResponse +="\"temperature\":\"" + String(tem) +"\"";//Trmprature from RTC ds3232
    jsonResponse += "}";

    // Send JSON response
    server.send(200, "application/json", jsonResponse);
}



//****************************************************************************SET ALARAM TIME*************************************************************************************************************************************************************

// Handle setting the alarm time from a POST request
void handleSetAlarm() {
    if (server.hasArg("plain")) {  // Check if there's a body in the request
        String body = server.arg("plain");  // Get the body
        DynamicJsonDocument doc(1024);  // Allocate memory for JSON document
        deserializeJson(doc, body);  // Deserialize the JSON

        String timeString = doc["time"];  // Extract the time string
        setAlarmTime(timeString);  // Set the alarm time
        server.send(200, "application/json", "{\"status\":\"success\"}");  // Send success response
    } else {
        server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"No data received\"}");  // Send error response
    }
}



//**************************************************************MANUAL OPERATIONS*******************************************************************************************************************************************************************************
void handleRootCommand() { 
  if (server.hasArg("plain")) {  // Check if there's a JSON payload
    String command = server.arg("plain");  // Get the raw JSON payload

    // Parse the JSON command
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, command);

    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
      return;
    }

    // Extract the "command" value from the parsed JSON
    String receivedCommand = doc["command"];

    if (receivedCommand == "forward") {
      // Execute the forward movement
      Serial.println("Moving forward...");
      Robot_Status = 1;// if 0 = robot stop ; 1=robot forward; 2=robot reverse



    } else if (receivedCommand == "reverse") {
      // Execute the reverse movement
      Serial.println("Moving in reverse...");
      Robot_Status = 2;// if 0 = robot stop ; 1=robot forward; 2=robot reverse

    } else if (receivedCommand == "stop") {
      // Stop the robot
      Serial.println("Stopping the robot...");
      Robot_Status = 3;// if 0 = robot stop ; 1=robot forward; 2=robot reverse
 

    } else {
      // Unrecognized command
      Serial.println("Unknown command received: " + receivedCommand);
      server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"Unknown command\"}");
      return;
    }

    // Send success response to the client
    server.send(200, "application/json", "{\"status\":\"success\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"No command received\"}");
  }
}


//**********************************************************************************Drive_Motor_Speed_Control**************************************************************************************************************************************************8 
// Function to handle setting drive motor speed
void handleSetDriveSpeed() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024); // Adjust size as needed
    deserializeJson(doc, body);

    if (doc.containsKey("driveSpeed")) {
       motor1MaxSpeed = doc["driveSpeed"];
      EEPROM.write(EEPROM_MOTOR1_MAX_SPEED_ADDR, motor1MaxSpeed);
      EEPROM.commit();  // Commit the write operation
      // Add code to set the motor speed here (e.g., PWM signal)
      Serial.println("Drive Motor Speed set to: " + String(motor1MaxSpeed));
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing driveSpeed\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}


//**********************************************************************************************Brush motor Speed control**********************************************************************************************************************************************************************

// Function to handle setting brush motor speed
void handleSetBrushSpeed() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    DynamicJsonDocument doc(1024); // Adjust size as needed
    deserializeJson(doc, body);

    if (doc.containsKey("brushSpeed")) {
      motor2MaxSpeed = doc["brushSpeed"];
      EEPROM.write(EEPROM_MOTOR2_MAX_SPEED_ADDR, motor2MaxSpeed);
      EEPROM.commit();  // Commit the write operation
      // Add code to set the motor speed here (e.g., PWM signal)
      Serial.println("Brush Motor Speed set to: " + String( motor2MaxSpeed));
      server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Missing brushSpeed\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}



//******************************************************************************Set Real Time Clock*****************************************************************************************************************************************************************************************************************************************************8

// Function to handle setting Real Time (RTC - DS3232)
void handlesetrealTime() {
    if (server.hasArg("plain")) {  // Check for request body
        String body = server.arg("plain");  // Get body content
        DynamicJsonDocument doc(1024);  // JSON document

        // Deserialize JSON and check for errors
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.c_str());
            server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"Invalid JSON\"}");
            return;
        }

        // Extract real-time string from JSON
        String realtimeString = doc["Set_time"].as<String>();
        setRTCTime(realtimeString);  // Call function to set RTC time

        // Send success response
        server.send(200, "application/json", "{\"status\":\"success\"}");
    } else {
        // Send error if no data received
        server.send(400, "application/json", "{\"status\":\"error\", \"message\":\"No data received\"}");
    }
}




