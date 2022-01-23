#define BROKER_IP    "192.168.97.81" //MQTT Broker -> IP of the machine where is the 
#define DEV_NAME     "montserrat" //MQTT Broker -> name of the connection
#define MQTT_USER    "root" //MQTT Broker -> credentials (username)
#define MQTT_PW      "root" //MQTT Broker -> credentials (password)

const char ssid[] = "Lab-IoT"; //Network settings -> name of SSID
const char pass[] = "iscteloralab#1"; //Network settings -> passowrd of SSID
const int motion_sensor_pin = 12;//Motion Sensor -> the pin that the motion sensor is attached to
const int servo_pin = 32;//Servo -> the pin that the servo is attached to

#include <MQTT.h> //include to use MQTT
#include <ESP32Servo.h> //include to manage the servo
#include <WiFi.h> //include to manage Wifi

WiFiClient net;
MQTTClient client;
Servo servo;

unsigned long lastMillis = 0;
int val = 0; //Motion Sensor -> default value of the motion sensor (0 = no detection)
int pos = 0; //Servo -> default value of the servo position

void connect() {
  Serial.print("checking wifi...");
  //connect to WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  //connect to MQTT Broker
  while (!client.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  //subscribe to the /opendoor topic on MQTT Broker 
  client.subscribe("/opendoor");
}

void messageReceived(String &topic, String &payload) {
  //whenever a message is received through MQTT
  if(topic == "/opendoor") {
    //only open the door if the topic is /opendoor and if the payload is open
    if(payload == "open")
    {
      if(pos == 0) {
        pos = 180;
      } else {
        pos = 0;
      }
      servo.write(pos); //rotate the servo
    }
  }
  Serial.println("incoming: " + topic + " - " + payload);
}

void setup() {
  pinMode(motion_sensor_pin, INPUT); //setup pin to motion sensor
  Serial.begin(9600); //frequency for serial monitor
  WiFi.begin(ssid, pass); //setup WiFi
  servo.attach(servo_pin); //setup pin to servo 
  client.begin(BROKER_IP, 1883, net); //setup MQTT client 
  client.onMessage(messageReceived); //define function to listen the messages from MQTT
  connect(); //connect wifi and mqtt
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect();
  }
  //if passed 1 second
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();

    val = digitalRead(motion_sensor_pin); //read the value of the motion sensor
  
    if (val == HIGH) { // check if the sensor is HIGH -> and if was detected motion
      Serial.println("Motion detected!");
      client.publish("/motion", "motion detected"); //publish a message to MQTT alerting that a motion was detected
    }
  }  
}
