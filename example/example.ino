#include "../arduino/RPCTransport.h"


int32_t x = 0;
RPCTransport transport(Serial);

// RPCRequest req;
// RPCValue val((bool)true);
// RPCResponse res;

void turnOn(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, HIGH);
	// response.pushInt(x++);
	// request.getValue(5);
	// qDebug() << request.getType(0);
	for (byte c = 0; c < request.length; c++)
		response.pushValue(request.getValue(c));
	response.pushValue(request.getValue(92));
}

void turnOff(RPCRequest &request, RPCResponse &response) {
	digitalWrite(14, LOW);
	// response.pushBool(true);
	// response.pushInt(x++);
	for (byte c = 0; c < request.length; c++)
		response.pushValue(request.getValue(c));
	response.pushValue(request.getValue(92));
}

// #define RPCall(...) call((RPCValue[]){__VA_ARGS__}, strlen(#__VA_ARGS__) ? RPCTransport_num_args(__VA_ARGS__) : 0)
// #define RPCTransport_num_args(...) RPCTransport_num_args_impl(__VA_ARGS__, 5,4,3,2,1)
// #define RPCTransport_num_args_impl(_1,_2,_3,_4,_5,N,...) N


void setup() {
	pinMode(14, OUTPUT);
	Serial.begin(115200);

	// RPCTransport* x = new RPCTransport(Serial);
	// RPCRequest(x, "who are you?");
	RPCRequest(transport, "what the fuck?");


	// transport.RPCall("hello", "world");
	// RPCall((int32_t)10, (float)3.14, "hello");
	// transport.on("turnOn", turnOn);
	// transport.on("turnOff", turnOff);
	// transport.begin();
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