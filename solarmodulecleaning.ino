#include <Wire.h>
#include <EEPROM.h>             
#include "RTClib.h"
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include "CytronMotorDriver.h"
#include <Adafruit_INA219.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

const char* host = "esp32";
const char* ssid = "TP-Link_E0D4";
const char* password = "35330424";

WebServer server(80);

String RobotId= " ";

//AsyncWebServer server(80); 


// Constants for EEPROM memory layout
const int EEPROM_SIZE = 512;
const int EEPROM_ALARM_ADDR = 0;  // Starting address to store alarm time

const int EEPROM_MOTOR1_MAX_SPEED_ADDR = 20;  // EEPROM address for motor 1 max speed
const int EEPROM_MOTOR2_MAX_SPEED_ADDR = 24;  // EEPROM address for motor 2 max speed


//Cytron motor driver  pin initilization
CytronMD motor1(PWM_DIR, 5, 18);  // PWM = Pin 3, DIR = Pin 18.//drive motor
CytronMD motor2(PWM_DIR, 0,  4);  //PWM = Pin 0 , DIR = Pin 4.//Brush motor

// Create an INA219 instance
Adafruit_INA219 ina219;

RTC_DS3231 rtc;

LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long previousMillis = 0;  // Stores last time the display was updated
const long interval = 1000;        // Update interval (1 second)

String inputString = "";  // A string to hold incoming JSON data
bool stringComplete = false;  // Whether the string is complete


String alarmTime = "";  // String to store the alarm time
String Reverse_Time ="";//end sensor trigging time
String homeTime = "";// home sensor triggring time


int Robot_Status = 0;// if 0 = robot stop ; 1=robot forward; 2=robot reverse


int motor1MaxSpeed = 255;  // Maximum speed for motor1
int motor2MaxSpeed = 255;  // Maximum speed for motor2


int motor1Speed = 0;
int motor2Speed = 0;

const int motor1SpeedIncrement = 1;
const int motor2SpeedIncrement = 1;
unsigned long lastSpeedUpdateMillis = 0;
const unsigned long speedUpdateInterval = 100;  // Time between speed increments

//*****************************************************Sensor intilization********************************************************************************************************************************************************************

int Endsensor = 2; // End Sensor 

int Homesensor = 15; // Home Sensor


//*****************************************************Current Sensor ina219 intilization********************************************************************************************************************************************************************

 float shuntVoltage = 0.00;
 float busVoltage = 0.00;

unsigned long reverseDelayStart = 0;  // Tracks the time when reverse mode starts
bool reverseDelayActive = false;      // Indicates whether the delay is active
const unsigned long reverseDelayDuration = 2000;  // 2 seconds delay


unsigned long lastVoltageCurrentUpdate = 0;  // Track the last time voltage and current were updated
const unsigned long updateInterval = 1000;   // Update every 1 second (1000 ms)


//*************************************************************************************STATIC IP ADDRESS*****************************************************************************************************************************************************************************************************

// Set your Static IP address
IPAddress local_IP(172, 16, 10, 144);
// Set your Gateway IP address
IPAddress gateway(172, 16, 10, 10);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional


//***************************************************************************CSS,HTML,JS********************************************************************************************************************************************************************************************************
String style = 
"<style>"
"body {"
"   background-color: #f0f0f0; /* Fallback color */"
//"  background: url('https://drive.google.com/uc?id=1PTh7RNKUFwvusKYi9tSqOHtvY0DzjKxj') no-repeat center center fixed; "
"background-image: url('data:image/jpeg;base64,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');"
"  background-size: cover; "
"  font-family: sans-serif; "
"  font-size: 14px; "
"  color: #fff; "
"  text-align: center; "
"  padding: 10px;"
"}"
"#file-input, input, .btn {height: 44px; border-radius: 4px; font-size: 15px;}"
"input, .btn {background: #f1f1f1; border: 0; padding: 0 15px; color: #3498db; margin-bottom: 10px;}"
"form {background: rgba(34, 34, 34, 0.8); max-width: 400px; margin: 75px auto; padding: 20px; border-radius: 5px; text-align: left; color: #fff;}"
"#file-container {display: flex; justify-content: flex-end; align-items: center; gap: 10px; margin-bottom: 20px;}"
"#file-input {padding: 0; background: #3498db; color: #fff; cursor: pointer; width: auto;}"
"#bar, #prgbar {background-color: #f1f1f1; border-radius: 10px; width: 100%; height: 10px;}"
"#bar {background-color: #3498db; width: 0%; height: 10px;}"
"#alarmTime {width: 120px; margin-bottom: 10px;}"
"h2, h3 {color: #fff; margin-bottom: 10px;}"
"#status {margin-bottom: 20px; color: #fff;}"
"#driveSlider, #brushSlider {width: 300px;}"
".container {display: flex; flex-direction: column; align-items: flex-start; margin-top: 20px;}"



