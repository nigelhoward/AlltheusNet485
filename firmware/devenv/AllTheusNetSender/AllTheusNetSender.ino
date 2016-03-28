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

const int MESSAGE_SIZE = 50;

RS485 myChannel (fRead, fAvailable, fWrite,fWait, MESSAGE_SIZE+10);

int messageNotSentLED = D7;
int rtsPin = D1;
int busBusyPin = D3;

byte msg [MESSAGE_SIZE] = "AABBCCDDEE";
byte *ptrmsg = msg;
byte allBoardId=0x00;

int sequence = 0;

bool secondToggle = false;
bool halfSecondToggle = false;
bool tenthSecondToggle = false;
bool thisTimeThatTime = false;

Timer timerEvery1000ms(1000, everySecond);
Timer timerEvery500ms(500, everyHalfSecond);
Timer timerEvery100ms(100, every10thSecond);

void everySecond()
{
    secondToggle = true;
}
void everyHalfSecond()
{
  halfSecondToggle = true;
}
void every10thSecond()
{
  tenthSecondToggle = true;
}


void setup ()
{

  Serial1.begin (250000);
  Serial.begin (115200);

  allBoardId = allBoardIdFromBoardId();

  myChannel.messageNotSentLED = D7;
  myChannel.errorEventLED = D6;
  myChannel.rtsPin = rtsPin;
  myChannel.busBusyRetryCount = 4;
  myChannel.onlyReadMyMessages = true;
  myChannel.ignoreBoardcasts = true;
	myChannel.debugErrorsToSerial = true;
  myChannel.outQueueSize = 4;
  myChannel.inQueueSize = 8;
  myChannel.confQueueSize = 2;

  myChannel.begin (allBoardId);
  myChannel.allNet485Enable(busBusyPin);

  delay(allBoardId*100);

  timerEvery100ms.start();
  timerEvery500ms.start();
  timerEvery1000ms.start();

}  // end of setup

void busMessage(String myText)
{
	AllMessage newMessage;
	myText = String(allBoardId) + " says Hello    ";

	myText.getBytes(newMessage.Data,myText.length()+1);
	newMessage.ReceiverId = 0xFF; // Everyone
	newMessage.Type = RS485::MESSAGE_BOARDCAST; // For everyone to see
	newMessage.RequiresConfirmation = false;
	myChannel.OutQueueEnqueue(newMessage);
}

void loop ()
{
  myChannel.allNetUpdate();

  if(tenthSecondToggle)
  {
    busMessage("");
    tenthSecondToggle = false;
  }

  if(halfSecondToggle)
  {
    halfSecondToggle = false;
  }

  if(secondToggle)
  {
    secondToggle = false;
  }


}  // end of loop


void randomMillsecondDelay()
{
    long r = random(150,250);
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
