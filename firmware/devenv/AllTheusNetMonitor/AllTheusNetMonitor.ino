// This #include statement was automatically added by the Particle IDE.
#include "RS485NB.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMicroOLED.h"

//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET D7  // Connect RST to pin 7 (req. for SPI and I2C)
#define PIN_DC    D6  // Connect DC to pin 6 (required for SPI)
#define PIN_CS    A2 // Connect CS to pin A2 (required for SPI)

//////////////////////////////////
// MicroOLED Object Declaration //
//////////////////////////////////
// Declare a MicroOLED object. The parameters include:
// 1 - Mode: Should be either MODE_SPI or MODE_I2C
// 2 - Reset pin: Any digital pin
// 3 - D/C pin: Any digital pin (SPI mode only)
// 4 - CS pin: Any digital pin (SPI mode only, 10 recommended)
MicroOLED oled(MODE_SPI, PIN_RESET, PIN_DC, PIN_CS);


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
RS485 myChannel (fRead, fAvailable, fWrite,fWait,MESSAGE_DATA_SIZE + MESSAGE_HEADER_SIZE+50);

int messageNotSentLED = D2;
int rtsPin = D1;
int busBusyPin = D3;

byte msg [MESSAGE_DATA_SIZE] = "ok Basil?";
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

int samplePeriodForMessagesPerPeriod = 6000;
long lastMillis = 0;

bool secondToggle = false;
bool halfSecondToggle = false;
bool tenthSecondToggle = false;

void updateStats(AllMessage allMessage);

void everySecond()
{
  messagesPerPeriod = messagesReceived - messagesAtPeriodStart;
  messagesAtPeriodStart = messagesReceived;
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

Timer timerEvery1000ms(1000, everySecond);
Timer timerEvery500ms(500, everyHalfSecond);
Timer timerEvery100ms(100, every10thSecond);



void setup ()
{
  WiFi.setCredentials("Moon", "takeme2the");

  oled.begin();    // Initialize the OLED
  oled.clear(ALL); // Clear the display's internal memory
  oled.display();  // Display what's in the buffer (splashscreen)
  delay(1000);     // Delay 1000 ms

  oled.clear(PAGE);            // Clear the display
  oled.setCursor(0, 0);        // Set cursor to top-left
  oled.setFontType(0);         // Smallest font
  oled.print("Kiwi 4B");
  oled.display();
  delay(1000);
  oled.clear(PAGE);

  pinMode(messageNotSentLED, OUTPUT);

  Serial1.begin (250000);
  Serial.begin (115200);

  allBoardId = allBoardIdFromBoardId();

  myChannel.rtsPin = rtsPin;
  myChannel.busBusyRetryCount = 1;
  myChannel.begin (allBoardId);
  myChannel.allNet485Enable(busBusyPin);
  //myChannel.debugEnable(); DONT!

	myChannel.onlyReadMyMessages=false;
	myChannel.ignoreBoardcasts=false;

  delay(allBoardId*100);

  timerEvery100ms.start();
  timerEvery500ms.start();
  timerEvery1000ms.start();

}  // end of setup

void loop ()
{

  if(tenthSecondToggle)
  {
    tenthSecondToggle = false;
  }
  if(halfSecondToggle)
  {
    halfSecondToggle = false;
    oledPrintStats();
  }
  if(secondToggle)
  {
    secondToggle = false;
  }

  myChannel.update();

  int messageInQueue = myChannel.inQueue.items;

	for (int i = 0; i < messageInQueue; i++)
	{

      myChannel.update();
  		AllMessage allMessage;
  		allMessage = myChannel.InQueueDequeue();
      myChannel.update();
      updateStats(allMessage);
      Serial.print("Message from:");
      Serial.println(allMessage.SenderId);
  }

}  // end of loop

void updateStats(AllMessage allMessage)
{
  messagesReceived++;

	byte senderId = allMessage.SenderId;
	long thisSeq = allMessage.Id;
	long lastSeq = lastSequence[senderId];

	if(lastSeq +1 != thisSeq && lastSeq!=0)
	{
		sequenceError++;
		myChannel.incrementErrorCount();
	}

	lastSequence[boardId] = thisSeq;
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
void sendMessage()
{
	byte destinationId = 0x11;
	byte messageType = 0x01;

	String myText = String(percentageErrors,2) + "% N:"+ String(myChannel.getErrorCountNibble()) + "/" + String(messagesReceived)+"  ";
	myText.getBytes(msg,myText.length()+1);

	if(myChannel.sendMsg (msg, MESSAGE_DATA_SIZE , destinationId,messageType,false))
	{
		// It worked
		digitalWrite(messageNotSentLED, LOW);
	}
	else
	{
    // No worky!
		digitalWrite(messageNotSentLED, HIGH);
	}
}

void oledPrintStats()
{
  oled.setCursor(0, 0);        // Set cursor to top-left
  oled.setFontType(0);         // Smallest font

  oled.print("Hz:");
  oled.print(messagesPerPeriod);
  oled.print("   ");

  oled.setCursor(0, 8);
  oled.print("Me:");
  oled.print(messagesReceived);

  oled.setCursor(0, 16);
  oled.print("Er:");
  oled.print(myChannel.getErrorCount());

  oled.setCursor(0, 24);
  oled.print("Er:");
  oled.print(percentageErrors);
  oled.print("%");

  oled.setCursor(0, 32);
  oled.print("Ni:");
  oled.print(myChannel.getErrorCountNibble());

  oled.setCursor(0, 40);
  oled.print("Cr:");
  oled.print(myChannel.getErrorCountCRC());

  oled.print("Sq:");
  oled.print(sequenceError);

  oled.display();
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
