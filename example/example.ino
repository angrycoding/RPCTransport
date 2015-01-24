#include "../arduino/RPCTransport.h"
#include "../arduino/MemoryFree.cpp"


int32_t x = 0;
RPCTransport transport(Serial);

void turnOn(RPCPacket *packet) {

	packet->clear();
	packet->pushString("RESULT_STRING");
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
}


void serialEvent() {
	// transport.process();
}

bool state = true;
int32_t val = 0;
bool isOn = false;
long time = 0;

void loop() {
	// transport.process();

	// bool newState = !!digitalRead(15);
	// if (newState != state) {
	// 	state = newState;
	// 	RPCPacket(transport, "connection state is", state);
	// }

		RPCPacket(transport, "connection state is", state, val++, freeMemory());
		// delay(50);
	// if (digitalRead(15)) {
	// 	digitalWrite(14, HIGH);
	// } else {
	// 	digitalWrite(14, LOW);
	// }
	//
	// delay(1000);

	// Serial.println(digitalRead(15));
	// delay(1000);

	if (millis() - time < 500) return;
	time = millis();
	isOn = !isOn;
	digitalWrite(14, isOn ? HIGH : LOW);
}