#include <RS485NB.h>
#include <AllNetDevice.h>

String boardNameFromAllBoardId(byte allBoardId)
{
	if(allBoardId== 0x01) return "Apri"; // Apricot
	if(allBoardId== 0x02) return "Cher"; // Cherry
	if(allBoardId== 0x03) return "Appl"; // Apple
	if(allBoardId== 0x04) return "Kiwi"; // Kiwi
	if(allBoardId== 0x05) return "Pine"; // Pineapple
	if(allBoardId== 0x10) return "Rose"; // Rose
	if(allBoardId== 0x11) return "Basi"; // Basil
	if(allBoardId== 0x12) return "Thym"; // Thyme	
	if(allBoardId== 0x14) return "Pepr"; // Pepr
	
	return "Who";
}

// Callbacks for RS485 NonBlocking Protocol
size_t fWrite(const byte what) {return Serial1.write(what);}
void fWait()
{
	
	// Uncomment this return statement if you are using software serial!

	//////////////////////////////////////////////////////////////////////////
	// You must change below to use the correct port registers!
	// If you are using software serial keep it commented out and
	// uncomment the above return statement. I'm using a Leonardo so it's Serial1.
	// Credit to Nick Gammon - http://www.gammon.com.au/forum/?id=11428
	//////////////////////////////////////////////////////////////////////////
	
	while (!(UCSR1A & (1 << UDRE1)))	// Wait for empty transmit buffer
	UCSR1A |= 1 << TXC1;				// Mark transmission not complete
	while (!(UCSR1A & (1 << TXC1)));   // Wait for the transmission to complete
}
int fAvailable(){ return Serial1.available(); }
int fRead(){ return Serial1.read(); }


byte msg [MESSAGE_DATA_SIZE];

RS485 myChannel (fRead, fAvailable, fWrite,fWait, MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE);

