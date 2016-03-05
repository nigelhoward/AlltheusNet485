# AlltheusNet485
Arduino and Photon library for inter IOT Device coms using an adapted version of RS485

Allthough tested and functional the documentation needs updating and I am in the process of adding ring buffers to the AllNet485 methods of the RS485NB class. Please PM me for more details. 

RS485 protocol library - Particle Photon / Arduino Version

Based on above RS485 Non blocking by Nick Gammon. Thanks Nick :-) I could not have written this without your excellent code and forum. Please note I am not a C++ programmer. Pointers drive me nuts!

Added more error reporting, debug serial output and strange extra byte read after ETX to stop nibble errors.
Added RTS/CTS 'type' of control with an additional wire on pin hardware pulled to +5 which I've called AllNet485 to allow all boards to share the bus as 'masters'

In my setup running a medium size bus, 250000 Baud, about 10 boards and sending 20 messages per second produced less than 0.1% errors. See how the network is performing by checking the errorCounters and keeping track of the last sequence number for each board in an array. Comparing sequence numbers they always increment by each time a message is sent. So if the last message sequence number +1 doesn't equal this sequence number then you lost a message. Too bad, slow down a bit!

Hardware serial performed much better than software serial for me. 250000 Baud was about the max for the serial port. If you build a test receiver to analyze above then make sure it doesn't do too much Serial.Print to a slow serial port. This will skip messages and create sequence errors.

More later....
