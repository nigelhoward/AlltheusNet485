/*
 RS485 protocol library - Particle Photo Version
 Based on above RS485 Non blocking by Nick Gammon
 Thanks Nick :-) I could not have written this without your excellent code and forum
 Please note I am not a C++ programmer. Pointers drive me nuts!

 In my setup running a medium size bus, 250000 Baud, about 10 boards and sending 20 messages per second produced less than 0.1% errors
 See how the network is performing by checking the errorCounters and keeping track of the last sequence number for each board in an array
 Comparing sequence numbers they always increment by each time a message is sent. You can't use sequence number +1 following if confirmations are requested because confirmations have same seq number
 If the last message sequence number +1 doesn't equal this sequence number then you lost a message. Too bad, slow down a bit!
 Hardware serial performed much better than software serial for me. 250000 Baud was about the max for the serial port.
 If you build a test receiver to analyze above then make sure it doesn't do too much Serial.Print to a slow serial port. This will miss messages.

 Version P3.4 - By TopBanana 03-04-2016

*/
#ifndef _RS485NB
#define _RS485NB


// Particle boards have a PLATFORM_ID Defined but Arduino boards don't. Below includes relevant header.
#ifdef PLATFORM_ID
    #include "application.h" // This is Particle.io specific - Would have been arduino.h for Arduino
#else
    #include "Arduino.h"
#endif
#include "AllQueue.h"

const int MESSAGE_DATA_SIZE = 50;
const int MESSAGE_HEADER_SIZE = 8;

// Structure to hold the keyValue length and position
struct KeyValueData
{
	int valueStartPosition = 0;
	int valueLength = 0;
	bool error = false;
};

class AllMessage
	{
	public:
		AllMessage(int emptyDataChar=0)
		{
			memset(Data, emptyDataChar, MESSAGE_DATA_SIZE);
		};

		unsigned long Id;				// Id / Sequence number of the received message
		byte SenderId;					// Who sent the message
		byte ReceiverId;				// Who the message is for
		byte Type;						// Type of message
		byte RequiresConfirmation;		// Message requires a confirmation
		unsigned long WhenReceived;		// When the message was received in millis
		byte Data[MESSAGE_DATA_SIZE];	// Data bytes not including the header size
	};