// Media query for mobile devices
"@media (max-width: 600px) {"
"body {padding: 5px;}"
"form {padding: 10px; max-width: 90%; margin: 50px auto;}"
"#driveSlider, #brushSlider {width: 100%;}"
".container {align-items: center;}"
"}"

// Media query for tablets and laptops
"@media (min-width: 600px) {"
"body {padding: 20px;}"
"form {padding: 20px; max-width: 400px; margin: 75px auto;}"
"}"

"</style>";

String loginIndex = 
"<form name='loginForm'>"
"<h1>HILD ENERGY SOLAR MODULE CLEANING SYSTEM LOGIN</h1>"
"<input name='userid' placeholder='User ID'>"
"<input name='pwd' placeholder='Password' type='Password'>"
"<input type='button' onclick='check(this.form)' class='btn' value='Login'></form>"
"<script>"
"function check(form) {"
"if(form.userid.value === 'admin' && form.pwd.value === 'admin') {"
"window.location.replace('/serverIndex');"
"} else {"
"alert('Error Password or Username');"
"}"
"}"
"</script>" + style;

//************************************************************************************************************************************************************************************************

String serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"

"<h2>HILD ENERGY SOLAR MODULE CLEANING SYSTEM</h2>"
// File Upload Section
"<div id='file-container'>"
"<label id='file-input' for='file'>Choose file...</label>"
"<input type='file' name='update' id='file' onchange='sub(this)' style='display:none'>"
"<input type='submit' class='btn' value='Upload'>"
"</div>"
"<div id='prgbar'><div id='bar'></div></div>"
"<p id='prg'>Progress: 0%</p>"

// Container for all options
"<div class='container'>"

// Header and Current Status Section

"<h3>Current Status</h3>"
"<div id='status'>"
"Time: <span id='rtcTime'></span><br>"
"Voltage: <span id='voltage'></span> V<br>"
"Current: <span id='current'></span> A<br>"
"Robot Status: <span id='robotStatus'></span><br>"
"Alarm Time: <span id='AlarmTime'></span><br>"
"Drive Motor: <span id='Motor1MaxSpeed'></span><br>"
"Brush Motor: <span id='Motor2MaxSpeed'></span><br>"
"Temperature: <span id='temperature'></span> C<br>"
"</div>"

// Manual Controls for Robot Operation
"<h3>Manual Controls</h3>"
"<button type='button' class='btn' onclick='sendCommand(\"forward\")'>Forward</button>"
"<button type='button' class='btn' onclick='sendCommand(\"reverse\")'>Reverse</button>"
"<button type='button' class='btn' onclick='sendCommand(\"stop\")'>Stop</button><br><br>"

// Set Alarm Time
"<h3>Set Alarm Time</h3>"
"<input type='text' id='alarmTime' placeholder='HH:MM:SS' pattern='[0-2][0-3]:[0-5][0-9]:[0-5][0-9]' title='Please enter a time in HH:MM:SS format'>"
"<button type='button' class='btn' onclick='setAlarm()'>Set Alarm</button><br>"


// Set Real Time
"<h3>Set Real Time</h3>"
"<input type='text' id='RealTime' placeholder='HH:MM:SS' pattern='[0-2][0-3]:[0-5][0-9]:[0-5][0-9]' title='Please enter a time in HH:MM:SS format'>"
"<button type='button' class='btn' onclick='setRealTime()'>RTC</button><br>"


// Set Robot Speed
"<h3>Set Robot Speed</h3>"
"<h2>Drive Motor Speed</h2>"
"<input type='range' id='driveSlider' min='0' max='255' value='25' oninput='updateDriveSpeed(this.value)'>"
"<p>Value: <span id='driveSliderValue'>25</span></p>"

"<h2>Brush Motor Speed</h2>"
"<input type='range' id='brushSlider' min='0' max='255' value='25' oninput='updateBrushSpeed(this.value)'>"
"<p>Value: <span id='brushSliderValue'>25</span></p>"

// JavaScript section
"<script>"
"function sub(obj) {"
"var fileName = obj.value.split('\\\\');"
"document.getElementById('file-input').innerHTML = fileName[fileName.length - 1];"
"};"

