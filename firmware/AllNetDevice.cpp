/*

AllDevice for devices that use the AllNet485

 Version P1.1 - By TopBanana 27-03-2016

*/

#include "AllNetDevice.h"

// Enables the device and sets any required pin modes. Pointer given to access device's properties
bool AllNetDevice::Enable() {}	

// Disables the device
bool AllNetDevice::Disable() {}

// When this device sends any data in a message the message must be confirmed by receiving device  (messageRequiresConfirmation)
bool AllNetDevice::Robust() {}

// Changes the devices state if allowed / possible
bool AllNetDevice::StateChange(byte newState) {}

// Returns the device's current state
byte AllNetDevice::GetState() {}

// Returns the device's current state
byte AllNetDevice::GetLastState() {}

// Returns true/false by comparing last and current state
bool AllNetDevice::StateAndLastTheSame() {}	

// When this device last changed it's current state / status
unsigned long AllNetDevice::StateLastChangedMillis(){}

// Performs required actions on this device by calling given function pointer
bool AllNetDevice::DoActions() {}

// Get the last action that was performed on this device by DoActions
byte AllNetDevice::GetAtionLast() {}

// Processes any tasks or functions that are required to update / change / etc devices 
void AllNetDevice::Process(AllMessage allMessage) 
{
	if (allMessage.Type == RS485::MESSAGE_MESSAGE)
	{
		// Not much to do here user routines are responsible for pulling messages off the in_queue and acting accordingly
		return;
	}

	if (allMessage.Type == RS485::MESSAGE_DEVICE)
	{
		if (receiveDataFunction != nullptr) // If the device has a function pointer set then call it
		{
			bool result;
			result = receiveDataFunction(allMessage.Data); // Called function is responsible for getting data from data
		}

		else // Simple GPIO based set / get actions
		
		{
			int action ;
			char pinType ;
			int  pin ;
			int value = 0;
			AllMessage newMessage;

			switch (action)
			{
			case DEVICE_ACTION_GETVALUE: // Get the value from GPIO
				
				pinType = RS485::getKeyValueCharWithKey(allMessage.Data, "PinType");
				pin = RS485::getKeyValueIntWithKey(allMessage.Data, "Pin");
				value = 0;

				if (pinType == 'D')
				{
					pinMode(pin, INPUT);
					value = digitalRead(pin);
				}
				if (pinType == 'A')
				{
					value = analogRead(pin);
				}
				
				// Send a message back with the data

				newMessage.SenderId = allMessage.ReceiverId;
				newMessage.ReceiverId = allMessage.SenderId;
				newMessage.RequiresConfirmation = 0; // Must be this - Answers to get don't require confirmations
				newMessage.Type = RS485::MESSAGE_DEVICE;

				RS485::buildKeyValueDataFromKeyValueInt(newMessage.Data, "Value", value);
				RS485::buildKeyValueDataFromKeyValueInt(newMessage.Data, "Pin", pin);
				RS485::buildKeyValueDataFromKeyValueChar(newMessage.Data, "PinType", pinType);


				break;

			case DEVICE_ACTION_SETVALUE: // Set the value from GPIO

				value = RS485::getKeyValueIntWithKey(allMessage.Data, "Value");
				pinType = RS485::getKeyValueCharWithKey(allMessage.Data, "PinType");
				pin = RS485::getKeyValueIntWithKey(allMessage.Data, "Pin");

				if (pinType == 'D')
				{
					pinMode(pin, OUTPUT);
					digitalWrite(pin,value);
				}
				if (pinType == 'A')
				{
					// Nothing here yet
				}


				break;

			default:
				break;
			}
		}
	}


	return;
}

bool AllNetDevice::SetPin(int pin) {}	 // Sets the pin for this device if allowed (It might already be in use elsewhere
