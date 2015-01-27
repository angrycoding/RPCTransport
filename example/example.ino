#include "../arduino/RPCTransport.h"


RPCTransport transport(&Serial);

void changeState(RPCPacket* stream) {

	// packet->clear();
	// packet->pushString("RESULT_STRING");
	// digitalWrite(14, HIGH);
	// response.pushInt(x++);
	// request.getValue(5);
	// qDebug() << request.getType(0);
	// for (byte c = 0; c < request.length; c++)
		// response.pushValue(request.getValue(c));
	// response.pushValue(request.getValue(92));
}

void setup() {
	pinMode(14, OUTPUT);
	digitalWrite(15, HIGH);
	Serial.begin(115200);
	transport.on("changeState", changeState);
	transport.on("changeState2", changeState);
}


// void serialEvent() { rpcReader.process(); }

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