volatile int lastErrors = 0;
volatile long messagesReceived = 0;
volatile int sequenceError = 0;
volatile long thisSeq = 0;
volatile long lastSeq = 0;
volatile int thisPeriod = 0;
volatile int lastPeriod = 0;
volatile long messagesAtPeriodStart = 0;
volatile int messagesPerPeriod = 0;
volatile int boardId;
volatile float totalErrors;
volatile float totalMessages;
volatile float percentageErrors;
volatile long lastSequence[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile long lastPacketReceiveTimeMillis;

int samplePeriodForMessagesPerPeriod = 6000;
long lastMillis = 0;


byte myId = 0x88;

int rtsPin = 3;
int busBusyPin = 2;
int busBusyLEDPin = 52;
int errorLEDPin = 53;
int messageNotSentLEDPin = 13;

AllNetDevice myDevice;

volatile bool newMessage = false;

void setup ()
{
	thisPeriod = millis() / samplePeriodForMessagesPerPeriod;
	lastPeriod = thisPeriod;
	
	Serial1.begin (250000);
	Serial.begin (250000);

	myChannel.messageNotSentLED = messageNotSentLEDPin;
	myChannel.errorEventLED = errorLEDPin;
	myChannel.rtsPin = rtsPin;
	myChannel.busBusyRetryCount = 5;
	myChannel.onlyReadMyMessages=false;
	myChannel.ignoreBoardcasts=false;
	
	myChannel.begin (myId);
	
	pinMode(busBusyLEDPin,OUTPUT);
	
	myChannel.inQueueSize = 20;
	myChannel.allNet485Enable(busBusyPin);

	
	myChannel.inQueue.debug = false;
	
	attachInterrupt(digitalPinToInterrupt(busBusyPin), busBusyInterrupt, CHANGE);
	
	Serial.println("Hi I'm Chilli!");
	

	// TEST AllNetDevice //

	myDevice.Type = AllNetDevice::DEVICE_TYPE_COUNTER;
	myDevice.BoardId = myId;
	myDevice.Code = 0x01;
	myDevice.BoardIdLinked = 0x14;
	myDevice.CodeLinked = 0x02;
	myDevice.receiveDataFunction = doThisWhenDataArrivesB;

	// TEST AllNetDevice END //
	
}  // end of setup

// TEST AllNetDevice //
bool doThisWhenDataArrivesA(byte* data)
{
	Serial.print("doThisWhenDataArrivesA");
	return true;
}
bool doThisWhenDataArrivesB(byte* data)
{
	Serial.print("doThisWhenDataArrivesB");
	return true;
}
// TEST AllNetDevice END //


void busBusyInterrupt()
{
	digitalWrite(busBusyLEDPin,!digitalRead(busBusyLEDPin));
}


void updateStats(AllMessage allMessage)
{
	if(millis() < 1000) return; // Wait for start so we don't hit partial messages when re-booting;
	messagesReceived++;
	  
	messagesReceived++;
	byte senderId = allMessage.SenderId;
	long thisSeq = allMessage.Id;
	long lastSeq = lastSequence[senderId];
	
	if((lastSeq +1 != thisSeq) && lastSeq!=0)
	{
		//sequenceError++;
		//myChannel.incrementErrorCount();
	}
	
	lastSequence[senderId] = thisSeq;
	lastPacketReceiveTimeMillis = millis();
	
	totalErrors = (float) myChannel.getErrorCount() * 1000;
	totalMessages = (float) messagesReceived * 1000;
	
	percentageErrors = (totalErrors / totalMessages) * 100;
	newMessage = false;
}

void loop ()
{
	AllMessage allMessage;
	allMessage.Type = RS485::MESSAGE_DEVICE;
	myDevice.Process(allMessage);
	delay(5000);
	return;

	myChannel.allNetUpdate();
	int messageInQueue = myChannel.inQueue.items;
	for (int i = 0; i < messageInQueue; i++)
	{
		newMessage = true;
		myChannel.allNetUpdate();
		
		AllMessage allMessage;
		allMessage = myChannel.InQueueDequeue();

		//if (allMessage.SenderId == 0x01)
		//{
			updateStats(allMessage);

			serialPrintStats(allMessage);

			serialPrintErrors();

			Serial.println();

		//}

	}

	if (millis() > lastMillis + 1000)
	{		
		messagesPerPeriod = messagesReceived - messagesAtPeriodStart ;
		messagesAtPeriodStart = messagesReceived;
		lastMillis = millis();
		//sendMessage();
	}

	
}  // end of loop


void serialPrintStats(AllMessage allMessage)
{

	if(allMessage.Type == RS485::MESSAGE_BOARDCAST) {Serial.print ("B:");} else {Serial.print ("M:");}
	Serial.print (messagesReceived);	
	Serial.print (" ");
	
	if(allMessage.RequiresConfirmation)
	{
		Serial.print (allMessage.RequiresConfirmation);
		Serial.print (" <");

	}
	else
	{
		Serial.print ("   ");
	}
	
	if(allMessage.Type == RS485::MESSAGE_CONFIRMATION) {Serial.print (">");} else {Serial.print (" ");}
	
	Serial.print (" Rx:");
	if(allMessage.ReceiverId < 16) Serial.print ("0");
	Serial.print(allMessage.ReceiverId,HEX);
	
	Serial.print (" Tx:");
	if(allMessage.SenderId < 16) Serial.print ("0");
	Serial.print(allMessage.SenderId,HEX);
	Serial.print(" ");
	Serial.print(boardNameFromAllBoardId(allMessage.SenderId));
	
	Serial.print (" Ty:");
	Serial.print(allMessage.Type ,HEX);
	Serial.print (" ");
	Serial.print(myChannel.getBusSpeed());
	Serial.print ("Hz");
	Serial.print(" ");
	Serial.print(percentageErrors);
	Serial.print("%");
	Serial.print(" E:");
	Serial.print(myChannel.getErrorCount());
	//Serial.print(" Q:");
	//Serial.print(myChannel.inQueue.items);
	//Serial.print("/");
	//Serial.print(myChannel.inQueue.size);
	Serial.print(" I:");
	Serial.print(allMessage.Id);
	
	Serial.print(" '");

	for (int i = 0; i < 35; i++)
	{
		myChannel.allNetUpdate();
		if(allMessage.Data[i] > 31 && allMessage.Data[i] < 127)
		{
			Serial.print((char)allMessage.Data[i]);
		}
		else
		{
			Serial.print("_");			
		}
				
		//if(allMessage.Data[i]< 0x10) Serial.print("0");
		//Serial.print(allMessage.Data[i],HEX);
		//Serial.print(" ");
	}
    Serial.print("'");
	
}
void serialPrintErrors()
{
	if(lastErrors == myChannel.getErrorCount()) return;

	lastErrors = myChannel.getErrorCount();
	
	//if(sequenceError>0)
	//{
		//Serial.print(" S");
		//Serial.print(sequenceError);
	//}
	
	if(myChannel.getErrorCountNibble()>0)
	{
		Serial.print(" N");
		Serial.print(myChannel.getErrorCountNibble());
	}

	if(myChannel.getErrorCountCRC()>0)
	{
		Serial.print(" C");
		Serial.print(myChannel.getErrorCountCRC());
		
	}
	if(myChannel.getErrorCountOverflow()>0)
	{
		Serial.print(" O:");
		Serial.print(myChannel.getErrorCountOverflow());
	}
	
	if(myChannel.getErrorCount()>0)
	{
		Serial.print(" T");
		Serial.print(myChannel.getErrorCount());
		
	}
	

}







