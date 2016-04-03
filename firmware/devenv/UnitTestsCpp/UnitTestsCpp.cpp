// UnitTestsCpp.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <string>
#include <algorithm> // for std::swap, use <utility> instead if C++11
#include <iostream>

using std::string;
// Structure to hold the keyValue length and position
struct KeyValueData
{
	int valueStartPosition = 0;
	int valueLength = 0;

};

typedef unsigned char byte;

bool getValueDetailsWithKey(KeyValueData &tempData, const char * data, const char * key);
long getValueLongWithKey(const char * data, const char * key);
string getValueStringWithKey(const char * data, const char * key);
double getValueDoubleWithKey(const char * data, const char * key);
float getValueFloatWithKey(const char * data, const char * key);
int getValueIntWithKey(const char * data, const char * key);

bool buildDataFromKeyValue(char * data, const char * key, const char *value);
bool buildDataFromKeyValueDouble(char * data, const char * key, const double value);
bool buildDataFromKeyValueLong(char * data, const char * key, const long value);
bool buildDataFromKeyValueInt(char * data, const char * key, const int value);

const int MESSAGE_DATA_SIZE = 50;
const int MESSAGE_KEY_SIZE = 12;

int main()
{
	char data[] = "";
	char lcdBuffer[MESSAGE_DATA_SIZE];
	double myDouble = 123.45678;
	float myFloat = 654.456;
	long myLong = 12345678;
	double myInt = 123456;
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE,"%f", myDouble);

	// printf 

	//printf("Characters: %c %c \n", 'a', 65);			- Characters: a A
	//printf("Decimals: %d %ld\n", 1977, 650000L);		- Decimals : 1977 650000
	//printf("Preceding with blanks: %10d \n", 1977);	- Preceding with blanks : 1977
	//printf("Preceding with zeros: %010d \n", 1977);	- Preceding with zeros : 0000001977
	//printf("Width trick: %*d \n", 5, 10);				- Width trick : 10
	//printf("%s \n", "A string");						- A string
	//printf("Some different radices: %d %x %o %#x %#o \n", 100, 100, 100, 100, 100);	- Some different radices : 100 64 144 0x64 0144
	//printf("floats: %4.2f %+.0e %E \n", 3.1416, 3.1416, 3.1416);						- floats : 3.14 + 3e+000 3.141600E+000

	//sprintf(lcdBuffer, "Long=%ld Double=%1.1f%%", myLong, myFloat);

	//buildDataFromKeyValueInt(data, "MyInt", 123456);
	buildDataFromKeyValueInt(data, "LCDRow", 1);
	buildDataFromKeyValue(data, "Text","1234567890123456789012345678901234567890");

	//buildDataFromKeyValueDouble(data, "D", 1235.456);
	//buildDataFromKeyValueLong(data, "Long",74564894);
	//buildDataFromKeyValue(data, "Temp", "45.67");
	
	//{Day=Mday}{D=1235.456000}{Long=74564894}{Int=45345}
	//			1         2         3         4
	//012345678901234567890123456789012345678901234567890
	//{Text=0.00% 0H 485}{MyInt=123456}{LCDRow=1}
	KeyValueData kvd;

	bool yes = getValueDetailsWithKey(kvd, data, "Text");
	int valueInt = getValueIntWithKey(data, "LCDRow");
	int valueInt2 = getValueIntWithKey(data, "MyInt");

	//std::cout << " Data:" << data;

	//keyForValue[16] = "Day";
	//valueForKey[16] = "Monday";
	//buildDataFromKeyValue(data, valueForKey, keyForValue);
	//std::cout << " Data:" << data;

	//byte keyForValue[16] = "Errors";
	//byte valueForKey[16] = "12.45";
	//buildDataFromKeyValue(data, valueForKey, keyForValue);
	//std::cout << " Data:" << data;

	//getValueDetailsWithKey(tempData,data, keyForValue);

	long valueLong = getValueLongWithKey(data, "Long");
	string valueString = getValueStringWithKey(data, "Day");
	double valueDouble = getValueDoubleWithKey(data, "D");
	//float valueFloat = getValueFloatWithKey(data, "Temp");
	
	//std::cout << "Length  :" << tempData.valueLength;
	//std::cout << " Position:" << tempData.valueStartPosition;
	std::cout << " valueInt:" << valueInt;
	std::cout << " valueLong:" << valueLong;
	std::cout << " valueString:" << valueString;
	std::cout << " valueDouble:" << valueDouble;
	//std::cout << " valueFloat:" << valueFloat;
	//std::cout << " Data:" << data;

	getchar();
	return 0;
}

