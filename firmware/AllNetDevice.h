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
	
	enum
	{
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
		
		
		
	};  // end Devoice types
	
	bool Enable();
	bool Disable();
	
	private:
	
	bool Enabled = true;

	byte BoardId = 0xFF;	// Id of board the device belongs to
	byte Code = 0xFF;		// Unique code for this device

	byte BoardIdLinked = 0xFF; // Board Id of the device linked to this
	byte CodeLinked = 0xFF;		 // Code of the device linked to this device
	
	byte Type;	// Type of device
	
	
	
	




};