/*
 RS485 protocol library - Particle Photo Version
 Based on above RS485 Non blocking by Nick Gammon
 Thanks Nick :-) I could not have written this without your excellent code and forum
 Please note I am not a C++ programmer. Pointers drive me nuts!

 Added more error reporting, debug serial output and strange extra byte read after ETX to stop nibble errors.
 Added RTS Control and AllNet485 stuff to allow all boards to share the bus as masters

 Callbacks used by this to be set in application code.
 For using the TX,RX Pins on the board with most RS485 boards.
 Tested with Sparkfun RS485 that uses the SP3485 transceiver chip.


 */

// Version P2.4 - By TopBanana 23-03-2016

#include "RS485NB.h"

// allocate the requested buffer size
void RS485::begin (byte boardId)
{
	myId = boardId;
	data_ = (byte *) malloc (bufferSize_);
	reset ();
	errorCount_ = 0;
	pinMode(rtsPin, OUTPUT);
	if(messageNotSentLED!=255) pinMode(messageNotSentLED, OUTPUT);
	if(errorEventLED!=255) pinMode(errorEventLED, OUTPUT);

} // end of RS485::begin

// get rid of the buffer
void RS485::stop ()
{
	reset ();
	free (data_);
	data_ = NULL;
} // end of RS485::stop

// called after an error to return to "not in a packet"
void RS485::reset ()
{
haveSTX_ = false;
available_ = false;
inputPos_ = 0;
startTime_ = 0;
boardCastMessage = false;
} // end of RS485::reset

// calculate 8-bit CRC
byte RS485::crc8 (const byte *addr, byte len)
{
  byte crc = 0;
  while (len--)
    {
    byte inbyte = *addr++;
    for (byte i = 8; i; i--)
      {
      byte mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
      }  // end of for
    }  // end of while
  return crc;
}  // end of RS485::crc8

// send a byte complemented, repeated
// only values sent would be (in hex):
//   0F, 1E, 2D, 3C, 4B, 5A, 69, 78, 87, 96, A5, B4, C3, D2, E1, F0
void RS485::sendComplemented (const byte what)
{
byte c;

  // first nibble
  c = what >> 4;
  fWriteCallback_ ((c << 4) | (c ^ 0x0F));

  // second nibble
  c = what & 0x0F;
  fWriteCallback_ ((c << 4) | (c ^ 0x0F));
}  // end of RS485::sendComplemented

// send a message of "length" bytes (max 255) to other end
// put STX at start, ETX at end, and add CRC
bool RS485::sendMsg (const byte * data, const byte length, const byte receiverId, const byte messageType, const bool messageRequiresConfirmation)
{
	// no callback? Can't send
	if (fWriteCallback_ == NULL)
	return false;

  if(allNet485Enabled)
  {
    bool busWasStillBusy = true;
    for(int i = 0 ; i < busBusyRetryCount; i++)
    {
	    randomRetryMicrosDelay();
		if(!busIsBusy())
		{
			busWasStillBusy = false;
			i=busBusyRetryCount + 1;
		}
    }
    if(busWasStillBusy)
    {
		if (debug) Serial.println("busStillBusy");
		return false; // While loop finished after all retries
    }
  }

  if(allNet485Enabled) busMakeBusy();
  digitalWrite(rtsPin,HIGH);

  fWriteCallback_ (STX);  // STX

  // Build an entire message buffer to use in send. Bit of a waste of bytes :-)
  // But a quick and easy adaption of existing code so CRC still works
  byte messageData[MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE];

  // Header
  messageData[0]=messageType;
  messageData[1]=receiverId;
  messageData[2]=myId;

  sequenceNumber++;
  messageData[3]=(sequenceNumber >> 24) & 0xFF;
  messageData[4]=(sequenceNumber >> 16) & 0xFF;
  messageData[5]=(sequenceNumber >> 8) & 0xFF;
  messageData[6]= sequenceNumber & 0xFF;

  messageData[7]= messageRequiresConfirmation;

  // add given Data after the header
  for (byte i = 0; i < length; i++)
  {
	messageData[i + MESSAGE_HEADER_SIZE] = data [i];
  }

  // Send the data
  for (byte i = 0; i < (length + MESSAGE_HEADER_SIZE); i++)
  {
	  sendComplemented(messageData[i]);
  }
  
  // End of transmission byte
  fWriteCallback_ (ETX);  // ETX

  // CRC Byte
  sendComplemented (crc8 (messageData, length + MESSAGE_HEADER_SIZE ));
  fWaitCallback_(); // Waits for hardware serial sends to complete
	
  messageWasSent(); // Note that this is before the bus was made idle!
	
  digitalWrite(rtsPin,LOW);
  
  
  if(allNet485Enabled)busMakeIdle();

  return true;

}  // end of RS485::sendMsg

