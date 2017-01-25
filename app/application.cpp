#include <user_config.h>
#include <configuration.h>
#include <Libraries/LiquidCrystal/LiquidCrystal_I2C.h>
#include <Services/ArduinoJson/ArduinoJson.h>

LiquidCrystal_I2C lcd(I2C_LCD_ADDR, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Timer lcdTimer;
int8_t currentBrightness = 0; // 0 - 59
int8_t startFadeIn = 6; // Start fade in at 7 hrs
int8_t startFadeOut = 18; // start fade out at 18 hrs
bool automatic = true;
bool fan = false;
int activeSockets = 0;

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
	// initialize the lcd for 16 chars 2 lines, turn on backlight
	lcd.begin(16, 2);
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
	digitalWrite(FAN_1, state);
	digitalWrite(FAN_2, state);
}

//Set led's brightness
void controlLED(){
	StaticJsonBuffer<100> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	int value = ((255/59) * currentBrightness);
	root["brightness"] = value;
	String json;
	root.printTo(json);
	Serial.print(json);
}

void setCurrentBrightness(int brightness){
	currentBrightness = brightness;
	automatic = false;
	controlLED();
	controlFan(true);
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

	//Print current date - hour to LCD
	DateTime _date_time = SystemClock.now();
	lcd.setCursor(0,0);
	lcd.print(_date_time.toShortDateString() + " ");
	lcd.print(_date_time.toShortTimeString(false));
	//TODO get Temperature from probe
	if(automatic)
		onChangeLedBrightness();
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
	pinMode(FAN_1, OUTPUT);
	pinMode(FAN_2, OUTPUT);

	// Run our method when station was connected to AP (or not connected)
	WifiStation.waitConnection(connectOk, 30, connectFail);

	//Start lcd timer
	lcdTimer.initializeMs(5000, onLCDPrint).start();
}
