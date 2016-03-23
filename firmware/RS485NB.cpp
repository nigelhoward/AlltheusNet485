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

// Version P2.2 - By TopBanana 12-03-2016

#include "RS485NB.h"

// allocate the requested buffer size
void RS485::begin (byte boardId)
  {
  myId = boardId;
  data_ = (byte *) malloc (bufferSize_);
  reset ();
  errorCount_ = 0;
  pinMode(rtsPin, OUTPUT);

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
		if (debug)	{Serial.print("i:"); Serial.println(i);}

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
  byte messageData[MESSAGE_DATA_SIZE+MESSAGE_HEADER_SIZE];

  // Header
  int headerSize = MESSAGE_HEADER_SIZE;
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
	messageData[headerSize+i] = data [i];
	//if (allNet485Enabled) update();
  }

  // Send the data
  for (byte i = 0; i < (length + headerSize); i++)
  {
	  sendComplemented(messageData[i]);
	  //if (allNet485Enabled) update();
  }
  
  // End of transmission byte
  fWriteCallback_ (ETX);  // ETX

  // CRC Byte
  sendComplemented (crc8 (messageData, length + headerSize ));
  fWaitCallback_(); // Waits for hardware serial sends to complete

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
      				errorCountNibble_++;
      				errorCount_++;

      				reset ();
      				if(debug) Serial.print("N");
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
				errorCountCRC_++;
				errorCount_++;
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

			// Push the message on the queue
			inQueue.enqueue(newAllMessage);

			// Send confirmation if required
			if(messageRequiresConfirmation) sendConfirmation(newAllMessage);
			
            available_ = true;

            return true;  // show data ready
            }  // end if have ETX already

          // keep adding if not full
        if (inputPos_ < bufferSize_)
  		  {
			// Is it a boardCast? - messageType=0x00
			if(inputPos_ == 0 && currentByte_ == BOARDCAST)
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
        errorCountOverflow_++;
        errorCount_++;
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

  }

  AllMessage RS485::InQueueDequeue()
  {
	return inQueue.dequeue();
  }

  void RS485::allNet485Enable (byte busyPin)
  {
    busBusyPin = busyPin;
    allNet485Enabled = true;
    pinMode(busBusyPin, INPUT);

	// Initalise the queues
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
  // Does a delay of approx the right number of microseconds whilst still reading the bus / serial port
  void RS485::busDelay(int delayMS)
  {
	unsigned long microDelayRequired = 1000 * (unsigned long)delayMS;
	unsigned long microStart = micros();

	while (micros() < (microStart + microDelayRequired))
	{
		// Check if overflow (back to wero for micros() occured during this method
		if (microStart > micros()) microStart = micros();  // Re-assign it so we don't get a super huge delay
		if(allNet485Enabled) update(); // Read any data that's on it's way in if AllNet is enabled
	}

  }

  // Does a random delay with a twist
  int RS485::randomRetryDelay()
  {
    int randomDelay = random(10,100000) + (50 * myId);
    for(int i = 0 ; randomDelay;i++)
    {
      delayMicroseconds(1);
      doDelayStuff();
    }
  }
  // Stuff that needs doing while a delay is delaying
  void RS485::doDelayStuff()
  {
	  // Reading the bus would be good here
  }

  bool RS485::sendConfirmation(AllMessage allMessage)
  {
	//allMessage.SenderId == 0x11; // Basil
	
	//byte confirmationMessage[] ="CONFIRMATION      ";
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