"$('#upload_form').on('submit', function(e) {"
"e.preventDefault();"
"var formData = new FormData(this);"
"$.ajax({"
"url: '/update',"
"type: 'POST',"
"data: formData,"
"contentType: false,"
"processData: false,"
"xhr: function() {"
"var xhr = new window.XMLHttpRequest();"
"xhr.upload.addEventListener('progress', function(evt) {"
"if (evt.lengthComputable) {"
"var percentComplete = (evt.loaded / evt.total) * 100;"
"$('#prg').html('Progress: ' + Math.round(percentComplete) + '%');"
"$('#bar').css('width', percentComplete + '%');"
"}"
"}, false);"
"return xhr;"
"}"
"});"
"});"

// Function to send commands for manual controls
"function sendCommand(command) {"
"$.ajax({"
"url: '/command',"
"type: 'POST',"
"contentType: 'application/json',"
"data: JSON.stringify({ command: command }),"
"success: function(response) {"
"console.log('Command sent:', command);"
"}"
"});"
"}"

// Function to set alarm time
"function setAlarm() {"
"var alarmTime = document.getElementById('alarmTime').value;"
"$.ajax({"
"url: '/setAlarm',"
"type: 'POST',"
"contentType: 'application/json',"
"data: JSON.stringify({ command: 'alarm', time: alarmTime }),"
"success: function(response) {"
"console.log('Alarm set:', alarmTime);"
"alert('Alarm set to: ' + alarmTime);"
"}"
"});"
"}"

// JavaScript function to set Real Time
"function setRealTime() {"
"var RealTime = document.getElementById('RealTime').value;"  // Get input value
"$.ajax({"
"url: '/setRealTime',"       // Server endpoint
"type: 'POST',"
"contentType: 'application/json',"
"data: JSON.stringify({'Set_time': RealTime }),"  // JSON format expected by server
"success: function(response) {"
"console.log('Real time set to:', RealTime);"
"alert('Real time set to: ' + RealTime);"  // Notify user of success
"},"
"error: function() {"
"alert('Failed to set real time. Please try again.');"
"}"
"});"
"}"

// Function to update drive motor speed
"function updateDriveSpeed(value) {"
"document.getElementById('driveSliderValue').innerText = value;"
"$.ajax({"
"url: '/setDriveSpeed',"
"type: 'POST',"
"contentType: 'application/json',"
"data: JSON.stringify({ driveSpeed: value }),"
"success: function(response) {"
"console.log('Drive speed set to:', value);"
"}"
"});"
"}"

// Function to update brush motor speed
"function updateBrushSpeed(value) {"
"document.getElementById('brushSliderValue').innerText = value;"
"$.ajax({"
"url: '/setBrushSpeed',"
"type: 'POST',"
"contentType: 'application/json',"
"data: JSON.stringify({ brushSpeed: value }),"
"success: function(response) {"
"console.log('Brush speed set to:', value);"
"}"
"});"
"}"





// Fetch and update RTC time, voltage, current, and robot status
"setInterval(function() {"
"$.get('/getStatus', function(data) {"
"$('#rtcTime').text(data.time);"
"$('#voltage').text(data.voltage);"
"$('#current').text(data.current);"
"$('#robotStatus').text(data.robotStatus);"
"$('#AlarmTime').text(data.AlarmTime);"
"$('#Motor1MaxSpeed').text(data.Motor1MaxSpeed);"
"$('#Motor2MaxSpeed').text(data.Motor2MaxSpeed);"
"$('#temperature').text(data.temperature);"//Temperature


"});"
"}, 1000);"

"</script>" + style;



