#include "../arduino/RPCTransport.h"

RPCTransport transport(&Serial);

void changeState(RPCPacket* packet) {
	digitalWrite(14, packet->getBool(2));
	packet->clear();
	packet->pushBool(digitalRead(14));
}

void registerMethods() {
	transport.on("changeState", changeState);
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

	RPCPacket(transport, "HELLO", valx++, state = !state);

	// bool newState = digitalRead(15);

	// if (state != newState) {
	// 	state = newState;
	// 	RPCWriter(rpcReader, "changed", newState);
	// }

	// RPCSend(transport, valx++);


	// if (millis() - time < 500) return;
	// time = millis();
	// isOn = !isOn;
	// digitalWrite(14, isOn ? HIGH : LOW);
}