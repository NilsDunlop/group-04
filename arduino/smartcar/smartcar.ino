#include <vector>

#include <MQTT.h>
#include <WiFi.h>
#ifdef __SMCE__
#include <OV767X.h>
#endif

#include <Smartcar.h>

#ifndef __SMCE__
WiFiClient net;
#endif
MQTTClient mqtt;

ArduinoRuntime arduinoRuntime;
BrushedMotor leftMotor(arduinoRuntime, smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(arduinoRuntime, smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control(leftMotor, rightMotor);

SimpleCar car(control);

const auto oneSecond = 1UL;
const auto triggerPin = 6;
const auto echoPin = 7;
const auto maxDistance = 400;
const int TRIGGER_PIN           = 6; // D6
const int ECHO_PIN              = 7; // D7
const unsigned int MAX_DISTANCE = 100;
const int SIDE_FRONT_PIN = 0;
const int stoppingSpeed = 0; 
const int stopAngle = 0;
const int stopDistance = 80;
const int autoSpeed = 60;
const int autoAngle = 90;
int carSpeed = 0;
int autoDriving = 0;

SR04 frontSensorUS(arduinoRuntime, triggerPin, echoPin, maxDistance);
GP2D120 fronSensorIR(arduinoRuntime, SIDE_FRONT_PIN);

std::vector<char> frameBuffer;

void setup() {
  Serial.begin(9600);
#ifdef __SMCE__
Camera.begin(QQVGA, RGB888, 15);
  frameBuffer.resize(Camera.width() * Camera.height() * Camera.bytesPerPixel());
  mqtt.begin("3.138.188.190", 1883, WiFi);
  //mqtt.begin("aerostun.dev", 1883, WiFi);
  //mqtt.begin(WiFi); // Will connect to localhost
#else
  mqtt.begin(net);
#endif
  if (mqtt.connect("arduino", "public", "public")) {
    mqtt.subscribe("/smartcar/control/#", 1);
    mqtt.onMessage([](String topic, String message) {
      if (topic == "/smartcar/control/speed" && autoDriving == 0) {
        carSpeed = message.toInt();
        if (obstacleAvoidance() && carSpeed <=0){
          car.setSpeed(carSpeed);
        }
        car.setSpeed(carSpeed);
      } else if (topic == "/smartcar/control/turning" && autoDriving == 0) {
        car.setAngle(message.toInt());
      } else if (topic == "/smartcar/control/auto") {
        autoDriving = message.toInt();
        Serial.println(autoDriving);
        if (autoDriving == 0){
          car.setSpeed(stoppingSpeed);
          car.setAngle(stopAngle);
          carSpeed = stoppingSpeed;
        } else {
          car.setSpeed(autoSpeed);
          carSpeed = autoSpeed;
        }
      } else {
        Serial.println(topic + " " + message);
      }
    });
  }
}

void loop() {
  if (mqtt.connected()) {
    mqtt.loop();
    const auto currentTime = millis();
#ifdef __SMCE__
    static auto previousTransmission = 0UL;
    if (currentTime - previousTransmission >= oneSecond) {
      previousTransmission = currentTime;
      Camera.readFrame(frameBuffer.data());
      mqtt.publish("Camera_Stream", frameBuffer.data(), frameBuffer.size(), false, 0);
    }
#endif
  }
#ifdef __SMCE__
  // Avoid over-using the CPU if we are running in the emulator
  obstacleAvoidance();
  delay(35);
#endif
}

boolean obstacleAvoidance()
{
     int distanceFromObject = frontSensorUS.getDistance();
     if (distanceFromObject < stopDistance && distanceFromObject > 1 && carSpeed > 0){
      if (autoDriving == 0){
        car.setSpeed(stoppingSpeed);
      } else {
        autonomousMoving();
      }
      return true;
     } else {
          //Serial.println(distanceFromObject);
          return false;
     }
}

void autonomousMoving(){
  car.setSpeed(stoppingSpeed);
        delay(1000);
        car.setSpeed(-autoSpeed);
        delay(1000);
        car.setSpeed(stoppingSpeed);
        delay(1000);
        car.setSpeed(autoSpeed);
        car.setAngle(-autoAngle);
        delay(1000);
        car.setSpeed(stoppingSpeed);
        car.setAngle(stopAngle);
        delay(1000);
        car.setSpeed(autoSpeed);
        delay(1000);
}