//*********************************************************void Setup*************************************************************************************************************************************************************************************8
void setup() {


Serial.begin(115200);



Wire.begin(21, 22);

// lcd initilization
  lcd.init();
  lcd.backlight();
 

 lcd.print("  WELCOME TO ");
 lcd.setCursor(0, 1); // Set cursor to line 2
 lcd.print("  HILD ENERGY");

 delay(700);

 lcd.clear();
 lcd.print("SOLAR CLEANING");
 lcd.setCursor(0, 1); // Set cursor to line 2
 lcd.print("ROBOT VER 3.0");
 delay(700);
 lcd.clear();




//***********************************************************************ina 219 current sensor check *********************************************************************************************************

  // Attempt to initialize the INA219
  if (!ina219.begin()) {
    // If initialization failed, print error to Serial and LCD
    Serial.println("Failed to find INA219 chip");
    lcd.setCursor(0,0);  // Set cursor to 0th row, 0th column
    lcd.print("INA219: Failed");  // Display failure message
    delay(2000);
  //  while (1) { delay(10); }  // Stay here if INA219 isn't found
  }
else{
  // If initialization succeeded, print success to Serial and LCD
  Serial.println("INA219 current sensor initialized!");
  lcd.setCursor(0,0);  // Set cursor to 0th row, 0th column
  lcd.print("INA219: OK");  // Display success message
  delay(1000);
  lcd.print("   ");




}
//******************************************************************** RTC INITILITION ********************************************************************************************************************************
  // Initialize the RTC module
  if (!rtc.begin()) {
    Serial.println("Failed to find find RTC");
    lcd.setCursor(0, 1);  // Set cursor to 0th row, 0th column
    lcd.print("RTC: Failed");  // Display failure message
    delay(2000);
    while (1) { delay(10); }; // Stop if RTC is not found
  }
  Serial.println("RTC sensor initialized!");
  lcd.setCursor(0, 1);  // Set cursor to 0th row, 0th column
  lcd.print("RTC: OK");  // Display success message
  delay(1000);

//******************************************************************** END AND HOME SENSOR INITILIZATION ****************************************************************************************************


  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Read the maximum motor speeds from EEPROM
  motor1MaxSpeed = EEPROM.read(EEPROM_MOTOR1_MAX_SPEED_ADDR);  // Read motor 1 max speed from EEPROM
  motor2MaxSpeed = EEPROM.read(EEPROM_MOTOR2_MAX_SPEED_ADDR);  // Read motor 2 max speed from EEPROM

   // Print the motor speeds to Serial
  Serial.print("Drive Motor Max Speed: ");
  Serial.println(motor1MaxSpeed);
  Serial.print("Brush Motor Max Speed: ");
  Serial.println(motor2MaxSpeed);

  // Display motor speeds on the LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Drive Speed: ");
  lcd.print(motor1MaxSpeed);
  lcd.setCursor(0,1);
  lcd.print("Brush Speed: ");
  lcd.print(motor2MaxSpeed);
  delay(1000);



  // Read the alarm time from EEPROM
  readAlarmFromEEPROM();
 
  delay(1000);
  // Display information on how to use JSON commands
  Serial.println("Enter JSON Payload commends:");


  Serial.println("{\"command\": \"clock\", \"time\": \"YYYY-MM-DD HH:MM:SS\"}");
  Serial.println("{\"command\": \"alarm\", \"time\": \"HH:MM:SS\"}");
  Serial.println("{\"command\": \"setSpeeds\", \"motor1MaxSpeed\": \"200\", \"motor2MaxSpeed\": \"150\"}");






  lcd.setCursor(0, 0);
  lcd.print(" *HILD ENERGY* ");




  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

 // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { 
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
 

  
  server.begin();

  server.on("/GetDetails", HTTP_GET, sendRobotDetailsToServer); 
  server.on("/CurrentTime",HTTP_POST, handleSetCurrentTime);

//******************************************************************************************************************************** web page control *******************************************************
  server.on("/getStatus", HTTP_GET, handleGetStatus);
  server.on("/setAlarm", HTTP_POST, handleSetAlarm);
  server.on("/setRealTime",HTTP_POST, handlesetrealTime);
  server.on("/command", HTTP_POST, handleRootCommand);  // Handle manual control commands
  server.on("/setDriveSpeed", HTTP_POST, handleSetDriveSpeed);
  server.on("/setBrushSpeed", HTTP_POST, handleSetBrushSpeed);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");


}


//********************************************************void loop********************************************************************************************************************************************************************
void loop() {



  // Read sensor status
  int end_sensor = digitalRead(Endsensor);
 

 int home_sensor =  digitalRead(Homesensor);
 
 server.handleClient();

  // Get Bus voltage in V
   busVoltage = ina219.getBusVoltage_V();


  //Get shunt voltage in mV
  shuntVoltage = ina219.getShuntVoltage_mV();




  // If the end sensor is triggered, reverse the robot
  if (end_sensor == 0) {
    Robot_Status = 2;  // Set status to reverse mode
  }


  else if(home_sensor == 0)
  {
  Robot_Status = 3;  // Set status to stop mode

  }


  // Check for serial input
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }

  if (stringComplete) {
    processJSON(inputString);
    inputString = "";
    stringComplete = false;
  }

  // Non-blocking delay mechanism using millis()
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Display the current time and temperature every 1 second
    displayTimeAndTemperature();
    
    // Check if RTC time matches the alarm time stored in EEPROM
    checkAlarm();


  }

  Check_Robot_Status();

}



