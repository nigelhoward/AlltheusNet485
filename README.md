# AlltheusNet485
Arduino and Photon library for inter IOT Device coms using an adapted version of RS485

Allthough tested and functional the documentation needs updating and I am in the process of adding ring buffers to the AllNet485 methods of the RS485NB class. Please PM me for more details. 

RS485 protocol library - Particle Photon / Arduino Version

Based on above RS485 Non blocking by Nick Gammon. Thanks Nick :-) I could not have written this without his excellent code and forum. Please note I am not a C++ programmer. Pointers drive me nuts!

To Nick's class I added more error reporting, debug serial output and strange extra byte read after ETX to stop nibble errors.
Added RTS/CTS 'type' of control with an additional wire on pin hardware pulled to +5 which I've called AllNet485 to allow all boards to share the bus as 'masters' - BusBusy

In my setup running a medium size bus, 200000 Baud, about 10 boards and sending 20 messages per second produced less than 0.1% errors. See how the network is performing by checking the errorCounters and keeping track of the last sequence number for each board in an array. Comparing sequence numbers they always increment by each time a message is sent. So if the last message sequence number +1 doesn't equal this sequence number then you lost a message. Too bad, slow down a bit!

Hardware serial performed much better than software serial for me. 250000 Baud was about the max for the serial port. If you build a test receiver to analyze above then make sure it doesn't do too much Serial.Print to a slow serial port. This will skip messages and create sequence errors.

AllNet485 Extension uses an additional wire to stop other boards from transmitting. This reduces the speed capabilities of RS485
But adds the ability to have multiple masters that can all talk on the same bus
Primarily for home automation that doesn't require a fast bus but does require reliability

###Notes on callbacks
Callbacks handle the serial IO and are passed as function pointers in the instantiation of the RS485 Channel at the begining of the sketck. Different board types need different callback functions. The photon is very different from the Arduino Mega. 

For the Photon I am using the gardware serial1 for RS485 RX/TX
```CPP
// Callbacks for Photon
 size_t fWrite(const byte what) {return Serial1.write(what);}
 int fAvailable(){return Serial1.available();}
 int fRead(){return Serial1.read();} // See above
 void fWait() {while( !Serial1TXcomplete()){}} // See above
 int Serial1TXcomplete(void)
 {
	if(USART_GetFlagStatus(USART1, USART_FLAG_TC) != RESET)
	return 1; // Complete
	else
	return 0; // Not Complete
 }
 // End callbacks
```

For Arduino serial callbacks use this but change the port numbers. E.G UCSR1A would be UCSR2A for the second serial2 on an Arduino Mega. 
```CPP
// Callbacks for Arduino - This is a Mega2560 with RS485 on TX/RX1
size_t fWrite(const byte what) {return Serial1.write(what);}
void fWait()
{
	while (!(UCSR1A & (1 << UDRE1)))	// Wait for empty transmit buffer
	UCSR1A |= 1 << TXC1;				// Mark transmission not complete
	while (!(UCSR1A & (1 << TXC1)));   // Wait for the transmission to complete
}
int fAvailable(){ return Serial1.available(); }
int fRead(){ return Serial1.read(); }
// End callbacks
```
Credit again to Nick Gammon - http://www.gammon.com.au/forum/?id=11428 for these


More later ....
