# Arduino-Node-RPC

Provides easy way to organize two - way communication between Arduino and Node.js:

```c++
// include library
#include "RPCTransport.h"

#define LED_PIN 8

// instantiate RPCTransport and attach it to desired Serial
RPCTransport transport(&Serial);

// used to set LED state
void setState(RPCPacket* packet) {
	// get desired state
	bool state = packet->getBool(0);
	// write LED state
	digitalWrite(LED_PIN, state);
	// clear request arguments
	packet->clear();
}

// used to get LED state
void getState(RPCPacket* packet) {
	// clear request arguments
	packet->clear();
	// read LED state
	bool state = digitalRead(LED_PIN);
	// add return value
	packet->pushBool(state);
}

// this will be called when Node.js client is connected to the board
void registerMethods() {
	transport.on("setState", setState);
	transport.on("getState", getState);
}

void setup() {
	// set the digital pin as output
	pinMode(LED_PIN, OUTPUT);
	// initialize Serial
	Serial.begin(115200);
	// initialize RPCTransport
	transport.begin(registerMethods);
}

// called if there is serial data in the buffer
void serialEvent() {
	// process incoming request
	transport.process();
}
```
