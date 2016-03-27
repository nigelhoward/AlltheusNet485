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
		DEVICE_TYPE_SENSOR_IRRADIATION ,	// Irradiation / solar sensor
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
		DEVICE_ACTION_DELETE		// Deletes a temporary device - Hard coded devices can't be deleted
		
		};
	
	bool Enable();	// Enables the device and sets any required pin modes
	bool Disable(); // Disables the device
	bool StateChange(byte newState); // Changes the devices state if allowed / possible
	byte GetState(); // Returns the device's current state
	
	
	private:
	
	bool Enabled = true;
	byte State;	// Type of device
	
	
	
	




};