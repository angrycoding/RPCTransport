#include "RPCTransport.h"

int32_t x = 0;
RPCTransport transport(Serial);

// RPCRequest req;
// RPCValue val((bool)true);
// RPCResponse res;

void turnOn(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, HIGH);
	response.pushInt(x++);
	// request.getValue(5);
	// qDebug() << request.getType(0);
	response.pushValue(request.getValue(5));
}

void turnOff(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, LOW);
	// response.pushBool(true);
	response.pushInt(x++);
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