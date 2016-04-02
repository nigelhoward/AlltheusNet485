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

 // Version P3.2 - By TopBanana 27-03-2016

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
	bootLightShow();
	bootBusWaiter();

} // end of RS485::begin

// When a board starts it waits until it has seen 10 busBusy cycles or some milliseconds before starting
void RS485::bootBusWaiter()
{
	unsigned long now = millis();
	int busIsBusyCounter = 0;
	
	while(busIsBusyCounter < 10)
	{
		if(busIsBusy())
		{
			busIsBusyCounter++;
		}		
		// Or break out if timer lapsed
		if(millis()> now + 200) break;
	}	
}
// Flashes the LEDS that have valid pin numbers
void RS485::bootLightShow()
{	
	for (int i=0; i < 10 ;i++)
	{
		if(messageNotSentLED!=255) digitalWrite(messageNotSentLED,!digitalRead(messageNotSentLED));
		if(errorEventLED!=255) digitalWrite(errorEventLED,!digitalRead(errorEventLED));
		delay(200);
	}
	if(messageNotSentLED!=255) digitalWrite(messageNotSentLED,LOW);
	if(errorEventLED!=255) digitalWrite(errorEventLED,LOW);
}
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
bool RS485::sendMsg (const byte * data, const byte length, const byte receiverId, const byte messageType, const byte messageRequiresConfirmation, unsigned long givenMessageId)
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
	byte messagePacket[MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE];
	
	// And an AllMessage
	AllMessage allMessage;
	
	// Header
	messagePacket[0]=messageType;
	messagePacket[1]=receiverId;
	messagePacket[2]=myId;
	
	allMessage.Type = messageType;
	allMessage.ReceiverId = receiverId;
	allMessage.SenderId=  myId;

	unsigned long sendMessageId = 0;
	
	// Only generate new Id / sequence number if this is not a confirmation
	if(messageType != MESSAGE_CONFIRMATION)
	{
		sequenceNumber++;
		sendMessageId = sequenceNumber;
	}
	else
	{
		sendMessageId = givenMessageId;
	}
 	
	allMessage.Id = sendMessageId;
	
	messagePacket[3]=(sendMessageId >> 24) & 0xFF;
	messagePacket[4]=(sendMessageId >> 16) & 0xFF;
	messagePacket[5]=(sendMessageId >> 8) & 0xFF;
	messagePacket[6]= sendMessageId & 0xFF;
  
	messagePacket[7]= messageRequiresConfirmation;
	allMessage.RequiresConfirmation = messageRequiresConfirmation;
	
	// add given Data after the header
	for (byte i = 0; i < length; i++)
	{
		messagePacket[i + MESSAGE_HEADER_SIZE] = data [i];
		allMessage.Data[i] = data[i];
	}

	// Send the data
	for (byte i = 0; i < (length + MESSAGE_HEADER_SIZE); i++)
	{
		sendComplemented(messagePacket[i]);
	}
  
	// End of transmission byte
	fWriteCallback_ (ETX);  // ETX

	// CRC Byte
	sendComplemented (crc8 (messagePacket, length + MESSAGE_HEADER_SIZE ));
	fWaitCallback_(); // Waits for hardware serial sends to complete
	
	messageWasSent(); // Note that this is before the bus was made idle!
	// Does it require a confirmation?
	if(allMessage.RequiresConfirmation )
	{
		confirmationIsRequired(allMessage);
	}
	
	digitalWrite(rtsPin,LOW); 
  
	if(allNet485Enabled)busMakeIdle();

	return true;

}  // end of RS485::sendMsg

