#include "RPCTransport.h"

RPCTransport transport(Serial);

void turnOn(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, HIGH);
	response.pushBool(true);
}

void turnOff(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, LOW);
	response.pushBool(true);
}

void setup() {
	pinMode(14, OUTPUT);
	Serial.begin(115200);
	transport.on("turnOn", turnOn);
	transport.on("turnOff", turnOff);
	transport.begin();
}

void serialEvent() {
	transport.process();
}

// unsigned long time;
// bool isOn = true;


void loop() {
	// if (millis() - time < 500) return;
	// time = millis();
	// isOn = !isOn;
	// digitalWrite(14, isOn ? HIGH : LOW);
}