float getValueFloatWithKey(const char * data, const char * key)
{
	KeyValueData funcTempData;
	getValueDetailsWithKey(funcTempData, data, key);
	char buffer[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	return strtof(buffer, nullptr);
}
double getValueDoubleWithKey(const char * data, const char * key)
{
	KeyValueData funcTempData;
	getValueDetailsWithKey(funcTempData, data, key);
	char buffer[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	return strtod(buffer,nullptr);
}
long getValueLongWithKey(const char * data, const char * key)
{
	KeyValueData funcTempData;
	getValueDetailsWithKey(funcTempData, data, key);
	char buffer[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition, funcTempData.valueLength);
	return atol(buffer);

}
int getValueIntWithKey(const char * data, const char * key)
{
	KeyValueData funcTempData;
	getValueDetailsWithKey(funcTempData, data, key);
	char buffer[MESSAGE_DATA_SIZE];
	memcpy(buffer, data + funcTempData.valueStartPosition , funcTempData.valueLength);
	return atoi(buffer);
}
string getValueStringWithKey(const char * data, const char * key)
{
	string retString = "";
	KeyValueData funcTempData;
	getValueDetailsWithKey(funcTempData, data, key);
	for (int i = 0; i < funcTempData.valueLength; i++)
	{
		retString = retString + (char) data[funcTempData.valueStartPosition + i];
	}
	return retString;
}

bool buildDataFromKeyValueInt(char * data, const char * key, const int value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%i", value);
	buildDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}
bool buildDataFromKeyValueLong(char * data, const char * key, const long value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%i", value);
	buildDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}
bool buildDataFromKeyValueDouble(char * data, const char * key, const double value)
{
	char *valueBuffer = new char[MESSAGE_DATA_SIZE];
	int cx = snprintf(valueBuffer, MESSAGE_DATA_SIZE, "%f", value);
	buildDataFromKeyValue(data, key, valueBuffer);
	delete valueBuffer;
	if (cx > 0) return true;
	return false;
}

bool buildDataFromKeyValue(char * data, const char * key ,const char * value)
{
	// Find position of the last value cruly
	int loopTo = 0;
	char * memoryLocationOfData = data; // Gets the actual mem address of the first byte of data
	char * lastPosOfcurlystr = strrchr(data, '}');

	if (lastPosOfcurlystr == 0) lastPosOfcurlystr = memoryLocationOfData;
	int lastValueCurly = lastPosOfcurlystr - memoryLocationOfData;

	//for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	//{
	//	if (data[i] == '}') lastValueCurly = i;
	//}

	int insertPosition = 0;
	if(lastValueCurly > 0 ) insertPosition = lastValueCurly +1;

	data[insertPosition] = '{';
	insertPosition++;
	// Calculate size of insert loop
	loopTo = MESSAGE_DATA_SIZE - insertPosition;

	for (int i = 0; i < loopTo; i++)
	{
		char keyChar = key[i];
		if (keyChar == '\0') break;
		data[insertPosition] = keyChar;
		insertPosition++;
	}
	data[insertPosition] = '=';
	insertPosition++;

	loopTo = MESSAGE_DATA_SIZE - insertPosition;
	for (int i = 0; i < loopTo; i++)
	{
		char valueChar = value[i];
		if (valueChar == '\0') 
			break;

		data[insertPosition] = valueChar;
		insertPosition++;
	}
	data[insertPosition] = '}';
	insertPosition++;
	data[insertPosition] = '\0';

	return true;
}

bool getValueDetailsWithKey(KeyValueData &tempData, const char * data, const char * key)
{
	tempData.valueStartPosition = 0;
	tempData.valueLength = 0;

	bool foundKey = false;
	// Make a temporary buffer of the search string as '{' + key + '='
	char *findKeyBuffer = new char[MESSAGE_DATA_SIZE];
	strcpy(findKeyBuffer, "{");
	strcat(findKeyBuffer, key);
	strcat(findKeyBuffer, "=");

	// Search for the temporary findKeyBuffer in data
	const char* findKeyPtr = strstr(data, findKeyBuffer);
	if (findKeyPtr == nullptr) return false; // Not there - Return false

	// Get the length of the key with syntax
	int KeyLengthIncSyntax = 0;
	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		char myChar = findKeyBuffer[i];
		if (myChar == '=')
		{
			KeyLengthIncSyntax = i + 1;
			break;
		}
	}

	// Get the data details

	void * memoryLocationOfData = (void*)&data[0]; // Gets the actual mem address of the first byte of data
	int keyStartPosition = (findKeyPtr - memoryLocationOfData);
	
	// Find the length the value by looking for the closing }
	tempData.valueStartPosition = keyStartPosition + KeyLengthIncSyntax;
	for (size_t i = 0; i < MESSAGE_DATA_SIZE; i++)
	{
		char myChar = data[i + tempData.valueStartPosition];
		if (myChar == '}')
		{
			tempData.valueLength = i; // i is zero indexed so no need to --i even thou on next char in array
			break;
		}
		
	}

	delete findKeyBuffer;
	return true;
}

bool getValueDetailsWithKeyOld(KeyValueData &tempData, const char * data, const char * key)
{
	bool foundStartTag = false;
	bool foundEndTag = false;
	bool foundEquals = false;
	bool foundKey = false;
	
	int byteCounter = 0;
	int keyByteCounter = 0;
	int valueByteCounter = 0;
	int valueLength = 0;

	tempData.valueStartPosition = -9;

	while (byteCounter < MESSAGE_DATA_SIZE)
	{
		char aByte = data[byteCounter];

		switch (aByte)
		{

		case '{':
			foundStartTag = true;
			byteCounter++;
			break;
		case '}':
			if (foundStartTag == true) foundEndTag = true;
			break;
		case '=':
			if (foundStartTag == true)
			{
				foundEquals = true;
				byteCounter++;
				break;
			}
		default:
			break;
		}
		if (foundStartTag && !foundEquals) // This must be the start of the key name
		{
			char dataByteIs = data[byteCounter];
			char dataP1ByteIs = data[byteCounter + 1];
			char keyByteIs = key[keyByteCounter];

			if (data[byteCounter] != key[keyByteCounter])
			{
				foundStartTag = false;
				foundEndTag = false;
				foundEquals = false;
				keyByteCounter = 0;
				foundKey = false;
			}
			else
			{

				if (data[byteCounter + 1] == '=')
				{
					foundKey = true;
				}
				keyByteCounter++;
			}
		}
		if (foundEquals && !foundEndTag) // we are in the data
		{
			tempData.valueLength++;
			if (tempData.valueStartPosition == -9) tempData.valueStartPosition = byteCounter;
		}

		byteCounter++;
	}
	return foundKey;
}