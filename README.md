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

Include the library and instantiate RPCTransport with appropriate Serial:

```c++
// include library
#include "RPCTransport.h"
// instantiate RPCTransport and attach it to desired Serial
RPCTransport transport(&Serial);
```

Register methods that you want to make public (can be called by Node.js). There are couple of problems that I've been trying to solve here. First one is: what should be used as keys when registering those methods? Using numbers as keys works faster and uses less memory, but it's less convinient than string keys. Using strings solves the problem but it needs more RAM, access time is also slower. Second problem is that usually when you are connecting to Arduino over serial port, it's going to be automatically restarted, so anything that you sent to it after connection and before it's started - will be lost, and the only way to deal with it, is hardware modifications (not that difficult but you would do that only when you finished your project). To solve first problem, library transparently stores callback map on Node.js side, this way we don't need to waist Arduino RAM and in the same time provide fast lookups. Second problem is solved by providing special "internal" method, that will be called once Arduino is ready for communication, letting Node.js know that everything is set up, and no packets will be lost due the autorestart. At the end, all you need to do is to call RPCTransport::begin when you ready to communicate with the other side, passing it method that is registering all you callbacks:

```c++
void registerMethods() {
	transport.on("setState", setState);
	transport.on("getState", getState);
	...
}

void setup() {
	// initialize Serial
	Serial.begin(115200);
	// initialize RPCTransport
	transport.begin(registerMethods);
}
```
