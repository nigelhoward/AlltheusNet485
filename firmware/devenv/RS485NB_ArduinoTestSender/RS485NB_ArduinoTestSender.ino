#include <RS485NB.h>

// This #include statement was automatically added by the Particle IDE.
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

RS485 myChannel (fRead, fAvailable, fWrite,fWait, MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE);

int rtsPin = 2;
int busBusyPin = 3;

byte msg [MESSAGE_DATA_SIZE];
byte *ptrmsg = msg;
byte allBoardId;
int sensorValue=0;
unsigned long sequence = 800;
unsigned long millisLast;

void setup ()
{

	//pinMode(messageNotSentLED, OUTPUT);
	//pinMode(rtsPin, OUTPUT);

	Serial1.begin (250000);
	Serial.begin (250000);

	allBoardId = 0x10; // Rose
	//allBoardId = 0x12; // Thyme
	//allBoardId = 0x11; // Basil

	myChannel.messageNotSentLED = 13;
	myChannel.errorEventLED = 12;	
	myChannel.rtsPin = rtsPin;
	myChannel.busBusyRetryCount = 4;
	myChannel.onlyReadMyMessages = true;
	myChannel.ignoreBoardcasts = true;
	myChannel.confQueueSize = 10;
	myChannel.begin (allBoardId);
	myChannel.allNet485Enable(busBusyPin);
	myChannel.debugErrorsToSerial = true;

	delay(allBoardId*100);
	Serial.print("Hello I'm ");
	Serial.println(boardNameFromAllBoardId(allBoardId));
	if(myChannel.debugErrorsToSerial) 
	{
		Serial.println("Errors to serial is selected!");
	}
	// To Slow Basil down
	if(allBoardId == 0x11)
	{
		pinMode(11,OUTPUT);
		
}	// To switch on / off confirmations
	if(allBoardId == 0x10)
	{
		pinMode(10,INPUT);
	}

}  // end of setup

void analogueReadForMessage(byte * data)
{
	myChannel.buildKeyValueDataFromKeyValueInt(data, "AD0", analogRead(0));
	myChannel.buildKeyValueDataFromKeyValueInt(data, "AD1", analogRead(1));
	myChannel.buildKeyValueDataFromKeyValueInt(data, "AD2", analogRead(2));
	myChannel.buildKeyValueDataFromKeyValueInt(data, "AD3", analogRead(3));
}

void busMessage()
{
	if(allBoardId == 0x10) // Rose
	{
		AllMessage newMessage;	
		analogueReadForMessage(newMessage.Data);
		newMessage.ReceiverId = 0x11; // to Basil
		newMessage.Type = RS485::MESSAGE_MESSAGE;
		newMessage.RequiresConfirmation = 0;
		if(digitalRead(10)) newMessage.RequiresConfirmation = 2;
		myChannel.OutQueueEnqueue(newMessage);
		
	}
	if(allBoardId == 0x11) // Basil
	{
		AllMessage newMessage;
		analogueReadForMessage(newMessage.Data);
		newMessage.ReceiverId = 0xFF;
		newMessage.Type = RS485::MESSAGE_BOARDCAST;
		newMessage.RequiresConfirmation = 0;
		myChannel.OutQueueEnqueue(newMessage);
		
	}
	if(allBoardId == 0x12) // Thyme
	{		
		AllMessage newMessage;
		analogueReadForMessage(newMessage.Data);
		newMessage.ReceiverId = 0xFF;
		newMessage.Type = RS485::MESSAGE_BOARDCAST;
		newMessage.RequiresConfirmation = 0;
		myChannel.OutQueueEnqueue(newMessage);
	
	}
}

void loop ()
{
	myChannel.allNetUpdate();

	sensorValue = analogRead(A0);
	if(sensorValue < 800)
	{	
		if(millis()  > millisLast + sensorValue )
		{
			//Serial.print(sensorValue);
			busMessage();
			millisLast = millis();
		}
	}
		
	int messageInQueue = myChannel.inQueue.items;
		
	for (int i = 0; i < messageInQueue; i++)
	{

		myChannel.allNetUpdate();
		
		AllMessage allMessage;
		allMessage = myChannel.InQueueDequeue();

	}


	
	// Slow Basil down
	if(allBoardId == 0x11)
	{
		int adc1 = analogRead(1);
		if(adc1 < 5) {digitalWrite(11,HIGH);return;}
		
		digitalWrite(11,!digitalRead(11));
		//Serial.print("Delay:");
		//Serial.println(adc1);
		delayMicroseconds(adc1*15 );
	}


}  // end of loop


void randomMillsecondDelay()
{
	long r = random(100,200);
	delay(r);
}
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

		//Serial.println();
		//Serial.print(boardNameFromAllBoardId(allBoardId));
		//Serial.print(": ");
		//
		//if(allMessage.Type == RS485::MESSAGE_BOARDCAST) {Serial.print ("B:");} else {Serial.print ("M:");}
		//
		//if(allMessage.RequiresConfirmation) {Serial.print ("<");} else {Serial.print (" ");}
		//if(allMessage.Type == RS485::MESSAGE_CONFIRMATION) {Serial.print ("!");} else {Serial.print (" ");}
		//Serial.print("Msg frm:");
		//Serial.print(boardNameFromAllBoardId(allMessage.SenderId));
		//Serial.print(" Id:");
		//Serial.print(allMessage.Id);
		//Serial.print(" Cnf:");
		//Serial.print(allMessage.RequiresConfirmation);
		//
		//Serial.print(" - IqCnt:");
		//Serial.print(myChannel.inQueue.count());
		//
		//Serial.print(" OqCnt:");
		//Serial.print(myChannel.outQueue.count());
		//
		//Serial.print(" CqCnt:");
		//Serial.print(myChannel.confQueue.count());
		//
		//Serial.print(" - IqOvf:");
		//Serial.print(myChannel.getErrorCountInQueueOverflow());
		//
		//Serial.print(" OqOvf:");
		//Serial.print(myChannel.getErrorCountOutQueueOverflow());
		//
		//Serial.print(" CqOvf:");
		//Serial.print(myChannel.getErrorCountConfQueueOverflow());
		//
		//Serial.print("  ");
