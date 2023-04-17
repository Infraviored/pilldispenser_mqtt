#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"


WiFiClient espClient;
PubSubClient client(espClient);

// Declare a global servo object
Servo currentServo;











void handleMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.println("Command received");
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if (strcmp(topic, "pilldispenser/commands") == 0) {
    if (message == "dispense_zinc") {
      Serial.println("Dispense zinc");
      dispense(zinc);
    } else if (message == "dispense_omega") {
      Serial.println("Dispense omega");
      dispense(omega);
    } else if (message == "dispense_magnesium") {
      Serial.println("Dispense magnesium");
      dispense(magnesium);
    } else {
      Serial.println("Command unknown");
    }
  }
}



void setup() {
  // Attach the servo to pin D2
  //180 is bottom, 0 is top

  // myServo.attach(D2, lowestPWM, highestPWM);
  // myServo.write(180);

  // Set the sensor pin as an input
  pinMode(SENSOR_PIN, INPUT);

  // Initialize the serial console
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(handleMqttMessage);
  while (!client.connected()) {
    if (client.connect("pilldispenser", mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe("pilldispenser/commands");
      client.publish("pilldispenser/state", "ready");
    } else {
      Serial.print("Failed to connect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop() {
  // Reconnect to MQTT broker if necessary
  if (!client.connected()) {
    Serial.println("Lost connection to MQTT broker, reconnecting...");
    if (client.connect("pilldispenser", mqtt_user, mqtt_pass)) {
      Serial.println("Reconnected to MQTT broker");
      client.subscribe("pilldispenser/commands");
    } else {
      Serial.print("Failed to reconnect to MQTT broker, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
      return;
    }
  }

  // Handle incoming MQTT messages
  client.loop();
}















void moveTo(int *servo, int position, int duration){
  int oldPosition = servo[4];
  int deltaPos = abs(oldPosition - position);
  currentServo.attach(servo[0], lowestPWM, highestPWM);

  if (duration == 0 || deltaPos*maxRate > duration){ //no duration given or duration smaller than minimal Time

  currentServo.write(position);
  delay(maxRate*deltaPos);
  }

  else {
    if (oldPosition <= position){
      for (int pos = oldPosition; pos <= position; pos += 1) {
        currentServo.write(pos);
        delay(duration/deltaPos);
      }

    } else{
      for (int pos = oldPosition; pos >= position; pos -= 1) {
        currentServo.write(pos);
        delay(duration/deltaPos);
      }
    }
  }


  currentServo.detach();
  servo[4] = position;

}





void shakeBottom(int *servo){
  int lowpos = servo[1];
  int highpos = lowpos - servo[5];
  for (int j = 0; j < 8; j++) {
    moveTo(servo, lowpos, 0);
    moveTo(servo, highpos, 0);
  }
}




bool dropSense(int *servo) {
  //pre drop location is LARGER
  int preDropLocation = servo[2];
  int dropLocation = servo[3];
  int duration = servo[6];

  // Move to pre drop location
  moveTo(servo, preDropLocation, 800);

  // Attach servo
  currentServo.attach(servo[0], lowestPWM, highestPWM);

  // Calculate distance to drop location
  float deltaPos = abs(preDropLocation - dropLocation);

  bool dropped = false;

  int loopdelay = 2;
  int maxLoopStep = duration / loopdelay;

  // First loop: Read and move for 1/2 second
  for (int i = 0; i < 20; i++) {
    // Read sensor value and move servo
    int sensorValue = analogRead(SENSOR_PIN);
    int pos = preDropLocation - deltaPos * ((float)i / (float)20);
    currentServo.write(pos);

    // Check if threshold is reached
    if (sensorValue > threshold) {
      Serial.println("DROP");
      dropped = true;
      break;
    }
  }

  // Second loop: Read only, Skip when already dropped.
  if (!dropped) {
    for (int i = 0; i <= maxLoopStep; i++) {
      int sensorValue = analogRead(SENSOR_PIN);

      // Check if threshold is reached
      if (sensorValue > threshold) {
        Serial.println("DROP");
        dropped = true;
        break;
      }

      delay(loopdelay);
    }
  }

  // Detach servo and return result
  currentServo.detach();
  if (dropped) {
    return true;
  } else {
    Serial.println('-');
    return false;
  }
}



void dispense(int *servo){
  client.publish("pilldispenser/state", "dispensing");

  for (int i=0; i<5; i++){
    moveTo(servo, servo[1], 0);
    shakeBottom(servo);

    if (dropSense(servo)){
      Serial.println("Dispensed!");
      client.publish("pilldispenser/state", "ready");
      return;
    }
    client.loop();
  }
  Serial.println("Error: Pill dispenser empty!");
  client.publish("pilldispenser/state", "error");
}
