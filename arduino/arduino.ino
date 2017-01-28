#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

SoftwareSerial mySerial(7, 8); // RX, TX
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

String msg, date, hora;
int brightness = 0;
float temperature;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  mySerial.setTimeout(50);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);
  lcd.begin();
  lcd.backlight();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mySerial.available()) {
    msg = mySerial.readString();
    Serial.println(msg);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(msg);
    if (root.success()){
      if(root["date"]){
        date = root["date"].asString();
        Serial.println(date);
      }
      if(root["time"]){
        hora = root["time"].asString();
        Serial.println(hora);
      }
      if(root["temperature"]){
        temperature = root["temperature"];
        Serial.println(temperature);
      }
      if(root["brightness"] > -1){
        brightness = root["brightness"];
        Serial.println(brightness);
        analogWrite(6, brightness);
        analogWrite(5, brightness);
        analogWrite(3, brightness);
      }
      lcd.setCursor(0, 0);
      lcd.print(date + " " + hora);
      lcd.setCursor(0, 1);
      lcd.print("Temp " + String(temperature) + " " + (char)223 + "C");
    }
  }
}