// called periodically from main loop to process data and
// assemble the finished packet in 'data_'
// returns true if packet received.
bool RS485::update ()
  {

  while (fAvailableCallback_ () > 0)
    {
    byte inByte = fReadCallback_ ();
    if(debug) Serial.print("*");

    switch (inByte)
      {
        case STX:   // start of text is here
          if(debug) Serial.print("STX:");
          haveSTX_ = true;
          haveETX_ = false;
          inputPos_ = 0;
          firstNibble_ = true;
          startTime_ = millis ();
          boardCastMessage = false;
          break;

        case ETX:   // end of text (now expect the CRC check)
          if(debug) Serial.print(":ETX");
		      haveETX_ = true;
          break;

        default:
          // wait until packet officially starts
          if (!haveSTX_)
            break;

          // check byte is in valid form (4 bits followed by 4 bits complemented)
          if ((inByte >> 4) != ((inByte & 0x0F) ^ 0x0F) )
            {
				errorHandler(ERROR_NIBBLE);
   				reset ();
   				break;  // bad character
            } // end if bad byte

          // convert back
          inByte >>= 4;

          // high-order nibble?
          if (firstNibble_)
            {
				currentByte_ = inByte;
				firstNibble_ = false;
				break;
            }  // end of first nibble

          // low-order nibble
          currentByte_ <<= 4;
          currentByte_ |= inByte;
          firstNibble_ = true;

          // if we have the ETX this must be the CRC
          if (haveETX_)
            {
				if(debug) Serial.print(":CRC");
				if (crc8 (data_, inputPos_) != currentByte_)
				{
					reset ();
					errorHandler(ERROR_CRC);
					break;  // bad crc
				} // end of bad CRC

				// Strangely (my lack of understanding :-) there is a spare byte on the end that causes a nibble error.
				// Comment this line out if you are getting lots of errors.
				// Required for Particle.io Photon
				byte spareByte = fReadCallback_ (); // Extra byte read

				// Set properties
				messageType = data_[0]; // Type of message
				messageReceiverId = data_[1]; // Who the message is for
				messageSenderId = data_[2]; // Who sent the message
				messageSequenceNumber = (data_[03] << 24) | (data_[04] << 16) | (data_[05] << 8) | (data_[06]); // sequence number of this received message
				messageRequiresConfirmation = data_[7];

				// Create a new AllMessage for the inQueue
				AllMessage newAllMessage;
				newAllMessage.Type = messageType;
				newAllMessage.ReceiverId = messageReceiverId;
				newAllMessage.SenderId = messageSenderId;
				newAllMessage.Id = messageSequenceNumber;
				newAllMessage.RequiresConfirmation = messageRequiresConfirmation;

				// Keep the time it arrived albeit for 0 to 64 seconds because this is a u int and not a u long
				// Good enough for what we need without adding extra memory to the queues 
				newAllMessage.WhenReceived = (int)millis();

				// Copy message data to the AllMessage.Data property
				for (int i = 0; i < MESSAGE_DATA_SIZE ; i++) // inputPos_ held last byte count
				{
					newAllMessage.Data[i] = data_[i + MESSAGE_HEADER_SIZE];
				}
			
				// Push the message on the queue
				if(!inQueue.enqueue(newAllMessage)) errorHandler(ERROR_INQUEUEOVERFLOW);
				
				messageWasReceived();

				// Send confirmation if required
				if(messageRequiresConfirmation) sendConfirmation(newAllMessage);
			
				available_ = true;

				return true;  // show data ready
				}  // end if have ETX already

				// keep adding if not full
				if (inputPos_ < bufferSize_)
				{
				// Is it a boardCast? - messageType=0x00
				if(inputPos_ == 0 && currentByte_ == MESSAGE_BOARDCAST)
				{
					boardCastMessage = true;
				}

				if(ignoreBoardcasts==true) boardCastMessage = false; // Even if it is a boardcast say it isn't so it gets ignored

				if(!boardCastMessage)
				{
					// Check if it's for me and not a boardCast :-) - > Second byte is the receiver Id in the message
					if(inputPos_ == 1 && onlyReadMyMessages == true )
					{
  						if(currentByte_ != myId)
  						{
  							reset();
  							return false;
  						}
					}
				}
				// Add the data to the data_ array
				data_ [inputPos_++] = currentByte_;
				if(debug)
				{
					Serial.print(" ");
					Serial.print(currentByte_,HEX);
				}
				}
				else
				{
				reset (); // overflow, start again

				//Serial.println("Overflow Error");
				errorHandler(ERROR_BUFFEROVERFLOW);
				}

				break;

      }  // end of switch
    }  // end of while incoming data

  return false;  // not ready yet

  } // end of RS485::update

  // AllNet485 Extension uses an additional wire to stop other boards from transmitting. This reduces the speed capabilities of RS485
  // But adds the ability to have multiple masters that can all talk on the same bus
  // Primarily for home automation that doesn't require a fast bus but does require reliability
  // AllNet485 Methods & properties

  void RS485::allNetUpdate()
  {
	  // Do a read / update
	    RS485::update();
	  
	  // Send any messages that are in the OutQueue
	  if(outQueue.count()>0)
	  {
		AllMessage allMessage = outQueue.dequeue();
		if(RS485::sendMsg (allMessage.Data, MESSAGE_DATA_SIZE , allMessage.ReceiverId,allMessage.Type,allMessage.RequiresConfirmation))
		{
			// It worked
			if(messageNotSentLED!=255) digitalWrite(messageNotSentLED, LOW);	    
			RS485::update();
		}
		else
		{
			// Did not work - Put message back in the OutQueue
			if(messageNotSentLED!=255) digitalWrite(messageNotSentLED, HIGH);
			OutQueueEnqueue(allMessage);
		    RS485::update();
		}
	  }
	  
	  calculateBusSpeed();
  	  errorLEDHandler(LOW); // Reset the error LED if appropriate

  }

  AllMessage RS485::InQueueDequeue()
  {
	return inQueue.dequeue();
  }
  void RS485::OutQueueEnqueue(AllMessage allMessage)
  {
	  if(!outQueue.enqueue(allMessage)) errorHandler(ERROR_OUTQUEUEOVERFLOW);
  }

  void RS485::allNet485Enable (byte busyPin)
  {
    busBusyPin = busyPin;
    allNet485Enabled = true;
    pinMode(busBusyPin, INPUT);

	// Initialize the queues
	initMessageQueues(inQueueSize, outQueueSize, confQueueSize);
  }
  void RS485::allNet485Disable ()
  {
    busBusyPin = 255;
    allNet485Enabled = false;
  }

  void RS485::busMakeBusy()
  {
    pinMode(busBusyPin, OUTPUT);
    digitalWrite(busBusyPin, 0);
    delay(busBusyDelayBeforeTransmit);

  }
  void RS485::busMakeIdle()
  {
    delay(busBusyDelayAfterTransmit);
    pinMode(busBusyPin, OUTPUT);
    digitalWrite(busBusyPin, 1);
    pinMode(busBusyPin, INPUT);

  }
  bool RS485::busIsBusy()
  {
    if (digitalRead(busBusyPin) == 0) // Bus is busy
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  // Does a delay of approx number of microseconds whilst still reading the bus / serial port
  void RS485::busDelayMicros(unsigned long microsDelayRequired)
  {
	unsigned long microsStart = micros();

	while (micros() < (microsStart + microsDelayRequired))
	{
		// Check if overflow (back to wero for micros() occured during this method
		if (microsStart > micros()) microsStart = micros();  // Re-assign it so we don't get a super huge delay
		if(allNet485Enabled) update(); // Read any data that's on it's way in if AllNet is enabled
	}
  }
  // As above but with milliseconds
  void RS485::busDelayMillis(int millisDelayRequired)
  {
	  unsigned long microsDelayRequired = 1000 * (unsigned long)millisDelayRequired;
	  busDelayMicros(microsDelayRequired);	  
  }
  
  int RS485::getBusSpeed()
  {
	 return busSpeed; 
  }
  // Does a quick bus speed calculation for the last 2 seconds 
  void RS485::calculateBusSpeed()
  {
	  if(millis() > busSpeedLastMillis + 2000)
	  {
		  Serial.print("busSpeedLastMessageReceivedCount");
		  Serial.print(busSpeedLastMessageReceivedCount);
		  Serial.print(" messRcv cnt:");
		  Serial.println(messagesReceivedCounter);
		  
		  busSpeedLastMillis = millis();
		  busSpeed = (messagesReceivedCounter - busSpeedLastMessageReceivedCount) / 2;	  
		  busSpeedLastMessageReceivedCount = messagesReceivedCounter;
	  }
  }

  // Does a random microsecond delay
  void RS485::randomRetryMicrosDelay()
  {
    unsigned long randomMicrosDelay = random(280,10000) + (50 * myId);
	busDelayMicros(randomMicrosDelay);
  }
  // Stuff that needs doing while a delay is delaying
  void RS485::doDelayStuff()
  {
	  update();
  }

  bool RS485::sendConfirmation(AllMessage allMessage)
  {	  
	
	//allMessage.SenderId == 0x11; // Basil
	//byte confirmationMessage[] ="MESSAGE_CONFIRMATION      ";
	////sendMsg(confirmationMessage,sizeof(confirmationMessage),allMessage.SenderId,CONFIRMATION,false);
	//this->busBusyRetryCount = 100;
	//sendMsg(confirmationMessage,MESSAGE_DATA_SIZE,0x11,MESSAGE,false);
	//this->busBusyRetryCount = 2;
	
	//Serial.print ("CONF to:");
	//Serial.println(allMessage.SenderId,HEX);
  }

  // Message Queue setup
  void RS485::initMessageQueues(int inSize, int outSize, int conSize)
  {
	  inQueue.init(inSize); // Incoming messages
	  outQueue.init(outSize); // Outgoing messages
	  confQueue.init(conSize); // Messages that need a confirmation
  }
  
  // Stuff to do when a message was sent
  void RS485::messageWasSent()
  {
	  messagesSentCounter++;
  } 
  // Stuff to do when a message was received
  void RS485::messageWasReceived()
  {	  
	  messagesReceivedCounter++;
  }
  
  // Error handler
  void RS485::errorHandler(int errorType)
  { 
	bool tempSerialOut = false;
	errorCount_++;
	errorLEDHandler(HIGH);
	Serial.print(getErrorCount()); 		  
	errorLastMillis_ = millis(); // Just do this after above or errorLedHandler won't turn LED off when called
	switch (errorType)
	{

		case ERROR_BUFFEROVERFLOW:
			errorCountOverflow_++;				
			if(tempSerialOut) Serial.println("ERROR_BUFFEROVERFLOW");
		break;
		  
		case ERROR_CRC:
			errorCountCRC_++;
			if(tempSerialOut) Serial.println("ERROR_CRC");
		break;
		  
		case ERROR_NIBBLE:
			errorCountNibble_++;				
			if(tempSerialOut) Serial.println("ERROR_NIBBLE");
		break;
		  
		case ERROR_INQUEUEOVERFLOW:
			errorCountInQueueOverflow_++;
			if(tempSerialOut) Serial.println("ERROR_INQUEUEOVERFLOW");
		break;
		  
		case ERROR_OUTQUEUEOVERFLOW:
			errorCountOutQueueOverflow_++;
			if(tempSerialOut) Serial.println("ERROR_OUTQUEUEOVERFLOW");
		break;
		  
		case ERROR_CONFQUEUEOVERFLOW:
			errorCountConfQueueOverflow_++;
			if(tempSerialOut) Serial.println("ERROR_CONFQUEUEOVERFLOW");
		break;
		  
		default:
		break;
	}	
  }
// Error LED handler
void RS485::errorLEDHandler(bool state)
{
	if(errorEventLED==255) return ; // Don't do anything if the pin isn't set
	
	if(millis() > (errorLastMillis_ + 100) && state == LOW) // Asked to switch it off and 100ms has passed
	{
		digitalWrite(errorEventLED,LOW);
		return;		
	}

	if(state==HIGH)
	{
		digitalWrite(errorEventLED,HIGH);
	}
}

