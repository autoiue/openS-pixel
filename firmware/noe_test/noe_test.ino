#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define NEO_PIN		5  // NeoPixel DATA
// What type of NeoPixel strip is attached to the Arduino?
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_GRBW    Pixels are wired for GRBW bitstream (f.e. SK6812)
#define NEO_PTYPE	NEO_GRBW	// f.e. SK6812
#define NUMPIXELS	1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, NEO_PIN, NEO_PTYPE + NEO_KHZ800);

const char* ssid = "OpenS";
#define SERVER "192.168.2.10"
#define PORT 1933
#define INTERVAL 25
#define PACKET_SIZE 5
#define TIMEOUT 5000

WiFiUDP Udp;
byte mac[6]; // mac address

void setup() {
	pinMode(2, OUTPUT);

	strip.begin();
	strip.show();

	delay(100);

	WiFi.begin(ssid);

	strip.setPixelColor(0, 0x00080000);
	strip.show();

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
	}

	WiFi.macAddress(mac);
	Udp.begin(PORT);

	strip.setPixelColor(0, 0);
	strip.show();
}

unsigned long lastMillis = 0;
unsigned long lastPacket = 0;
bool heartbeatEnable = true;

void loop() {
	unsigned long NOW = millis();

	digitalWrite(2, NOW%1000 > 20 || !heartbeatEnable);

	if(NOW%INTERVAL == 0 && NOW != lastMillis){
		lastMillis =  NOW;
		sendBeacon();
	}

	listen(NOW);

	heartbeatEnable = lastPacket + 5000 < NOW;
}

void sendBeacon(){
	Udp.beginPacket(SERVER, PORT);
	Udp.write((char*)mac);
	Udp.endPacket();
}

void listen(unsigned long NOW) {
	char buff[5];
	int cb = Udp.parsePacket();
	if (cb) {
		Udp.read(buff, PACKET_SIZE);
		uint32_t c = buff[0] << 24 | (buff[1] & 0xFF) << 16 | (buff[2] & 0xFF) << 8 | (buff[3] & 0xFF);
		heartbeatEnable = buff[4] > 127;
		strip.setPixelColor(0, c);
		strip.show();
		lastPacket = NOW;
	}
}



