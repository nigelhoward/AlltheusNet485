#include <EthernetUdp.h>
#include <EthernetServer.h>
#include <EthernetClient.h>
#include <Ethernet.h>
#include <Dns.h>
#include <Dhcp.h>
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

const int RECEIVE_MESSAGE_BUFFER_SIZE = 256;
byte messageReceivedBuffer[RECEIVE_MESSAGE_BUFFER_SIZE];
int messageReceivedBufferIndex;
bool messageStartJsonReceived = false;
bool messageEndJsonReceived = false;
const char messageStartJsonReceivedChar = '[';
const char  messageEndJsonReceivedChar = ']';


// TCP Server Client

// MAC address for the ethernet shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//the IP address for the shield:
byte ip[] = { 192, 168, 1, 151 };
// the router's gateway address:
byte gateway[] = { 192, 168, 1, 1 };
// the subnet:
byte subnet[] = { 255, 255, 255, 0 };

EthernetServer server = EthernetServer(23);
EthernetClient client;

void sendServerData(AllMessage allMessage);
AllMessage readServerData();

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

	myChannel.debugErrorsToSerial = true;
	
	Ethernet.begin(mac, ip, gateway, subnet);
	server.begin();

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

/////////////////////////////
//         Loop            //
/////////////////////////////

void loop ()
{
	if (!client)
	{
		client = server.available();
	}

	myChannel.allNetUpdate();
	int messageInQueue = myChannel.inQueue.items;
	for (int i = 0; i < messageInQueue; i++)
	{
		newMessage = true;
		myChannel.allNetUpdate();
		
		AllMessage allMessage;
		allMessage = myChannel.InQueueDequeue();


		sendServerData(allMessage);
		updateStats(allMessage);

		//serialPrintStats(allMessage);

		//serialPrintErrors();

		//Serial.println();


	}

	if (millis() > lastMillis + 1000)
	{		
		messagesPerPeriod = messagesReceived - messagesAtPeriodStart ;
		messagesAtPeriodStart = messagesReceived;
		lastMillis = millis();
		//sendMessage();
	}

	readServerData();

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

AllMessage readServerData()
{
	bool debug = false;
	//int bytesRead = client.read(*messageInBuffer, MESSAGE_DATA_SIZE);
	if (client)
	{
		int readByte = client.read();
		if (readByte != -1)
		{
			long Id = 0;
			int SId = 0;
			int RId = 0;
			bool Cnf = 0;
			byte aBuffer[50];


			AllMessage allMessage;

			switch (readByte)
			{

				case messageStartJsonReceivedChar:

					messageStartJsonReceived = true;

					break; // messageStartJsonReceivedChar

				case messageEndJsonReceivedChar:
				
					// Process the message then reset the read
					
					messageReceivedBuffer[messageReceivedBufferIndex] = '\0'; // Terminate the char string correctly

	
					SId = myChannel.getKeyValueIntWithKey(messageReceivedBuffer, "SId");					
					RId = myChannel.getKeyValueIntWithKey(messageReceivedBuffer, "RId");
					Cnf = myChannel.getKeyValueIntWithKey(messageReceivedBuffer, "Cnf");

					

					// Data is has undetermined size so need to get KeyValueData object for start and length
					if (debug)
					{
						Serial.print(" SenderId=");
						Serial.print(SId);					
						Serial.print(" ReceiverId=");
						Serial.print(RId);
						Serial.print(" ConfirmationRequired=");
						Serial.print(Cnf);
						Serial.println("messageReceivedBuffer");

						Serial.print(" Data=");

						for (int i = 0; i < messageReceivedBufferIndex; i++)
						{
							Serial.print((char)messageReceivedBuffer[i]);
						}
					}
					static KeyValueData keyValueData;

					myChannel.getKeyValueDetailsWithKey(keyValueData, messageReceivedBuffer, "Dat"); // The data text

					allMessage.SenderId = SId;
					allMessage.ReceiverId = RId;
					allMessage.RequiresConfirmation = Cnf;
					allMessage.Type = RS485::MESSAGE_MESSAGE;

					for (int i = 0; i < keyValueData.valueLength; i++)
					{
						char byteForMessage = messageReceivedBuffer[i + keyValueData.valueStartPosition];

						// Put back curlys because send from csharp used <>
						if (byteForMessage == '<') { byteForMessage = '{'; }
						if (byteForMessage == '>') { byteForMessage = '}'; }

						allMessage.Data[i] = byteForMessage;
					}

					myChannel.OutQueueEnqueue(allMessage);

					resetReadServerData();

					break; // messageEndJsonReceivedChar

				default:

					if (messageStartJsonReceived == true)
					{
						messageReceivedBuffer[messageReceivedBufferIndex] = readByte;
						messageReceivedBufferIndex++;

						if (messageReceivedBufferIndex > RECEIVE_MESSAGE_BUFFER_SIZE)
						{
							Serial.println("messageReceivedBufferIndex overflow");
							resetReadServerData();
							break;
						}

						//Serial.print(char(readByte));
					}
					break; // Default
			}

		}
	}



	//if (readByte != -1)
	//{
	//	messageInBufferIndex++;
	//	if (messageInBufferIndex>MESSAGE_DATA_SIZE) messageInBufferIndex == 0;
	//	// Do something with the data
	//	digitalWrite(myChannel.errorEventLED, !digitalRead(myChannel.errorEventLED));
	//}
}
void resetReadServerData()
{
	messageReceivedBufferIndex = 0;
	messageStartJsonReceived = false;
	messageEndJsonReceived = false;

	for (int i = 0; i < RECEIVE_MESSAGE_BUFFER_SIZE; i++)
	{
		messageReceivedBuffer[i] = '\0';
	}

}
void sendServerData(AllMessage allMessage)
{
	if (client)
	{
		server.print("[");

		//server.write("{\"AllMessage\":");
		server.print("{");

		myChannel.allNetUpdate();

		server.print("\"Id\":");
		server.print(allMessage.Id);
		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"Typ\":");
		server.print(allMessage.Type);
		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"SId\":");
		server.print(allMessage.SenderId);
		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"RId\":");
		server.print(allMessage.ReceiverId);
		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"Cnf\":");
		if (allMessage.RequiresConfirmation == true)
		{
			server.print("\"TRUE\"");
		}
		else
		{
			server.print("\"FALSE\"");
		}
		myChannel.allNetUpdate();

		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"Rcv\":");
		server.print(allMessage.WhenReceived);
		server.print(",");

		myChannel.allNetUpdate();

		server.print("\"Dat\":\"");
		//server.print("None");

		for (int i = 0; i < MESSAGE_DATA_SIZE; i++)
		{
			server.write(allMessage.Data[i]);

			myChannel.allNetUpdate();
		}

		server.print("\"");
		server.print("}");
		server.print("]");
	}
}







