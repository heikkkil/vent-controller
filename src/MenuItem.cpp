/*
 * MenuItem.cpp
 *
 *  Created on: 1.2.2016
 *      Author: krl
 */

#include "MenuItem.h"


/**
*@param property		: Pointer to a baseclass of an editable value.
*@param fancon			: pointer to a fancontroller object.
*/
MenuItem::MenuItem(PropertyEdit *property, FanController *fancon): pe(property), fcon(fancon) {

}

MenuItem::~MenuItem() {

}


/**
*@brief 	Handles events.
*@param e	: Event type.
*@return	Returns true if item has handled the event. 
*/

bool MenuItem::event(menuEvent e) {
	bool handled = true;
	switch(e) {
	case ok:
		if(pe->getFocus()) {
			pe->accept();
			pe->setFocus(false);
		}
		else {
			pe->setFocus(true);
		}
		break;
	case back:
		if(pe->getFocus()) {
			pe->cancel();
			pe->setFocus(false);
		}
		else {
			handled = false;
		}

		break;
	case show:
		break;
	case up:
		if(pe->getFocus()) pe->increment();
		else handled = false;
		break;
	case down:
		if(pe->getFocus()) pe->decrement();
		else handled = false;
		break;
	}
	if(handled) pe->display();

	return handled;
}

