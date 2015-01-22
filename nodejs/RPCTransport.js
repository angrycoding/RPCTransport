var SerialPort = require('serialport').SerialPort;

var serialPort = new SerialPort.SerialPort('/dev/cu.usbserial-A965DFR7', {
	baudrate: 115200,
	// parser: SerialPort.parsers.readline("\n")
}, false);
