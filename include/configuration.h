#ifndef INCLUDE_CONFIGURATION_H_
#define INCLUDE_CONFIGURATION_H_

//WiFi settings
#ifndef WIFI_SSID
  #define WIFI_SSID "pelas" // Put you SSID and Password here
  #define WIFI_PWD "Qa13#Ty1977"
#endif

//LCD Address
#define I2C_LCD_ADDR 0x27

//Fan pin
#define FAN_1 5
#define FAN_2 4

#include <SmingCore/SmingCore.h>

void controlFan(bool state);
void onChangeLedBrightness();
void startWebServer();
void onNtpReceive(NtpClient& client, time_t timestamp);
void wsSendData(String json);
int getActiveSockets();
void setCurrentBrightness(int brightness);
#endif /* INCLUDE_CONFIGURATION_H_ */
