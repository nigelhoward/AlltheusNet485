// This #include statement was automatically added by the Particle IDE.
#include "RS485NB.h"


size_t fWrite(const byte what) {return Serial1.write(what);}
int fAvailable(){return Serial1.available();} // Don't be tempted to comment out the serial stuff here or board will hang waiting
int fRead(){return Serial1.read();} // See above
void fWait() {while( !Serial1TXcomplete()){}} // See above
int Serial1TXcomplete(void)
  {
    // MAKE SURE you choose the correct USART! - Wrong one = IoT in infinite loop!
  	// Check if the USART Transmission is complete
  	if(USART_GetFlagStatus(USART1, USART_FLAG_TC) != RESET)
  		return 1; // Complete
  	else
  		return 0; // Not Complete
  }
// MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE
RS485 myChannel (fRead, fAvailable, fWrite,fWait,MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE);

int messageNotSentLED = D7;
int rtsPin = D5;
int busBusyPin = D3;

byte msg [MESSAGE_DATA_SIZE] = "Cherryisme";
byte *ptrmsg = msg;
byte allBoardId=0x00;

int sequence = 0;

// For stats and counting
int lastErrors = 0;
long messagesReceived = 0;
int sequenceError = 0;
long thisSeq = 0;
long lastSeq = 0;
int thisPeriod = 0;
int lastPeriod = 0;
long messagesAtPeriodStart = 0;
int messagesPerPeriod = 0;
int boardId;
float totalErrors;
float totalMessages;
float percentageErrors = 0;
long lastSequence[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
long lastPacketReceiveTimeMillis;
long lastMillis = 0;

bool secondToggle = false;
bool halfSecondToggle = false;
bool tenthSecondToggle = false;
bool thisTimeThatTime = false;
bool thisTimeThatTime2 = false;

bool doneThisOnce = false;

char lcdTextBuffer[MESSAGE_DATA_SIZE];
char messageInBuffer[MESSAGE_DATA_SIZE];
int messageInBufferIndex;

// TCP Server Client
TCPServer server = TCPServer(23);
TCPClient client;

void updateStats(AllMessage allMessage);
void sendServerData(AllMessage allMessage);
AllMessage readServerData();

void everySecond()
{
    messagesPerPeriod = messagesReceived - messagesAtPeriodStart;
    messagesAtPeriodStart = messagesReceived;
    secondToggle = true;
    return;
}
void everyHalfSecond()
{
  halfSecondToggle = true;
}
void every10thSecond()
{
  tenthSecondToggle = true;
}

Timer timerEvery1000ms(1000, everySecond);
Timer timerEvery500ms(500, everyHalfSecond);
Timer timerEvery100ms(100, every10thSecond);

/////////////////////////////
//         Setup           //
/////////////////////////////
void setup ()
{
	WiFi.setCredentials("ituescon", "iiamnotcon",WPA2);
  WiFi.disconnect();
  WiFi.connect();

	pinMode(messageNotSentLED, OUTPUT);

	Serial1.begin (250000);
	Serial.begin (115200);

	allBoardId = allBoardIdFromBoardId();
  myChannel.messageNotSentLED = D7;
	myChannel.errorEventLED = D6;
	myChannel.rtsPin = rtsPin;
	myChannel.busBusyRetryCount = 4;
  myChannel.outQueueSize = 8;
  myChannel.inQueueSize = 16;
  myChannel.confQueueSize = 8;


	myChannel.begin (allBoardId);
	myChannel.allNet485Enable(busBusyPin);

	myChannel.onlyReadMyMessages=false;
	myChannel.ignoreBoardcasts=false;

	delay(allBoardId*100);

	// I2C
	Wire.setSpeed(CLOCK_SPEED_400KHZ);
	Wire.begin(0x20);
	delay(500);

  // TCP
  server.begin();

	timerEvery100ms.start();
	timerEvery500ms.start();
	timerEvery1000ms.start();

}  // end of setup

void requestEvent() // WIRE Testing
{
  Wire.write("aaaaaaaaaaaaa");         // respond with message of 6 bytes as expected by master
}
String PadMyText(String myText , int newLength)
{
    int myTextLength = myText.length();
    if(myTextLength < newLength )
     {
       for(int i=0; i < (newLength - myTextLength); i++)
       {
         myText = myText + " ";
       }
     }
   return myText;
}
void doThisOncePlease()
{
  int lcdRow;

  doneThisOnce=true;

  AllMessage newMessageIP;
  newMessageIP.ReceiverId = 0x14; // Pepper
  newMessageIP.Type = RS485::MESSAGE_MESSAGE; // Normal message
  newMessageIP.RequiresConfirmation = false;
  lcdRow=1;

  String iPAddress = String(WiFi.localIP()) + "      ";
  iPAddress.toCharArray(lcdTextBuffer, MESSAGE_DATA_SIZE);
  RS485::buildKeyValueDataFromKeyValueInt(newMessageIP.Data,"LCDRow",lcdRow);
  RS485::buildKeyValueDataFromKeyValue(newMessageIP.Data,"Text",lcdTextBuffer);
  myChannel.OutQueueEnqueue(newMessageIP);

  myChannel.allNetUpdate();



  AllMessage newMessageText;
  newMessageText.ReceiverId = 0x14; // Pepper
  newMessageText.Type = RS485::MESSAGE_MESSAGE; // Normal message
  newMessageText.RequiresConfirmation = false;
  lcdRow=0;
  sprintf ( lcdTextBuffer, "%s","Cherry IP           ");
  RS485::buildKeyValueDataFromKeyValueInt(newMessageText.Data,"LCDRow",lcdRow);
  RS485::buildKeyValueDataFromKeyValue(newMessageText.Data,"Text",lcdTextBuffer);
  myChannel.OutQueueEnqueue(newMessageText);
  myChannel.allNetUpdate();



  delay(3000);
}
void prepareSendMessages()
{
  int lcdRow=0;
  //String myText ;
  if(thisTimeThatTime)
  {
    lcdRow = 0;
    int busSpeed = myChannel.getBusSpeed();
    if(busSpeed > 100) {busSpeed= busSpeed-100;}
    sprintf ( lcdTextBuffer, "%1.3f%% %06d %02d ",percentageErrors,messagesReceived,busSpeed);
  }
  else
  {
    lcdRow = 1;
    if(thisTimeThatTime2)
    {
      sprintf ( lcdTextBuffer, "N:%d C:%d B:%d M:%d  ", myChannel.getErrorCountNibble(),myChannel.getErrorCountCRC(),myChannel.getErrorCountOverflow(),messageInBufferIndex);
    }
    else
    {
      sprintf ( lcdTextBuffer, "I:%d O:%d C:%d M:%d ", myChannel.getErrorCountInQueueOverflow(),myChannel.getErrorCountOutQueueOverflow(),myChannel.getErrorCountConfQueueOverflow(),messageInBufferIndex);
    }

    thisTimeThatTime2 = !thisTimeThatTime2;
  }

  thisTimeThatTime = !thisTimeThatTime;

  // Create a new message
  AllMessage newMessage;

  // Build two key values in newMessage.Data
  // myChannel.buildKeyValueDataFromKeyValueInt(newMessage.Data,"AD0",analogRead(A0));
  RS485::buildKeyValueDataFromKeyValueInt(newMessage.Data,"LCDRow",lcdRow);
  RS485::buildKeyValueDataFromKeyValue(newMessage.Data,"Text",lcdTextBuffer);

  newMessage.ReceiverId = 0x14; // Pepper
  newMessage.Type = RS485::MESSAGE_MESSAGE; // Normal message
  newMessage.RequiresConfirmation = false;
  myChannel.OutQueueEnqueue(newMessage);
  messagesReceived ++; // Or we don't include our sent messages in bus performance / speed

}

AllMessage readServerData()
{
  //int bytesRead = client.read(*messageInBuffer, MESSAGE_DATA_SIZE);
  int readByte = client.read();

  if (readByte!=-1)
  {
    messageInBufferIndex++;
    if(messageInBufferIndex>MESSAGE_DATA_SIZE) messageInBufferIndex ==0;
    // Do something with the data
    digitalWrite(myChannel.errorEventLED, !digitalRead(myChannel.errorEventLED));
  }
}

void sendServerData(AllMessage allMessage)
{
  if (client.connected())
  {

    server.write("[");

    //server.write("{\"AllMessage\":");
    server.write("{");

    server.write("\"Id\":");
    server.write(String(allMessage.Id));
    server.write(",");

    server.write("\"Typ\":");
    server.write(String(allMessage.Type));
    server.write(",");

    server.write("\"SId\":");
    server.write(String(allMessage.SenderId));
    server.write(",");

    server.write("\"RId\":");
    server.write(String(allMessage.ReceiverId));
    server.write(",");

    server.write("\"Cnf\":");
    if(allMessage.RequiresConfirmation==true)
    {
      server.write("\"TRUE\"");
    }
    else
    {
      server.write("\"FALSE\"");
    }

    server.write(",");

    server.write("\"Rcv\":");
    server.write(String(allMessage.WhenReceived));
    server.write(",");

    server.write("\"Dat\":\"");
    //server.write("None");
    server.write(allMessage.Data,MESSAGE_DATA_SIZE);
    server.write("\"");
    server.write("}");
    server.write("]");
  }
}

/////////////////////////////
//         Loop            //
/////////////////////////////
void loop ()
 {

  if (!client.connected())
  {
    // if no client is yet connected, check for a new connection
    client = server.available();
  }

  myChannel.allNetUpdate();

  Wire.onRequest(requestEvent); // register event

  if(tenthSecondToggle)
  {
    tenthSecondToggle = false;
    //serverPrintStats();
  }
  if(halfSecondToggle)
  {
    halfSecondToggle = false;
    prepareSendMessages();

  }

  if(secondToggle)
  {

    AllMessage newMessage;
    RS485::buildKeyValueDataFromKeyValueInt(newMessage.Data,"BusSpeed",myChannel.getBusSpeed());
    newMessage.ReceiverId = 0x01; // Apricot
    newMessage.Type = RS485::MESSAGE_MESSAGE;
    newMessage.RequiresConfirmation = false;
    myChannel.OutQueueEnqueue(newMessage);
    secondToggle = false;
  }

  int messageInQueue = myChannel.inQueue.items;

	for (int i = 0; i < messageInQueue; i++)
	{
    myChannel.allNetUpdate();
		AllMessage allMessage;
		allMessage = myChannel.InQueueDequeue();

    sendServerData(allMessage);
    updateStats(allMessage);

  }

  readServerData();

  if(doneThisOnce==false)
  {
    doThisOncePlease();
  }

}  // end of loop



void updateStats(AllMessage allMessage)
{

  if(millis()<1000) return; // Wait for start so we don't hit partial messages when re-booting;
  messagesReceived++;

	/*byte senderId = allMessage.SenderId;
	long thisSeq = allMessage.Id;
	long lastSeq = lastSequence[senderId];

	if(lastSeq +1 != thisSeq && lastSeq!=0)
	{
		sequenceError++;
		myChannel.incrementErrorCount();
	}

	lastSequence[boardId] = thisSeq;*/

	lastPacketReceiveTimeMillis = millis();

	totalErrors = (float) myChannel.getErrorCount() * 1000;
	totalMessages = (float) messagesReceived * 1000;

	percentageErrors = (totalErrors / totalMessages) * 100;

}

void randomMillsecondDelay()
{
    long r = random(200,400);
    delay(r);
}

byte allBoardIdFromBoardId()
{
    String deviceId = System.deviceID();

    if(deviceId.compareTo("230029001647343339383037")==0) return 0x01; // Apricot
    if(deviceId.compareTo("22003b000247343337373738")==0) return 0x02; // Cherry
    if(deviceId.compareTo("19003a001747343339383037")==0) return 0x03; // Apple
    if(deviceId.compareTo("2d0025001747343337363432")==0) return 0x04; // Kiwi
    if(deviceId.compareTo("1e0039001747343339383037")==0) return 0x05; // Pineapple

    return 0xFF;
}

void serialPrintErrors()
{
  if(lastErrors == myChannel.getErrorCount()) return;

  lastErrors = myChannel.getErrorCount();

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
		Serial.println(myChannel.getErrorCount());

	}
}


void serverPrintStatsHIDEME()
{
  if (client.connected())
  {
    server.print("Messages :");
    server.print(messagesReceived);;
    server.print(" Rate :");
    server.print( myChannel.getBusSpeed());
    server.print(" PercentageErrors :");
    server.println( percentageErrors);

  }
}