class RS485
  {

	typedef size_t (*WriteCallback)  (const byte what);    // send a byte to serial port
	typedef int  (*AvailableCallback)  ();    // return number of bytes available
	typedef int  (*ReadCallback)  ();    // read a byte from serial port
	typedef void (*WaitCallback)  (); // For using hardware serial ports

	enum
	{
		STX = '\2',   // start of text
		ETX = '\3'    // end of text
	};  // end of STX/ETX enum

	// callback functions to do reading/writing
	ReadCallback fReadCallback_;
	AvailableCallback fAvailableCallback_;
	WriteCallback fWriteCallback_;
	WaitCallback fWaitCallback_;


	// where we save incoming stuff
	byte * data_;

	// how much data is in the buffer
	const int bufferSize_;

	// this is true once we have valid data in buf
	bool available_;

	// an STX (start of text) signals a packet start
	bool haveSTX_;

	// count of errors
	unsigned long errorCount_;

	// variables below are set when we get an STX
	bool haveETX_;
	byte inputPos_;
	byte currentByte_;
	bool firstNibble_;
	unsigned long startTime_;

	// helper private functions
	byte crc8 (const byte *addr, byte len);
	void sendComplemented (const byte what);

	/*
	Additional private properties and methods
	Added by Tb.
	*/

	void randomRetryMicrosDelay(); // Retry sending message delay in microseconds
	void doDelayStuff(); // Stuff to do for a delay
	void messageWasSent(); // Stuff to do when a message got sent
	void messageWasReceived(); // Stuff to do when a message got sent

	// Error counters
	unsigned int errorCountNibble_;
	unsigned int errorCountCRC_;
	unsigned int errorCountOverflow_;
	unsigned int errorCountInQueueOverflow_;
	unsigned int errorCountOutQueueOverflow_;
	unsigned int errorCountConfQueueOverflow_;
	unsigned int errorCountConfQueueTimeout_;
	unsigned long errorLastMillis_;

	// Sender properties
	long sequenceNumber; // Incremented sequence number for checking that packets arrive in sequence
	bool boardCastMessage = false; // Some messages are sent for everyone to see

	// Debug
	bool debug = false;


	// Board Id
	byte myId = 0xFF; // Aka senderId
	
	// Bus speed	
	unsigned int busSpeed = 0; // Approx messages per second calculation
	unsigned long busSpeedLastMillis = 0; // Use to calculate approx bus speed in messages per second
	unsigned long busSpeedLastMessageReceivedCount = 0; // Use to calculate approx bus speed in messages per second
	void calculateBusSpeed(); // Does the waiting and calculation

	// handle incoming data, return true if packet ready
	// Now private because no one should call this. Call allNetUpdate instead
	bool updateReceive();

  public:
	  // Types of message
	  enum
	  {
		  MESSAGE_BOARDCAST,	// Message sent to all boards on the bus
		  MESSAGE_MESSAGE,		// For a specific Id
		  MESSAGE_CONFIRMATION	// Confirmation that message received
	  };
	  
	  // Types of errors
	  enum
	  {
		  ERROR_NIBBLE,				// Nibble Error occurred whilst reading data from the serial port
		  ERROR_CRC,				// CRC Error occurred whilst reading data from the serial port
		  ERROR_BUFFEROVERFLOW,		// Size of buffer was about to be exceeded before ETX-CRC was received
		  ERROR_INQUEUEOVERFLOW,	// Input / receive queue ran out off space 
		  ERROR_OUTQUEUEOVERFLOW,	// Output / send queue ran out of space
		  ERROR_CONFQUEUEOVERFLOW,	// Confirmation message queue ran out of space
		  ERROR_CONFRECEIPTTIMEOUT,	// Confirmation message wasn't received after retrying
		  ERROR_CONFMESSAGENOTFOUND	// Confirmation message wasn't found in local confirmation queue   		  
	  };

	// constructor
    RS485 (
			ReadCallback fReadCallback,
			AvailableCallback fAvailableCallback,
			WriteCallback fWriteCallback,
			WaitCallback fWaitCallback,
			const byte bufferSize)
			:
			fReadCallback_ (fReadCallback),
			fAvailableCallback_ (fAvailableCallback),
			fWriteCallback_ (fWriteCallback),
			fWaitCallback_(fWaitCallback),
			bufferSize_ (bufferSize),
			data_ (NULL) {}

    // destructor - frees memory used
    ~RS485 () { stop (); }

    // allocate memory for buf_
    void begin (byte);

    // free memory in buf_
    void stop ();

    // reset to no incoming data (eg. after a timeout)
    void reset ();

    // send data
    // receiverId - Who the message is for
    // messageType - Distinguishing between message types - reserved type of 0X00 for a BoardCast
    // messageRequiresConfirmation - Please let me know you got my message! Library does not respond so handle this in your application code
    bool sendMsg (const byte * data, const byte length, const byte receiverId, const byte messageType, const byte messageRequiresConfirmation, unsigned long givenMessageId);

    // returns true if packet available
    bool available () const { return available_; };

    // once available, returns the address of the current message
    const byte * getData ()   const { return data_; }
    const byte   getLength () const { return inputPos_; }

    // return how many errors we have had
    unsigned long getErrorCount () const { return errorCount_; }

    // return when last packet started
    unsigned long getPacketStartTime () const { return startTime_; }

    // return true if a packet has started to be received
    bool isPacketStarted () const { return haveSTX_; }

/*
   Additional public properties and methods
   Added by Tb.
*/
	// Output any errors to the default serial port
	bool debugErrorsToSerial = false;
	
	// Queues for messages
	AllQueue <AllMessage> inQueue;
	AllQueue <AllMessage> outQueue;
	AllQueue <AllMessage> confQueue;

	int inQueueSize = 2;
	int outQueueSize = 2;
	int confQueueSize = 2;
	
	int confQueueTimeoutDelay = 500; // How long a message stays in the queue waiting for a confirmation before delete or re-transmit
	
	unsigned int messagesReceivedCounter = 0;
	unsigned int messagesSentCounter = 0;

	// The actual in and out queuing methods
	AllMessage InQueueDequeue();
	void OutQueueEnqueue(AllMessage);

	void initMessageQueues(int,int,int);

	int rtsPin = 255; // RS485 RTS Pin. Pulled high when sending data on the bus.
	int messageNotSentLED = 255; // RS485 RTS Pin. Pulled high when sending data on the bus.
	int errorEventLED = 255; // RS485 RTS Pin. Pulled high when sending data on the bus.

	void incrementErrorCount() {errorCount_ ++; }; // Means of incrementing the error counter in application for other errors
	void decrementErrorCount() {errorCount_ --; }; // Means of decrementing the error counter in application
	unsigned long getErrorCountNibble () const { return errorCountNibble_; }
	unsigned long getErrorCountCRC () const { return errorCountCRC_; }
	unsigned long getErrorCountOverflow () const { return errorCountOverflow_; }
	unsigned long getErrorCountInQueueOverflow () const { return errorCountInQueueOverflow_; }
	unsigned long getErrorCountOutQueueOverflow () const { return errorCountOutQueueOverflow_; }
	unsigned long getErrorCountConfQueueOverflow () const { return errorCountConfQueueOverflow_; }
	unsigned long getErrorCountConfQueueTimeout () const { return errorCountConfQueueTimeout_; }
		

	// Enable / disable debug messages to serial port (USB)
	// Doing it like this means that individual messages can be debugged instead of debugging the entire class
	void debugEnable(){debug = true;}
	void debugDisable(){debug = false;}

	// received message properties
	byte messageSenderId;		// Who sent the message
	byte messageReceiverId;		// Who the message is for
	byte messageType = 0xFF;	// Type of message
	long messageSequenceNumber; // sequence number of the received message

	// receive message filters
	bool onlyReadMyMessages = false; // Only messages with myId as the receiver are read
	bool ignoreBoardcasts = false; // Boardcasts originate from Id 0 and are for everyone to read unless this set to ignore them

	// AllNet485 Uses an additional wire to stop other boards from transmitting. This reduces the speed capabilities of RS485
	// But adds the ability to have multiple masters that can all talk on the same bus
	// Primarily for home automation which doesn't require a very fast bus but does require reliability
	// AllNet485 Methods & properties

	bool allNet485Enabled = false; // No buss busy stuff is the default! Shame if you leave it like this :-)
	byte busBusyPin = 255; // If this is 255 then AllNet485 Bus checking has been disabled

	void allNet485Enable (byte busyPin);
	void allNet485Disable (); //

	void allNetUpdate(); // Updates allNet stuff - inQueue , outQueue , confQueue and confirmation messages

	void busMakeBusy(); // Makes the bus busy by pulling the busBusyPin wire to zero
	void busMakeIdle(); // Lets the bus go back up to being pulled hi by the resistor on the wire
	bool busIsBusy(); // Reads the wire to see if it is high (idle) or low (busy)
	void busDelayMicros(unsigned long); // Bus delays for delaying by about the same microseconds but with bus updates / reads
	void busDelayMillis(int);			// As above but in milliseconds 
	
	int  getBusSpeed(); // Approx messages per second calculation

	// Confirmation byte set by the message sending code. Bigger the number the more retries it does
	byte messageRequiresConfirmation = 0; // Set if sender asked for a confirmation


	// Widens the gap either side of the data transmitted when bufferBusy wire goes low
	// Can help reduce collisions but better kept low.
	byte busBusyDelayBeforeTransmit = 1; // Millis
	byte busBusyDelayAfterTransmit = 1; // Millis

	// Retries lengthen the time spent in the sendMessage method. Better to keep this low unless you don't mind missing our on
	// Other data that might have been sent to the board. No the less the retry delay is still very short.
	// Better to handle retries in the application code as it can decide how important retrying is.
	byte busBusyRetryCount = 5; // How many times send message is repeated before giving up and returning false - message not sent;

	// For handling errors
	void errorHandler(int);
	void errorLEDHandler(bool);
	
	// When a board starts it waits until it has seen busBusy for some cycles or a some millis before starting
	void bootBusWaiter();
	// Flashes the LEDS that have valid pin numbers
	void bootLightShow();
	
	// Confirmation methods
	void confQueueHandler(); // Scans the conf queue and re-sends or deletes messages
	void confirmationIsRequired(AllMessage); // When a confirmation is required for message we sent
	void confirmationWasRequested(AllMessage); // When a confirmation is requested do these things
	void confirmationWasReceived(AllMessage); // When a conf is received do these things
	void confirmationWasNotReceived(); // All retries and the timeout has expired
	bool confirmationTimedOutForMessage(AllMessage allMessage); // Check if the message has timed out / expired
	bool filterOutThisMessage(byte msgByte, int inputPosition); // Filters out unwanted messages

	// Methods for setting and retrieving values from the AllDevice.Data property or any buffer by reference
	// Use KeyValue data structure if calling getKeyValueDetailsWithKey for data location and length
	// Some examples:-
	// buildKeyValueDataFromKeyValue(data, "Day", "Monday");
	// buildKeyValueDataFromKeyValueDouble(data, "D", 1235.456);
	// buildKeyValueDataFromKeyValueLong(data, "Long",74564894);
	// buildKeyValueDataFromKeyValueInt(data, "Int",45345);
	// Will populate the given data C type string with the following
	// {Day=Monday}{D=1235.456000}{Long=74564894}{Int=45345}

	const int MESSAGE_KEY_SIZE = 12; // How long the key char array can be - 12 should cover most uses - Case sensitive
									 
	// Returns true / false if a given key exists in the data
	bool keyValueKeyExists(byte * data, const char * key);
	// Main get value method which passes back a structure called KeyValueData which contains the array position and size of value associated with the given key
	bool getKeyValueDetailsWithKey(KeyValueData &tempData, const byte * data, const char * key);
	// Uses main get value method to return a long from given key
	long getKeyValueLongWithKey(const byte * data, const char * key);
	// Uses main get value method to return a Double from given key
	double getKeyValueDoubleWithKey(const byte * data, const char * key);
	// Uses main get value method to return an int from given key
	int getKeyValueIntWithKey(const byte * data, const char * key);

	// Main method for putting the key value in it's syntax in the given data buffer (byte array)
	bool buildKeyValueDataFromKeyValue(byte * data, const char * key, const char *value);
	// Uses Main buildKeyValue.. Method to create a double key value
	bool buildKeyValueDataFromKeyValueDouble(byte * data, const char * key, const double value); 
	// Uses Main buildKeyValue.. Method to create a long key value
	bool buildKeyValueDataFromKeyValueLong(byte * data, const char * key, const long value);
	// Uses Main buildKeyValue.. Method to create an int key value
	bool buildKeyValueDataFromKeyValueInt(byte * data, const char * key, const int value);

  }; // end of class RS485

#endif // !_RS485NB