void displayTimeAndTemperature() {
  DateTime now = rtc.now();

  // Display date and time on Serial Monitor
  Serial.print("Date: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" Time: ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);

  // Display temperature on Serial Monitor
  float temperaTure = rtc.getTemperature();
  Serial.print("Temperature: ");
  Serial.print(temperaTure);
  Serial.println(" C");

  // Get the current time (in milliseconds) to check if it is time to update voltage and current readings
  unsigned long currentMillis = millis();

  // Update voltage and current every second
  if (currentMillis - lastVoltageCurrentUpdate >= updateInterval) {
    lastVoltageCurrentUpdate = currentMillis;

    // Assume busVoltage and shuntVoltage variables are already defined and updated elsewhere in the code
    float busVoltage = ina219.getBusVoltage_V();  // Example, assuming INA219 for voltage reading
    float shuntVoltage = ina219.getCurrent_mA();  // Example, assuming INA219 for current reading

    // Display voltage and current on Serial Monitor
    Serial.print("             ");
    Serial.print("Voltage: ");
    Serial.print(busVoltage);
    Serial.println(" Volt");

    Serial.print("             ");
    Serial.print("Current: ");
    Serial.print(shuntVoltage);
    Serial.println(" Amps");

    // Display voltage and current on the LCD at position (0, 0)
    lcd.setCursor(0, 0);
    lcd.print("V: ");
    lcd.print(busVoltage);
    lcd.print("V ");

    lcd.print("I: ");
    float curnt= (shuntVoltage/10);
    lcd.print(curnt);
    lcd.print("A ");


    
  }

  // Display time on the LCD at position (0, 1)
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second());
  lcd.print("     ");
}

//******************************************************************************* for postman ******************************************************************************************************************************************************************
void sendRobotDetailsToServer() {

 DateTime now = rtc.now();
 

  String RobotStatus;

  if (Robot_Status == 1) {

    RobotStatus = "FORWARD";
  } 
  else if (Robot_Status == 2) {

   RobotStatus = "REVERSE";
  } 
  else if (Robot_Status == 3 || Robot_Status == 0) {  // Assuming 0 means not set or null

    RobotStatus = "STOP";
  } 




int rssiValue = WiFi.RSSI(); // Get WiFi signal strength (RSSI)
String res = "{\"roboId\":\"" + String(RobotId) +
                  "\",\"serverIP\":\"" + WiFi.localIP().toString() +
                  "\",\"macAddress\":\"" + WiFi.macAddress() +
                 "\",\"rssi\":\"" + String(rssiValue) + " dBm" +
                  "\",\"currentTime\":\"" + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) +
                  "\",\"currentDate\":\"" + String(now.day()) + ":" + String(now.month()) + ":" + String (now.year()) +
                 "\",\"referenceTimeStatus\":\"" + String(alarmTime)  +
                  "\",\"startTime\":\"" + String(alarmTime) +
                  "\",\"returnTime\":\"" + String(Reverse_Time) +
                  "\",\"homeTime\":\""+String(homeTime)+
                  "\",\"batteryVoltage\":" + String(ina219.getBusVoltage_V()) +
                  ",\"current\":" + String(ina219.getShuntVoltage_mV()) +
                  ",\"drivemotorspeed\":" + String( motor1MaxSpeed )+
                  ",\"brushmotorspeed\":"+ String( motor2MaxSpeed )+
                 ",\"roboState\":\"" + RobotStatus + "\"" +
                  "}";


  server.send(200, "text/json", res);
  Serial.println("response  robotid");
  Serial.println(res);



}





//****************************************************************** for postman *********************************************************************************************************************************************************************
void handleSetCurrentTime()
{
  if(server.hasArg("plain")==false){
  }
  String body = server.arg("plain");

  StaticJsonDocument <250> jsonDocument;
  char buffer[250];
  deserializeJson(jsonDocument, body);
  if(jsonDocument.containsKey("currenthour") && jsonDocument.containsKey("currentminute") && jsonDocument.containsKey("currentdate")){
    int hour = jsonDocument["currenthour"];
    int minute = jsonDocument["currentminute"];
    String current_date=jsonDocument["currentdate"];
     Serial.print("CurrentHour: ");
     Serial.print(hour);
     Serial.print("CurrentMinute: ");
     Serial.print(minute);
     Serial.print("CurrentDate");
     Serial.print(current_date);

  String responsetimestamp = "{\"currentHour\":"+String(hour)+",\"currentMinute\":"+String(minute)+",\"CurrentDate\":"+String(current_date)+"}";
  server.send(200, "application/json",  responsetimestamp);

  DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute));


 
}
else{
      server.send(500, "application/json", "errordata");
  }
}


