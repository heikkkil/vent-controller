/*
 * FanController.cpp
 *
 *  Created on: 10 Mar 2020
 *      Author: Eelis
 */

#include "FanController.h"
#include "I2C.h"

/**
*@brief								Interface for controlling the fan. 
*@param conf					: 	Configuration object.
*@param initialTargetPressure	:	Default target pressure value for auto mode
*@param initialFanSpeed			:	Default fan speed for manual mode
*/

FanController::FanController(I2C_config conf, int initialTargetPressure = 80, int initialFanSpeed = 10000) : fan(2), pressureSensor(SENSOR_ADDRESS, conf) {
	fanSpeed = initialFanSpeed;
	targetPressure = initialTargetPressure;
	autoOutOfLimits = 0;
	mode = AUTO;
}

FanController::~FanController() {
	// TODO Auto-generated destructor stub
}

void FanController::run() {
	pressure = pressureSensor.getPressure();
	//For some reason (int overlow?), when pressure is close to 0, it sometimes spikes to ~250
	//normally pressure never gets over 200 so make all of those to 0
	if (mode == AUTO) {
		automatic();
	} else {
		manual();
	}
	printf("fanspeed %d pressure %d target %d", fanSpeed, pressure, targetPressure);
	printf("\n");
}

void FanController::automatic() {
	int comp = targetPressure - pressure;

	if (comp <= 2 && comp >= -2) {
		autoOutOfLimits = 0;
	} else {
		++autoOutOfLimits;
	}

	if (autoOutOfLimits >= 2) {
		if (comp <= 10 && comp >= -10) {
			if (comp < 0) {
				fanSpeed -= SMALL_FAN_ADJUST;
			} else {
				fanSpeed += SMALL_FAN_ADJUST;
			}
		} else if (comp >= 25 || comp <= -25) {
			if (comp >= 25) {
				fanSpeed += BIG_FAN_ADJUST;
			} else {
				fanSpeed -= BIG_FAN_ADJUST;
			}
		} else {
			if (comp < 0) {
				fanSpeed -= MEDIUM_FAN_ADJUST;
			} else {
				fanSpeed += MEDIUM_FAN_ADJUST;
			}
		}
	}

	if (fanSpeed > 20000) {
		fanSpeed = 20000;
	} else if (fanSpeed < 0) {
		fanSpeed = 0;
	}

	fan.setNonRelativeFrequency(fanSpeed);
}

void FanController::manual() {
	fan.setNonRelativeFrequency(fanSpeed);
}


/**
*@brief 		Sets a target pressure for automatic mode
*@param	target 	: Target pressure	
*/

void FanController::setTargetPressure(int target) {
	if (target >= 0 && target <= 255) {
		targetPressure = target;
	}
}

/**
*@brief 		Sets a fixed speed for fan
*@param speed 	: Target speed
*/

void FanController::setFanSpeed(int speed) {
	if (mode == MANUAL) {
		fanSpeed = 200*speed;
	}
}


/**
*@brief			Set mode to automatic or manual
*@param m		: Boolean value for mode. True for automatic, false for manual
*/

void FanController::setMode(bool m) {
	mode = m;
	autoOutOfLimits = 0;
}


/**
*@brief			Getter for current pressure.
*@return		Current pressure
*/
int FanController::getPressure() {
	return pressure;
}
/**
*@brief			Getter for fan speed
*@return		Current fan speed	
*/

int FanController::getFanSpeed() {
	return fanSpeed / 200;
}


/**
*@brief			Getter for target pressure
*@return		Current target pressure
*/

int FanController::getTargetPressure() {
	return targetPressure;
}


/**
*@brief			Getter for current mode
*@return 		Current mode
*/

bool FanController::getMode() {
	return mode;
}

/**
*@brief			Checks is target pressure is reachable
*@return		True for reachable
*/

bool FanController::isPressureReachable() {
	return (autoOutOfLimits < NOT_REACHED_LIMIT);
}
