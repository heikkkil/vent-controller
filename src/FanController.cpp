/*
 * FanController.cpp
 *
 *  Created on: 10 Mar 2020
 *      Author: Eelis
 */

#include "FanController.h"
#include "I2C.h"

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

void FanController::setTargetPressure(int target) {
	if (target >= 0 && target <= 255) {
		targetPressure = target;
	}
}

void FanController::setFanSpeed(int speed) {
	if (mode == MANUAL) {
		fanSpeed = 200*speed;
	}
}

void FanController::setMode(bool m) {
	mode = m;
	autoOutOfLimits = 0;
}

int FanController::getPressure() {
	return pressure;
}

int FanController::getFanSpeed() {
	return fanSpeed / 200;
}

int FanController::getTargetPressure() {
	return targetPressure;
}


bool FanController::getMode() {
	return mode;
}

bool FanController::isPressureReachable() {
	return (autoOutOfLimits < NOT_REACHED_LIMIT);
}
