/*

AllDevice for devices that use the AllNet485

 Version P1.1 - By TopBanana 27-03-2016

*/

// Particle boards have a PLATFORM_ID Defined but Arduino boards don't. Below includes relevant header.
#ifdef PLATFORM_ID
#include "application.h" // This is Particle.io specific - Would have been arduino.h for Arduino
#else
#include "Arduino.h"
#endif
#include "AllQueue.h"

#ifndef _ALLNETDEVICE
#define _ALLNETDEVICE

class AllNetDevice
{
	public:
	
	byte BoardId = 0xFF;	// Id of board the device belongs to
	byte Code = 0xFF;		// Unique code for this device

	byte BoardIdLinked = 0xFF; // Board Id of the device linked to this
	byte CodeLinked = 0xFF;		 // Code of the device linked to this device
	
	byte Type;	// Type of device
	
	
	/*
	
	Important to realize the difference between a device and a board. A board hosts the device so PIN D1 is a device on a board. But just to 
	confuse things the board has a virtual device that is the board itself. This is used to perform actions or read data about the board like booting or errors
	
	Devices don't get sent over the bus. The data for or about the device gets sent. With the exception of create device the data for which is in the message to the board device.

	Setting up devices pins and config that aren't simple GPIO are the responsibility of the board firmware. So some of the device types are for informative purposes.
	
	*/
	
	enum
	{
		DEVICE_TYPE_BOARD ,		// The board itself
		DEVICE_TYPE_NULL ,		// Null device
		DEVICE_TYPE_ADC ,		// Analogue to digital converter
		DEVICE_TYPE_DIG ,		// Digital IO
		DEVICE_TYPE_PWM ,		// PWM Pulse wave modulation
		DEVICE_TYPE_RELAY ,		// Relay

		DEVICE_TYPE_SWITCH ,	// Switch that keeps it state - ON / OFF
		DEVICE_TYPE_BUTTON ,	// Button that has temporary state of ON when pushed
		DEVICE_TYPE_BUTTONX ,	// Button that has temporary state of OFF when pushed

		DEVICE_TYPE_COUNTER ,	// Pulse counter

		DEVICE_TYPE_SENSOR_TEMPERATURE ,	// Temperature sensor
		DEVICE_TYPE_SENSOR_MOVEMENT ,		// Movement sensor
		DEVICE_TYPE_SENSOR_HUMIDITY ,		// Humidity sensor
		DEVICE_TYPE_SENSOR_LIGHT ,			// Light sensor
		DEVICE_TYPE_SENSOR_IRADIATION ,	// Irradiation / solar sensor
		DEVICE_TYPE_SENSOR_CURRENT ,		// Current sensor / shunt for measuring power
		
		DEVICE_TYPE_DISPLAY_LCD ,			// Standard LCD Library display
		DEVICE_TYPE_DISPLAY_LED ,			// LED Segmented digital display		

		DEVICE_STATE_BOOTING,		// Device / board is or just has booted
		DEVICE_STATE_OK,			// Device is ok
		DEVICE_STATE_LOST,			// Device is no longer present but should be
		DEVICE_STATE_CHANGED,		// It's value / measurement etc has changed
		DEVICE_STATE_ERROR,			// An error has occurred

		DEVICE_ACTION_NONE,			// Nothing required of the device
		DEVICE_ACTION_GETVALUE,		// Get a value from the device
		DEVICE_ACTION_GETDATA,		// Get any data associated with this device
		DEVICE_ACTION_SETVALUE,		// Set the value of this device
		DEVICE_ACTION_SETDATA,		// Set the data for this device
		
		DEVICE_ACTION_CREATE,		// Creates a temporary device with the received data that only exists when board alive 
		DEVICE_ACTION_UPDATE,		// Updates a device properties with the received data
		DEVICE_ACTION_DELETE,		// Deletes a temporary device - Hard coded devices can't be deleted

		DEVICE_ACTION_REBOOT,		// Reboots a board. Not easily possible on Arduinos
		DEVICE_ACTION_RADIO_OFF,	// If possible turns OFF the radio on a device
		DEVICE_ACTION_RADIO_ON,		// If possible turns ON the radio on a device
		DEVICE_ACTION_POWER_LOW,	// Go into low power mode if possible
		DEVICE_ACTION_POWER_NORMAL	// Come out of low power mode 
		
	};
	
	bool Enable();	// Enables the device and sets any required pin modes. Pointer given to access device's properties
	bool Disable(); // Disables the device
	bool Robust();  // When this device sends any data in a message the message must be confirmed by receiving device  (messageRequiresConfirmation)
	
	bool StateChange(byte newState); // Changes the devices state if allowed / possible
	byte GetState(); // Returns the device's current state
	byte GetLastState(); // Returns the device's current state
	bool StateAndLastTheSame (); // Returns true/false by comparing last and current state
	unsigned long StateLastChangedMillis(); // When this device last changed it's current state / status
	
	bool DoActions(); // Performs required actions on this device by calling given function pointer
	byte GetAtionLast(); // Get the last action that was performed on this device by DoActions
	void Process(); // Processes any tasks or functions that are required to update / change / etc devices 
	
	bool SetPin(int pin); // Sets the pin for this device if allowed (It might already be in use elsewhere)

	int Period=0; // PWM Parameter / Switch debounce / Tolerance
	int Frequency=0; // Update frequency (0 = doesn't broadcast or send any changes) any other value is min time in millis between sending data
		
	private:
	
	bool Enabled = true;
	byte State;	// Current state of device
	byte StateLast;	// Last state of device
	
	byte Action;		// Current action to do with device
	byte ActionLast;	// Last action performed on this device
	int Pin;			// Board pin number associated with this device - Not for devices that use several pins	but for single GPIO pin based devices etc 

	unsigned long SentLast = 0; // Millis() of the last time it's values were sent on the bus

};


#endif // !_ALLNETDEVICE