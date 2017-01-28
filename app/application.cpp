#include <user_config.h>
#include <configuration.h>
#include <Services/ArduinoJson/ArduinoJson.h>
#include <Libraries/OneWire/OneWire.h>
#include <Libraries/DS18S20/ds18s20.h>

Timer lcdTimer;
int8_t currentBrightness = 0; // 0 - 59
int8_t startFadeIn = 6; // Start fade in at 7 hrs
int8_t startFadeOut = 18; // start fade out at 18 hrs
bool automatic = true;
bool fan = false;
int activeSockets = 0;
DriverPWM fanPWM;
DS18S20 ReadTemp;

//ntp
NtpClient ntpClient ("mx.pool.ntp.org", 300, onNtpReceive);

void setActiveSockets(int sockets){
	activeSockets = sockets;
}

// Will be called when receive ntp response
void onNtpReceive(NtpClient& client, time_t timestamp) {
	//System timezone is LOCAL so to set it from UTC we specify TZ
	SystemClock.setTime(timestamp, eTZ_UTC);
	// Serial.printf("Time synchronized: %s\n", SystemClock.getSystemTimeString().c_str());
	debugf("NTP RECEIVE!");
}

// Will be called when WiFi station was connected to AP
void connectOk(){
	debugf("I'm CONNECTED");
	// Serial.println(WifiStation.getIP().toString());
	//Update system clock
	ntpClient.requestTime();
	//start web server
	startWebServer();
}

// Will be called when WiFi station timeout was reached
void connectFail(){
	debugf("I'm NOT CONNECTED!");
	// Repeat and check again
	WifiStation.waitConnection(connectOk, 10, connectFail);
}

//Turn on/off fan
void controlFan(bool state){
	fan = state;
	if(state == false){
		fanPWM.analogWrite(FAN_1, 0);
		fanPWM.analogWrite(FAN_2, 0);
	}else{
		fanPWM.analogWrite(FAN_1, 140);
		fanPWM.analogWrite(FAN_2, 120);
	}
}

float getTemperature(){
	uint8_t a;
	uint64_t info;
	float temperature = 0;
	if (!ReadTemp.MeasureStatus()){
	  if (ReadTemp.GetSensorsCount()){   // is minimum 1 sensor detected ?
	    for(a=0;a<ReadTemp.GetSensorsCount();a++){   // prints for all sensors
	      if (ReadTemp.IsValidTemperature(a)){   // temperature read correctly ?
					temperature = ReadTemp.GetCelsius(a);
        }
	    }
		}
		ReadTemp.StartMeasure();  // next measure, result after 1.2 seconds * number of sensors
		return temperature;
	}
	else
		return temperature;
}

//Set led's brightness
void controlLED(){
	DateTime _date_time = SystemClock.now();
	float temperature = getTemperature();
	StaticJsonBuffer<200> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	int value = ((255/59) * currentBrightness);
	root["brightness"] = value;
	root["date"] = _date_time.toShortDateString();
	root["time"] = _date_time.toShortTimeString(false);
	root["temperature"] = temperature;
	String json;
	root.printTo(json);
	Serial.print(json);
}

void setMode(bool  mode){
	automatic = mode;
}

void setCurrentBrightness(int brightness){
	currentBrightness = brightness;
	setMode(false);
	controlLED();
}

//Update LCD
void onLCDPrint() {
	activeSockets = getActiveSockets();
	if(activeSockets > 0){
		StaticJsonBuffer<200> jsonBuffer;
		JsonObject& root = jsonBuffer.createObject();
		root["brightness"] = currentBrightness;
		root["fans"] = fan;
		root["fadein"] = startFadeIn;
		root["fadeout"] = startFadeOut;
		String json;
		root.printTo(json);
		wsSendData(json);
	}
	if(automatic)
		onChangeLedBrightness();
	else
		controlLED();
}

//Check  hour and adjust brightness
void onChangeLedBrightness(){
	DateTime _date_time = SystemClock.now();
	if(_date_time.Hour == startFadeIn){
		currentBrightness = _date_time.Minute;
		controlFan(true);
	}else if(_date_time.Hour > startFadeIn && _date_time.Hour  < startFadeOut){
		currentBrightness = 59;
		controlFan(true);
	}else if(_date_time.Hour == startFadeOut){
		currentBrightness = 59 - _date_time.Minute;
		controlFan(true);
	}else if(_date_time.Hour > startFadeOut || _date_time.Hour < startFadeIn){
		currentBrightness = 0;
		controlFan(false);
	}
	controlLED();
}

// Will be called when WiFi hardware and software initialization was finished
// And system initialization was completed
void ready(){
	debugf("READY!");
	// If AP is enabled:
	debugf("AP. ip: %s mac: %s", WifiAccessPoint.getIP().toString().c_str(), WifiAccessPoint.getMAC().c_str());
	//Start with fan's and led's off
	controlFan(false);
	controlLED();
}

void init(){
	// Mount file system, in order to work with files
	spiffs_mount();

	Serial.begin(SERIAL_BAUD_RATE);
	// Allow debug print to serial
	Serial.systemDebugOutput(false);

	system_update_cpu_freq(SYS_CPU_160MHZ);
	wifi_set_sleep_type(NONE_SLEEP_T);

	// Set system ready callback method
	System.onReady(ready);

	// Soft access point
	WifiAccessPoint.enable(false);

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	//Set static IP address
	WifiStation.setIP(IPAddress(192, 168, 0, 250));

	// GMT-6
	SystemClock.setTimeZone(-6);

	//Configure pins
	fanPWM.initialize();
	pinMode(FAN_1, OUTPUT);
	pinMode(FAN_2, OUTPUT);
	ReadTemp.Init(2);
	ReadTemp.StartMeasure();

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail);

	//Start lcd timer
	lcdTimer.initializeMs(5000, onLCDPrint).start();
}
