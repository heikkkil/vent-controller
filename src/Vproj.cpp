/*
===============================================================================
 Name        : main.c
 Author      : $(Jere, Heikki, Eelis)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif
#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
#define EVENT_BUFFER_SIZE 10
#define BOUNCER_LIMIT 100
#define BACK_TIME_LIMIT 5000
#define REFRESH_PERIOD 500
#define OK_PRESSED 1
#define OK_NOT_PRESSED 0

#include <ring_buffer.h>
#include <memory>
#include <atomic>

#include "LiquidCrystal.h"
#include "SimpleMenu.h"
#include "IntegerEdit.h"
#include "FanController.h"
#include "DigitalIoPin.h"
#include "Fan.h"
#include "ModbusMaster.h"
#include "ModbusRegister.h"
#include "LpcUart.h"
#include "RingBuffer.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
static volatile std::atomic_int counter;
static volatile uint32_t systicks;
static volatile uint32_t prev_systicks;
static volatile uint32_t refresh_counter;
static volatile uint8_t ok_pressed;


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

 RingBuffer e_Ring(10);

*****************************************************************************
 * Private functions
 ****************************************************************************/
static void setup_Pin_Interrupts(){

	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 0,
							 (IOCON_DIGMODE_EN |(0x2<<3) |IOCON_MODE_INACT) );
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 3,
							 (IOCON_DIGMODE_EN |(0x2<<3) |IOCON_MODE_INACT) );
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 9,
							 (IOCON_DIGMODE_EN |(0x2<<3)| IOCON_MODE_INACT) );
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10,
							 (IOCON_DIGMODE_EN |(0x2<<3)| IOCON_MODE_INACT) );

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 0);

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 3);

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 9);

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 10);


	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PININT);

	/* Reset the PININT block */
	Chip_SYSCTL_PeriphReset(RESET_PININT);

	//Link Interrupt channels to physical pins
	Chip_INMUX_PinIntSel(0, 0, 0);
	Chip_INMUX_PinIntSel(1, 1, 3);
	Chip_INMUX_PinIntSel(2, 0, 10);
	Chip_INMUX_PinIntSel(3, 0, 9);

	//Clear interrupt status of pins
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(0));

	//Clear interrupt status of pins
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(1));


 //Setup pin block and switches to send interrupts properly
static void setup_Pin_Interrupts(){

	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	DigitalIoPin SW1(0,0,true,true,true);
	DigitalIoPin SW2(1,3,true,true,true);
	DigitalIoPin SW3(0,9,true,true,true);
	DigitalIoPin SW4(0,10,true,true,true);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PININT);

	/* Reset the PININT block */
	Chip_SYSCTL_PeriphReset(RESET_PININT);

	//Link Interrupt channels to physical pins
	Chip_INMUX_PinIntSel(0, 0, 0);
	Chip_INMUX_PinIntSel(1, 1, 3);
	Chip_INMUX_PinIntSel(2, 0, 10);
	Chip_INMUX_PinIntSel(3, 0, 9);

	//Clear interrupt status of pins
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(0)| PININTCH(1) | PININTCH(2) | PININTCH(3));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(0)| PININTCH(1) | PININTCH(2) | PININTCH(3));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(0)| PININTCH(1) | PININTCH(2) | PININTCH(3));
	
	//Enable IRQ's for pins
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT3_IRQn);
	NVIC_EnableIRQ(PIN_INT3_IRQn);

}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void SysTick_Handler(void)
{
	systicks++;
	if(counter > 0) counter--;
	if(ok_pressed == OK_PRESSED) {
		if (systicks - prev_systicks >= BACK_TIME_LIMIT	) {
			e_Ring.add(MenuItem::back);
			ok_pressed = OK_NOT_PRESSED;
		}
	}

	if(refresh_counter <= systicks ){
		e_Ring.add(MenuItem::show);
		refresh_counter = systicks + REFRESH_PERIOD;
	}
}

void PIN_INT0_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	if(systicks - prev_systicks >= BOUNCER_LIMIT ){
		e_Ring.add(MenuItem::up);
	}
	prev_systicks = systicks;
}

void PIN_INT1_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	if(systicks - prev_systicks >= BOUNCER_LIMIT ){
		e_Ring.add(MenuItem::ok);
	}
	prev_systicks = systicks;
	ok_pressed = OK_PRESSED;

}

void PIN_INT2_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	if(systicks - prev_systicks >= BOUNCER_LIMIT ){
		e_Ring.add(MenuItem::down);
	}
	prev_systicks = systicks;
}

void PIN_INT3_IRQHandler(void){

Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(3));
	if(systicks - prev_systicks >= BOUNCER_LIMIT ){
		e_Ring.add(MenuItem::back);
	}
	prev_systicks = systicks;
	ok_pressed = OK_NOT_PRESSED;
}


#ifdef __cplusplus
}
#endif


/**
 * @brief 	Processes events in the eventbuffer.

 * @param	menu : Menuobject that processes the events.
 */
void processEvents(SimpleMenu& menu){

	while(!e_Ring.IsEmpty()){
		enum MenuItem::menuEvent event;
		event = e_Ring.Pop();

		menu.event(event);
	}
}

/**
*@brief Program waits for a specified time amount
*@param ms : How many millisecons to sleep for
*/
void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

/* this function is required by the modbus library */
uint32_t millis() {
	return systicks;
}


/**
 * @brief	Main UART program body
 * @return	Always returns 1
 */
int main(void)
{

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
#endif
#endif

	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / 1000);

	/* Set up SWO to PIO1_2 */
	Chip_SWM_MovablePortPinAssign(SWM_SWO_O, 1, 2); // Needed for SWO printf


	//Setup FanController
	I2C_config conf;
	FanController controller(conf, 50, 80);

	//Setup pin interrupts for 4 digitalIoPins
	setup_Pin_Interrupts();

	//Initialize Pins for LCD screen
	DigitalIoPin rs(0,8,false,false,false);
	DigitalIoPin en(1,6,false,false,false);
	DigitalIoPin d4(1,8,false,false,false);
	DigitalIoPin d5(0,5,false,false,false);
	DigitalIoPin d6(0,6,false,false,false);
	DigitalIoPin d7(0,7,false,false,false);


	Chip_RIT_Init(LPC_RITIMER);
	//NVIC_EnableIRQ(RITIMER_IRQn);

	//Initialize lcd screen
	LiquidCrystal *lcd = new LiquidCrystal(&rs,&en,&d4,&d5,&d6,&d7);
	lcd->setCursor(0,0);

	SimpleMenu menu;
	IntegerEdit* mode_auto = new IntegerEdit(lcd, &controller, std::string("Auto  "),0,130);
	IntegerEdit* mode_man =  new IntegerEdit(lcd, &controller, std::string("Manual"),0,100);
	menu.addItem(new MenuItem(mode_auto, &controller));
	menu.addItem(new MenuItem(mode_man, &controller));

	LpcPinMap none = {-1, -1}; // unused pin has negative values in it
	LpcPinMap txpin = { 0, 18 }; // transmit pin that goes to debugger's UART->USB converter
	LpcPinMap rxpin = { 0, 13 }; // receive pin that goes to debugger's UART->USB converter

	
	LpcUartConfig cfg = { LPC_USART0, 115200, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1, false, txpin, rxpin, none, none };
	LpcUart dbgu(cfg);

	controller.run();
	menu.event(MenuItem::show);

	while(1){
		processEvents(menu);
		controller.run();
	}

	return 1;
}


