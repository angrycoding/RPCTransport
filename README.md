# Arduino - Node.js RPC transport

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
You start by instantiating RPCTransport and attaching it to desired serial port:
```c++
// instantiate RPCTransport and attach it to desired Serial
RPCTransport transport(&Serial);
```
When you ready to receive (or send) commands you call **transport.begin** and pass optional argument - callback function where you can register functions that will be available for calling from Node.js side (but don't forget to initialize serial communication first). Register public methods by calling **transport.on**, note that this method can only be called inside of the callback that is passed into **transport.begin**, attempt to call it in any other place won't take any effect. First argument is the name of your method (will be used to call it from Node.js), second argument is callback function, that will be executed when it's called:
```c++
void canBeCalledFromNode1(RPCPacket* packet) {
	...
}

void canBeCalledFromNode2(RPCPacket* packet) {
	...
}

// this will be called when Node.js client is connected to the board
void registerMethods() {
	transport.on("command1", canBeCalledFromNode1);
	transport.on("command2", canBeCalledFromNode2);
}

void setup() {
	// don't forget to initialize Serial
	Serial.begin(115200);
	// initialize RPCTransport
	transport.begin(registerMethods);
}
```
In order to process incoming messages, you have to call **transport.process**, if you don't do that, Node.js won't be able to call Arduino methods. There are two common places where you would call it, inside of loop subroutine or inside of special [serialEvent](http://arduino.cc/en/Tutorial/SerialEvent) subroutine (preferred):
```c++
void serialEvent() {
	// process incoming messages
	transport.process();
}
```
If you do everything right, Node.js can now execute methods on Arduino side, reading passed arguments is quite straightforward:
```c++
void canBeCalledFromNode1(RPCPacket* packet) {
	// reading first argument as boolean
	bool a1 = packet->getBool(0);
	// reading second argument as float
	float a2 = packet->getFloat(1);
	// reading third argument as long
	long a3 = packet->getInt(2);
	// reading fourth argument as string
	char* a4 = packet->getString(3);
}
```
Retrieving result is a bit tricky, same pointer is used to read arguments and to forward result to the caller. Before writing result, don't forget to clear the packet, otherwise everthing that you've received as an argument will be passed into result (echo mode):
```c++
void canBeCalledFromNode1(RPCPacket* packet) {
	// read first argument as boolean
	bool value = packet->getBool(0);
	// if you don't do it, result will contain original request
	packet->clear();
	// push boolean into result set
	packet->pushBool(true);
	// push string into result set
	packet->pushString("result from Arduino");
}
```
More realistic example, let's create method that will allow Node.js to call digitalWrite on Arduino side:
```c++
// include library
#include "RPCTransport.h"

// instantiate RPCTransport and attach it to desired Serial
RPCTransport transport(&Serial);

// our digitalWrite implementation available to Node.js
void myDigitalWrite(RPCPacket* packet) {
	// read pin number
	byte pinNumber = packet->getInt(0);
	// read pin state
	bool pinState = packet->getBool(1);
	// do digitalWrite thing
	digitalWrite(pinNumber, pinState);
	// clear request arguments
	packet->clear();
	// let's respond with actual pinState
	packet->pushBool(digitalRead(pinNumber));
}

// this will be called when Node.js client is connected to the board
void registerMethods() {
	transport.on("digitalWrite", myDigitalWrite);
}

void setup() {
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
