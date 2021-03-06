/*
 * Fan.cpp
 *
 *  Created on: Mar 6, 2020
 *      Author: krl / jere
 */

#include "Fan.h"


/**
 *@brief Logical control unit for fan.
 *
 *@param _node : Slave id
 *
 *
 */

Fan::Fan(int _node):node(_node),Speed(&node, 101), ControlWord(&node,0),
	StatusWord(&node, 3),OutputFrequency(&node,102),Current(&node,103) ,Frequency(&node, 1)
{
	node.begin(9600);

	// need to use explicit conversion since printf's variable argument doesn't automatically convert this to an integer
	printf("Status=%04X\n", (int)StatusWord); // for debugging

	ControlWord = 0x0406; // prepare for starting

	printf("Status=%04X\n", (int)StatusWord); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	//printf("Status=%04X\n", (int)StatusWord); // for debugging

	ControlWord = 0x047F; // set drive to start mode

	printf("Status=%04X\n", (int)StatusWord); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printf("Status=%04X\n", (int)StatusWord); // for debugging
}

Fan::~Fan() {
	// TODO Auto-generated destructor stub
}



/**
 * @brief 	Set target frequency for fan
 * @param 	speed : Relative speed value (0-100)
 * @return 	Boolean where true means the set speed was reached
 */
bool Fan::setFrequency(uint16_t speed){
	int result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	// Convert relative speed value(0-100) to reference frequency value (0-200)
	int freq = speed * 200 ;

	if(freq > 20000)
		freq = 20000;



	Frequency = freq; // set motor frequency

//	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;

	do {
		Sleep(delay);
		// read status word
		result = StatusWord;
		// check if we are at setpoint
		if (result >= 0 && (result & 0x0100)) atSetpoint = true;
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}


/**
 * @brief 	Set target frequency for fan
 * @param 	speed : Relative speed value (0-100)
 * @return 	Boolean where true means the set speed was reached
 */
bool Fan::setFrequency(uint16_t speed){
	int result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;


	// Convert relative speed value(0-100) to reference frequency value (0-200)
	int freq = speed * 200 ;

	if(freq > 20000)
		freq = 20000;




	Frequency = freq; // set motor frequency

//	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;

	do {
		Sleep(delay);
		// read status word
		result = StatusWord;
		// check if we are at setpoint
		if (result >= 0 && (result & 0x0100)) atSetpoint = true;
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}



/**
 * @brief 	Set target frequency for fan
 * @param 	speed : Speed value (0-20000)
 * @return 	Boolean where true means the set speed was reached
 */
bool Fan::setNonRelativeFrequency(int freq){
	int result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	if(freq > 20000)
		freq = 20000;

	Frequency = freq; // set motor frequency

//	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;

	do {
		Sleep(delay);
		// read status word
		result = StatusWord;
		// check if we are at setpoint
		if (result >= 0 && (result & 0x0100)) atSetpoint = true;
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}

