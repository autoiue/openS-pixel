#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define NEO_PIN		5
#define NEO_PTYPE	NEO_GRBW	// f.e. SK6812
#define NUMPIXELS	1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_PTYPE + NEO_KHZ800);

const char* ssid = "OpenS";
#define SERVER "192.168.2.10"
#define PORT 1933
#define INTERVAL 25
#define PACKET_SIZE 4
#define TIMEOUT 5000

WiFiUDP Udp;
byte mac[6]; // mac address
bool wifiConnected = true;

void setup() {
	pinMode(2, OUTPUT);

	strip.begin();
	strip.setPixelColor(0,0);
	strip.show();
	
	delay(100);
	
	connect();
}

void connect(){
	digitalWrite(2, LOW);

	WiFi.begin(ssid);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	digitalWrite(2, HIGH);

	WiFi.macAddress(mac);
	Udp.begin(PORT);

	wifiConnected = true;
}

unsigned long lastMillis = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastPacket = 0;
bool heartbeatEnable = true;

void loop() {

	while(wifiConnected){
		// store this instant	
		unsigned long NOW = millis();

		// heartbeat if connection to server lost
		heartbeatEnable = lastPacket + 5000 < NOW;
		digitalWrite(2, NOW%1000 > 20 || !heartbeatEnable);

		// send a beacon containing the mac address every 25ms (40Hz)
		if(NOW%INTERVAL == 0 && NOW != lastMillis){
			lastMillis =  NOW;
			sendBeacon();
		}

		// check the wifi connection
		if(NOW%(INTERVAL*100) == 0 && NOW != lastWifiCheck){
			lastWifiCheck = NOW;
			wifiConnected = WiFi.status() == WL_CONNECTED;
		}

		// check if a UDP packet arrived
		listen(NOW);

	}

	Udp.stop();
	connect();

}

void sendBeacon(){
	Udp.beginPacket(SERVER, PORT);
	Udp.write((char*)mac);
	Udp.endPacket();
}

void listen(unsigned long NOW) {

	char buff[5];
	int cb = Udp.parsePacket();

	// if data available
	if (cb) {
		Udp.read(buff, PACKET_SIZE);
		// convert packet to color
		uint32_t c = buff[0] << 24 | (buff[1] & 0xFF) << 16 | (buff[2] & 0xFF) << 8 | (buff[3] & 0xFF);
		// set color
		strip.setPixelColor(0, c);
		strip.show();
		lastPacket = NOW;
	}
}



