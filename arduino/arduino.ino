#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(7, 8); // RX, TX
String msg;
int brightness = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  mySerial.setTimeout(50);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mySerial.available()) {
    msg = mySerial.readString();
//    Serial.println("arduino "+ msg);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(msg);
    if (root.success()){
      brightness = root["brightness"];
      Serial.println(brightness);
      analogWrite(6, brightness);
      analogWrite(5, brightness);
      analogWrite(3, brightness);
    }
  }
}
