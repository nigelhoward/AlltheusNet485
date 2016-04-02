#include <LiquidCrystal.h>
#include <RS485NB.h>

// Callbacks for RS485 NonBlocking Protocol
size_t fWrite(const byte what) {return Serial1.write(what);}
void fWait()
{
	
	//return; // Uncomment this return statement if you are using software serial!

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

byte msg [MESSAGE_DATA_SIZE] = "AAAAAAA";

RS485 myChannel (fRead, fAvailable, fWrite,fWait, MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE);

int rtsPin = 2;
int busBusyPin = 3;


byte allBoardId = 0x14; // Pepper

//LiquidCrystal lcd (11, 12, 9, 8, 7, 6);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
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

char lcdTextBuffer[16];

void setup()
{
	// LCD Start

	lcd.begin(16,2);
	lcd.setCursor(0, 0);
	lcd.print("Pepper V3.0");
	lcd.print(allBoardId,HEX);
	delay(2000);

	lcd.clear();
	
	Serial1.begin (250000);
	Serial.begin (115200);
	
	myChannel.messageNotSentLED = 255;
	myChannel.errorEventLED = 255;
	myChannel.rtsPin = rtsPin;
	myChannel.busBusyRetryCount = 4;
	myChannel.onlyReadMyMessages = true;
	myChannel.ignoreBoardcasts = true;
	myChannel.begin (allBoardId);
	myChannel.allNet485Enable(busBusyPin);
	myChannel.debugErrorsToSerial = false;

	delay(allBoardId * 100);
	Serial.print("Hello I'm ");
	Serial.println(boardNameFromAllBoardId(allBoardId));
	if(myChannel.debugErrorsToSerial)
	{
		Serial.println("Errors to serial is selected!");
	}

}

void loop()
{

	myChannel.allNetUpdate();
	
	int messageInQueue = myChannel.inQueue.items;
	for (int i = 0; i < messageInQueue; i++)
	{
		myChannel.allNetUpdate();
		
		AllMessage allMessage;
		allMessage = myChannel.InQueueDequeue();
		if (myChannel.keyValueKeyExists(allMessage.Data, "LCDRow"))
		{
			int rowNumber = myChannel.getKeyValueIntWithKey(allMessage.Data, "LCDRow"); // The keyValue with the row number
			lcd.setCursor(0, rowNumber);

			// After call keyValueData will contain length and location of data in allMessage.Data
			KeyValueData keyValueData;
			myChannel.getKeyValueDetailsWithKey(keyValueData,allMessage.Data, "Text"); // The text
			
			Serial.println();
			Serial.println();
			Serial.print("keyValueData.valueLength:");
			Serial.println(keyValueData.valueLength);
			Serial.print("keyValueData.valueStartPosition:");
			Serial.println(keyValueData.valueStartPosition);

			for (int i = 0; i < 35; i++)
			{

				if (allMessage.Data[i] > 31 && allMessage.Data[i] < 127)
				{
					Serial.print((char)allMessage.Data[i]);
				}
				else
				{
					Serial.print("_");
				}
			}



			for (int i = 0; i < keyValueData.valueLength; i++)
			{
				lcd.print((char)allMessage.Data[i+ keyValueData.valueStartPosition]);
			}
		}

	}

}

String boardNameFromAllBoardId(byte allBoardId)
{
	if(allBoardId== 0x01) return "Apri"; // Apricot
	if(allBoardId== 0x02) return "Cher"; // Cherry
	if(allBoardId== 0x03) return "Appl"; // Apple
	if(allBoardId== 0x04) return "Kiwi"; // Kiwi
	if(allBoardId== 0x05) return "Pine"; // Pineapple
	if(allBoardId== 0x10) return "Rose"; // Rose
	if(allBoardId== 0x11) return "Basl"; // Basl
	if(allBoardId== 0x12) return "Thym"; // Thym
	if(allBoardId== 0x14) return "Pepr"; // Pepr
	
	return "Who";
}


