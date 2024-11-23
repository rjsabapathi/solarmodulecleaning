
void Check_Robot_Status() {
  unsigned long currentMillis = millis();

  // Forward movement
  if (Robot_Status == 1) {
    if (currentMillis - lastSpeedUpdateMillis >= speedUpdateInterval) {
      lastSpeedUpdateMillis = currentMillis;

      // Gradually increase motor speeds until they reach the max speed
      if (motor1Speed <= motor1MaxSpeed && motor2Speed <= motor2MaxSpeed) {
        motor1Speed += motor1SpeedIncrement;
        motor2Speed += motor2SpeedIncrement;

        if (motor1Speed > motor1MaxSpeed) motor1Speed = motor1MaxSpeed;
        if (motor2Speed > motor2MaxSpeed) motor2Speed = motor2MaxSpeed;

        motor1.setSpeed(motor1Speed);   // Motor 1 forward speed
        motor2.setSpeed(-motor2Speed);  // Motor 2 reverse speed (brush)

        Serial.print("Drive Speed forward: ");
        Serial.print(motor1Speed);
        Serial.print(" | Brush Speed forward: ");
        Serial.println(motor2Speed);
      } else {
        motor1.setSpeed(motor1MaxSpeed);   // Motor 1 at full speed
        motor2.setSpeed(-motor2MaxSpeed);  // Motor 2 at full speed (brush)

        Serial.println("Motors at full speed (forward)");
        Serial.print("Robot_Status: ");
        Serial.println(Robot_Status);
      }
    }
  }




// Reverse movement
else if (Robot_Status == 2) {
    if (!reverseDelayActive) {
        // Stop the motors initially when switching to reverse
        motor1.setSpeed(0);
        motor2.setSpeed(0);

       DateTime now = rtc.now();
       Reverse_Time = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()); 
        Serial.println("Motors stopped, waiting 2 seconds before reversing...");
        Serial.println("Reverse Time:");
        Serial.print( Reverse_Time);



        // Start the delay timer
        reverseDelayStart = currentMillis;
        reverseDelayActive = true;
    }

    // Check if 2 seconds have passed since stopping the motors
    if (reverseDelayActive && currentMillis - reverseDelayStart >= reverseDelayDuration) {
              reverseDelayStart = 0;
        // Reset speeds to zero before ramping up in reverse
        if (motor1Speed == 0 && motor2Speed == 0) {
            Serial.println("Motors ready to reverse...");
        }

        if (currentMillis - lastSpeedUpdateMillis >= speedUpdateInterval) {
            lastSpeedUpdateMillis = currentMillis;

            // Gradually increase motor speeds in reverse
            if (motor1Speed < motor1MaxSpeed || motor2Speed < motor2MaxSpeed) {
                motor1Speed += motor1SpeedIncrement;
                motor2Speed += motor2SpeedIncrement;

                if (motor1Speed > motor1MaxSpeed) motor1Speed = motor1MaxSpeed;
                if (motor2Speed > motor2MaxSpeed) motor2Speed = motor2MaxSpeed;

                motor1.setSpeed(-motor1Speed);   // Motor 1 reverse speed
                motor2.setSpeed(motor2Speed);    // Motor 2 forward speed (brush in reverse)

                Serial.print("Drive Speed reverse: ");
                Serial.print(motor1Speed);
                Serial.print(" | Brush Speed reverse: ");
                Serial.println(motor2Speed);
            } else {
                motor1.setSpeed(-motor1MaxSpeed);   // Motor 1 at full reverse speed
                motor2.setSpeed(motor2MaxSpeed);    // Motor 2 at full reverse speed (brush)

                Serial.println("Motors at full speed (reverse)");
                Serial.print("Robot_Status: ");
                Serial.println(Robot_Status);
            }
        }
      }
    }

 // STOP
else if (Robot_Status == 3) {
    
        // Stop the motors initially when switching to reverse
        motor1.setSpeed(0);
        motor2.setSpeed(0);

        Serial.println("Motors stopped,");

        motor1Speed = 0;
        motor2Speed = 0;
        DateTime now = rtc.now();
        homeTime = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()); 
         
        return(displayTimeAndTemperature());
    }   
    
    
    
    
    }