// called periodically from main loop to process data and
// assemble the finished packet in 'data_'
// returns true if packet received.
bool RS485::updateReceive()
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

				// Strangely (and probably my lack of understanding :-) there is a spare byte on the end that causes a nibble error.
				// Comment this line out if you are getting lots of errors.
				// Required for Particle.io Photon & Arduino
				
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

				// Keep the time it arrived
				newAllMessage.WhenReceived = millis();

				// Copy message data to the AllMessage.Data property
				for (int i = 0; i < MESSAGE_DATA_SIZE ; i++) // inputPos_ held last byte count
				{
					newAllMessage.Data[i] = data_[i + MESSAGE_HEADER_SIZE];
				}
							
				// Is this a confirmation of a sent message?
				if(newAllMessage.Type == MESSAGE_CONFIRMATION)
				{	
					// Treat it as a confirmation and process it
					confirmationWasReceived(newAllMessage);
				}

				// Was a confirmation requested?
				if(messageRequiresConfirmation) confirmationWasRequested(newAllMessage);					

				// Push the message on the queue and make it available
				if(!inQueue.enqueue(newAllMessage)) errorHandler(ERROR_INQUEUEOVERFLOW);	
				
				messageWasReceived();

				available_ = true;

				return true;  // show data ready
				}  // end if have ETX already

				// Do any filtering
				if (filterOutThisMessage(currentByte_, inputPos_ ) == true)
				{
					reset();
					return false;
				}

				// keep adding if the buffer is not full
				if (inputPos_ < bufferSize_)
				{
					// Add the data to the data_ array
					data_ [inputPos_++] = currentByte_;
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
	    RS485::updateReceive();
	  
	  // Send any messages that are in the OutQueue
	  if(outQueue.count()>0)
	  {
		AllMessage allMessage = outQueue.dequeue();
		if(RS485::sendMsg (allMessage.Data, MESSAGE_DATA_SIZE , allMessage.ReceiverId,allMessage.Type,allMessage.RequiresConfirmation,allMessage.Id))
		{
			// It worked
			if(messageNotSentLED!=255) digitalWrite(messageNotSentLED, LOW);	    
			RS485::updateReceive();
		}
		else
		{
			// Did not work - Put message back in the OutQueue
			if(messageNotSentLED!=255) digitalWrite(messageNotSentLED, HIGH);
			OutQueueEnqueue(allMessage);
		    RS485::updateReceive();
		}
	  }
	  confQueueHandler();
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
		if(allNet485Enabled) updateReceive(); // Read any data that's on it's way in if AllNet is enabled
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
	  updateReceive();
  }
  // Message Queue setup
  void RS485::initMessageQueues(int inSize, int outSize, int conSize)
  {
	  inQueue.init(inSize); // Incoming messages
	  outQueue.init(outSize); // Outgoing messages
	  confQueue.init(conSize); // Messages that need a confirmation
  }
  
  // Stuff to do when a message was sent by sendMessage
  void RS485::messageWasSent()
  {
	  messagesSentCounter++;
  } 
  // Stuff to do when a message was received by update (receive message)
  void RS485::messageWasReceived()
  {	  
	  messagesReceivedCounter++;
  }
  
  // Error handler
  void RS485::errorHandler(int errorType)
  { 
	errorCount_++;
	errorLEDHandler(HIGH); 		  
	errorLastMillis_ = millis(); // Just do this after above or errorLedHandler won't turn LED off when called
	
	if(debugErrorsToSerial)
	{
		Serial.println();
		Serial.print(getErrorCount());
		Serial.print(":");		
	}
	switch (errorType)
	{
		case ERROR_BUFFEROVERFLOW:
			errorCountOverflow_++;				
			if(debugErrorsToSerial) Serial.print("ERROR_BUFFEROVERFLOW");
		break;
		  
		case ERROR_CRC:
			errorCountCRC_++;
			if(debugErrorsToSerial) Serial.print("ERROR_CRC");
		break;
		  
		case ERROR_NIBBLE:
			errorCountNibble_++;				
			if(debugErrorsToSerial) Serial.print("ERROR_NIBBLE");
		break;
		  
		case ERROR_INQUEUEOVERFLOW:
			errorCountInQueueOverflow_++;
			if(debugErrorsToSerial) Serial.print("ERROR_INQUEUEOVERFLOW");
		break;
		  
		case ERROR_OUTQUEUEOVERFLOW:
			errorCountOutQueueOverflow_++;
			if(debugErrorsToSerial) Serial.print("ERROR_OUTQUEUEOVERFLOW");
		break;
		  
		case ERROR_CONFQUEUEOVERFLOW:
			errorCountConfQueueOverflow_++;
			if(debugErrorsToSerial) Serial.print("ERROR_CONFQUEUEOVERFLOW");
		break;
		
		case ERROR_CONFRECEIPTTIMEOUT:
			errorCountConfQueueOverflow_++;
			if(debugErrorsToSerial) Serial.print("ERROR_CONFRECEIPTTIMEOUT");
		break;
		
		case ERROR_CONFMESSAGENOTFOUND:
			if(debugErrorsToSerial) Serial.print("ERROR_CONFMESSAGENOTFOUND");
		break;
		
		default:
		break;
		if(debugErrorsToSerial)
		{
			Serial.println();
		}
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

// Confirmation methods

// Scans the conf queue and re-sends or deletes messages
void RS485::confQueueHandler() 
{
	// Get a message from the confirmation queue to look at
	
	if(confQueue.count() <1 )
	{
		return;
	}
		 
	// Get a message from the confirmation queue	
	AllMessage allMessage = confQueue.dequeue();
	
	unsigned long now = millis();
	
	// Has it been hanging around long enough to be re-sent and re-quered in confirmation queue?
	if((now - allMessage.WhenReceived) > confQueueTimeoutDelay)
	{			
		
		// Not happy with the logic here so limiting to 1 to override any set in code
		// Problem is that confirmations sent with a message Id that has been incremented are not then found when
		// a confirmation is actually received. So this code just fires off another allMessage.RequiresConfirmation -- times sending the message again regardless of the received
		// response which has a Id that can't be found in this confirmation queue.
		// A more elegant way is required here but one/two retries until timeout is fine given that this is a very reliable means of
		// communicating between IOT things. 
		
		if(allMessage.RequiresConfirmation>1) {allMessage.RequiresConfirmation = 2;}	
		if(allMessage.RequiresConfirmation > 0) // Not last try
		{
			// Decrement it's retry counter
			allMessage.RequiresConfirmation --;
		
			// Queue it for sending again
			OutQueueEnqueue(allMessage);
			
			// Put it back in conf queue
			allMessage.WhenReceived = millis();
			confQueue.enqueue(allMessage);
		
			return;
		}

		// Timeout for the last retry time - Error!
		errorHandler(ERROR_CONFRECEIPTTIMEOUT);
		return;
	}
	// Re-queue it timeout delay not passed
	confQueue.enqueue(allMessage);
	return;
}
// When a confirmation is required for message we sent
void RS485::confirmationIsRequired(AllMessage allMessage)
{
	allMessage.WhenReceived = millis(); // When the confirmation queue received it
	if(!confQueue.enqueue(allMessage)) errorHandler(ERROR_CONFQUEUEOVERFLOW); // Put the message on the confirmation queue to wait for confirmation to be received
} 
// When a confirmation of the message we received is requested do these things
void RS485::confirmationWasRequested(AllMessage allMessage) 
{
	// Send out a confirmation message to the board that sent us the message
	// IF message is NOT a MESSAGE_BOARDCAST AND IS for for ME
	
	if(allMessage.Type != MESSAGE_BOARDCAST && allMessage.ReceiverId==myId)
	{
		allMessage.Type = MESSAGE_CONFIRMATION;
		allMessage.ReceiverId = allMessage.SenderId; // Back to who sent it to us
		allMessage.SenderId = myId; // From me
		allMessage.RequiresConfirmation = 0;
		OutQueueEnqueue(allMessage);		
	}
	
}
// When a conf is received do these things
void RS485::confirmationWasReceived(AllMessage allMessage) 
{
	if(allMessage.ReceiverId!=myId) return; // Not for me so don't care!
	//int confQSize = confQueue.count();
//
	//Serial.println("");
	//Serial.print("Conf q contents:");
	//Serial.println(confQSize);
	//Serial.println("========================");
//
	//for (int i = 0; i<confQSize; i++)
	//{		
		//AllMessage myMessage ;
		//myMessage = confQueue.dequeue();
		//Serial.print("Id:");
		//Serial.print(myMessage.Id);
		//Serial.print(" From:");
		//Serial.print(myMessage.SenderId,HEX);
		//Serial.print(" Time:");
		//Serial.print(myMessage.WhenReceived);
		//
		//Serial.print(" '");
//
		//for (int i = 0; i < 16; i++)
		//{
			//if(myMessage.Data[i] > 31 && myMessage.Data[i] < 127)
			//{
				//Serial.print((char)myMessage.Data[i]);
			//}
			//else
			//{
				//Serial.print("_");
			//}
				//
			////if(allMessage.Data[i]< 0x10) Serial.print("0");
			////Serial.print(allMessage.Data[i],HEX);
			////Serial.print(" ");
		//}
		//Serial.println("'");
//
	//}	
	//Serial.println("========================");
	
	//Serial.println("looking for Id:");
	//Serial.println(allMessage.Id);
	if(!confQueue.deleteByLongId(allMessage.Id))
	{
		// It could not be deleted -  What do we do now!
		errorHandler(ERROR_CONFMESSAGENOTFOUND);
	}
	
}
// All retries and the timeout has expired
void RS485::confirmationWasNotReceived() 
{
	
}
// Check if the message has timed out / expired
bool RS485::confirmationTimedOutForMessage(AllMessage allMessage) 
{
	
}
// Filter out any unwanted mesages
bool RS485::filterOutThisMessage(byte msgByte, int inputPosition)
{

	// Filter out MESSAGE_BOARDCAST if selected
	if (inputPosition == 0) // This is the type of message byte
	{
		if (ignoreBoardcasts == true && msgByte == MESSAGE_BOARDCAST) return true;
	}

	// Filter out message that are not for me
	if (inputPosition == 1) // This is the recipient byte
	{
		if (onlyReadMyMessages == true && msgByte != myId) return true;
	}
	return false;
}

// Data utility methods for retrieving and setting values in an Alldevice.Data buffer
double RS485::getKeyValueDoubleWithKey(const byte * data, const char * key)
{
	KeyValueData funcTempData;
	getKeyValueDetailsWithKey(funcTempData, data, key);
	char *buffer = new char[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	double result = atof(buffer);
	delete buffer;
	return result;
}
long RS485::getKeyValueLongWithKey(const byte * data, const char * key)
{
	KeyValueData funcTempData;
	getKeyValueDetailsWithKey(funcTempData, data, key);
	char *buffer = new char[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	long result = atof(buffer);
	delete buffer;
	return result;

}
int RS485::getKeyValueIntWithKey(const byte * data, const char * key)
{
	KeyValueData funcTempData;
	getKeyValueDetailsWithKey(funcTempData, data, key);
	char *buffer = new char[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	int result = atof(buffer);
	delete buffer;
	return result;
}

bool RS485::keyValueKeyExists(byte * data, const char * key)
{
	KeyValueData funcTempData;
	return getKeyValueDetailsWithKey(funcTempData, data, key);
}
bool RS485::getKeyValueDetailsWithKey(KeyValueData &tempData, const byte * data, const char * key)
{
	tempData.valueStartPosition = 0;
	tempData.valueLength = 0;

	bool foundKey = false;
	// Make a temporary buffer of the search string as '{' + key + '='
	char *findKeyBuffer = new char[MESSAGE_DATA_SIZE];
	strcpy(findKeyBuffer, "{");
	strcat(findKeyBuffer, key);
	strcat(findKeyBuffer, "=");

	// Search for the temporary findKeyBuffer in data
	const char* findKeyPtr = strstr((char *)data, findKeyBuffer);
	if (findKeyPtr == 0) return false; // Not there - Return false
									   
	// Get the length of the key with syntax
	int KeyLengthIncSyntax = 0;
	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		char myChar = findKeyBuffer[i];
		if (myChar == '=')
		{
			KeyLengthIncSyntax = i + 1;
			break;
		}
	}

	// Get the data details
	int keyStartPosition = findKeyPtr - (char *)data;

	// Find the length the value by looking for the closing }
	tempData.valueStartPosition = keyStartPosition + KeyLengthIncSyntax;

	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		char myChar = data[i + tempData.valueStartPosition];
		if (myChar == '}' && tempData.valueLength==0)
		{
			tempData.valueLength = i; // i is zero indexed so no need to --i even though on next char in array
			break;
		}
	}
	delete findKeyBuffer;
	return true;
}

bool RS485::buildKeyValueDataFromKeyValue(byte * data, const char * key, const char * value)
{
	// Find position of the last value cruly
	int lastValueCurly = -1;
	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		if (data[i] == '}') lastValueCurly = i;
	}

	int insertPosition = lastValueCurly + 1;

	data[insertPosition] = '{';
	insertPosition++;

	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		char keyChar = key[i];
		if (keyChar == '\0') break;
		data[insertPosition] = keyChar;
		insertPosition++;
	}
	data[insertPosition] = '=';
	insertPosition++;
	for (size_t i = 0; i < MESSAGE_KEY_SIZE; i++)
	{
		char valueChar = value[i];
		if (valueChar == '\0') break;

		data[insertPosition] = valueChar;
		insertPosition++;
	}
	data[insertPosition] = '}';
	insertPosition++;
	data[insertPosition] = '\0';

	return true;
}
bool RS485::buildKeyValueDataFromKeyValueInt(byte * data, const char * key, const int value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%i", value);
	buildKeyValueDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}
bool RS485::buildKeyValueDataFromKeyValueLong(byte * data, const char * key, const long value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%i", value);
	buildKeyValueDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}
bool RS485::buildKeyValueDataFromKeyValueDouble(byte * data, const char * key, const double value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%f", value);
	buildKeyValueDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}