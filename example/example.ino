#include "../arduino/RPCTransport.h"

RPCTransport transport(&Serial);

void toggleLed(RPCRequest* packet) {
	bool state = digitalRead(14);
	digitalWrite(14, state = !state);
	packet->clear();
	packet->pushBool(state);
}

void changeLed(RPCRequest* packet) {
	digitalWrite(14, packet->getBool(0));
	packet->clear();
	packet->pushBool(digitalRead(14));
}

void registerMethods() {
	transport.on("toggleLed", toggleLed);
	transport.on("changeLed", changeLed);
}

void setup() {
	pinMode(14, OUTPUT);
	digitalWrite(15, HIGH);
	Serial.begin(115200);
	transport.begin(registerMethods);
}


void serialEvent() { transport.process(); }

bool state = true;
int32_t valx = 0;
// int32_t val = 0;
// bool isOn = false;
// long time = 0;

void loop() {


	bool newState = digitalRead(15);
	if (state != newState) {
		state = newState;
		RPCNotify(transport, "changed", newState);
	}

	// RPCSend(transport, valx++);


	// if (millis() - time < 500) return;
	// time = millis();
	// isOn = !isOn;
	// digitalWrite(14, isOn ? HIGH : LOW);
}