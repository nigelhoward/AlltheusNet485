/*
 * PointerTests02.ino
 *
 * Created: 3/28/2016 10:01:44 AM
 * Author: nigel
 */ 

class twoInts
{ public:
	int a;
	int b;
	byte something[5] = {1,2,3,4,5};
};

typedef bool (*functionToRun)(twoInts myTwoInts);

bool runFunction(twoInts &myTwoInts,  functionToRun runFunc);
bool aGreatb(twoInts myTwoInts);
bool bGreata(twoInts myTwoInts);
void PrintTwoInts(twoInts &theTwoInts);


void setup()
{

Serial.begin(250000);
	  /* add setup code here, setup code runs once when the processor starts */
}

void loop()
{

bool result;

int mya(4);
int myb(6);

twoInts myTwoInts;

myTwoInts.a = 4;
myTwoInts.b = 6;

Serial.print("In function Loop - ");PrintTwoInts(myTwoInts);PrintTwoInts(myTwoInts);



result = runFunction(myTwoInts, bGreata);

if(result) Serial.println("Comparison is true");
if(!result) Serial.println("Comparison is false");



while(0==0){}
}


bool runFunction(twoInts &myTwoInts, functionToRun runFunc)
{
	bool result = runFunc(myTwoInts);
}

bool aGreatb(twoInts myTwoInts)
{
	
	Serial.print("In function aGreatb - ");PrintTwoInts(myTwoInts);	
	if(myTwoInts.a > myTwoInts.b) return true;
	return false;
}


bool bGreata(twoInts myTwoInts)
{
    Serial.print("In function bGreata - ");PrintTwoInts(myTwoInts);	
	if(myTwoInts.a < myTwoInts.b) return true;
	return false;
}

void PrintTwoInts(twoInts &theTwoInts)
{
	Serial.print("a=");
	Serial.print(theTwoInts.a);
	
	Serial.print(" b=");
	Serial.println(theTwoInts.b);
}
