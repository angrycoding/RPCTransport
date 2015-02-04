# RPC transport

Provides easy way to organize two - way communication between Arduino and Node.js, uses the serial port as transport layer. There are plenty of libraries that provides similar functionality, so what is the reason to create another one? Firstly I've tried the library called [Johnny-Five](https://github.com/rwaldron/johnny-five), well this library itself is quiet cool, but if you need to drive your Arduino using Node.js, it's very unlikely that you'd prefer to call low-level Arduino subroutines (such as pinMode, digitalWrite and so on), unless you are building something really weird, or you don't know C++ at all, so the only way for you is to use JavaScript to program Arduino. Second library that I've tried to use in order to accomplish my project, called [CmdMessenger](http://playground.arduino.cc/Code/CmdMessenger), this one looks more like what I actually need, but I couldn't use it because it has no Node.js integration + API looks quiet complicated to me (not in terms of understanding but in terms of usage). So I've come up with the idea to create my own library, with blackjack and hookers. So here is the list of features that in my opinion had to be done in order to make this library uber cool:
* Smallest possible footprint, so library itself should not be bigger than your project, unfortunately this is not the case for both of the libraries listed above.
* All basic data types must be supported (null, boolean, floats, ints and strings). Note that arrays and objects, despite that they are widely used in JavaScript are not supported, because supporting it would either increase size of your scetch and decrease performance. Anyways if you really need arrays and objects, then you should consider using something like [aJSON](https://github.com/interactive-matter/aJson).
* Simple API, make things as intuitive and simple to use as possible.
* Two - way communication, means that Node.js can call previously registered Arduino subroutine and receive result of it's execution and all the way around, Arduino can call previously registered subroutine on Node.js side, so more JavaScript, Node.js, web 2.0, HTML5 than hardcore C++.

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
