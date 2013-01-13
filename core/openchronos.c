/**
	@file	openchronos.c
	@brief	Main loop & user interface
	
	@author	Angelo Arrifano <miknix@gmail.com>
	
	@see		http://www.openchronos-ng.sourceforge.net
	@copyright
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	
 */

// *************************************************************************************************
//
//	Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
//
//
//	  Redistribution and use in source and binary forms, with or without
//	  modification, are permitted provided that the following conditions
//	  are met:
//
//	    Redistributions of source code must retain the above copyright
//	    notice, this list of conditions and the following disclaimer.
//
//	    Redistributions in binary form must reproduce the above copyright
//	    notice, this list of conditions and the following disclaimer in the
//	    documentation and/or other materials provided with the
//	    distribution.
//
//	    Neither the name of Texas Instruments Incorporated nor the names of
//	    its contributors may be used to endorse or promote products derived
//	    from this software without specific prior written permission.
//
//	  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//	  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//	  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//	  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//	  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//	  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//	  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//	  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//	  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// *************************************************************************************************


// *************************************************************************************************
// Include section

#include <core/openchronos.h>
#include <core/modinit.h>

// Drivers
#include <drivers/display.h>
#include <drivers/vti_as.h>
#include <drivers/vti_ps.h>
#include <drivers/radio.h>
#include <drivers/buzzer.h>
#include <drivers/ports.h>
#include <drivers/timer.h>
#include <drivers/pmm.h>
#include <drivers/rf1a.h>
#include <drivers/rtca.h>
#include <drivers/temperature.h>
#include <drivers/battery.h>


// *************************************************************************************************
// Defines section

#define BIT_IS_SET(F, B)  ((F) | (B)) == (F)


// *************************************************************************************************
// Global Variable section

/// Menu definitions and declarations
struct menu
{
	/// Menu item name
	char const * name;
	
	/// Pointer to up button handler
	void (*up_btn_fn)(void);
	/// Pointer to down button handler
	void (*down_btn_fn)(void);
	/// Pointer to function button (NUM)
	void (*num_btn_fn)(void);
	/// Pointer to settings button (long STAR)
	void (*lstar_btn_fn)(void);
	/// Pointer to function button (long NUM)
	void (*lnum_btn_fn)(void);
	/// Pointer to simultaneous up&down press
	void (*updown_btn_fn)(void);
	/// Pointer to activate function
	void (*activate_fn)(void);
	/// Pointer to deactivate function
	void (*deactivate_fn)(void);
	
	/// Pointer to next menu item
	struct menu *next;
	/// Pointer to the previous menu item
	struct menu *prev;
};

/// The head of the linked list holding menu items
static struct menu *menu_head;

/// Menu mode stuff
static struct
{
	uint8_t enabled:1;	/**< Is menu mode enabled ? */
	struct menu *item;	/**< The currently active menu item */
} menumode;

/// Menu edit mode stuff
static struct
{
	uint8_t enabled:1;					/**< Is edit mode enabled ? */
	uint8_t pos:7;						/**< The position for selected item */
	void (* complete_fn)(void);			/**< Call this fn when editmode exits */
	struct menu_editmode_item *items;	/**< Vector of editmode items */
} menu_editmode;

/// The message bus
static struct sys_messagebus *messagebus;


// *************************************************************************************************
// Extern section


// *************************************************************************************************
// BEGIN - THE SYSTEM MESSAGE BUS ******************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			sys_messagebus_register
/// @brief		Register an event callback.
/// @return		none
//* ************************************************************************************************
void sys_messagebus_register(void (*callback)(enum sys_message), enum sys_message listens)
{
	struct sys_messagebus **p = &messagebus;
	
	while (*p)
	{
		p = &(*p)->next;
	}
	
	*p = malloc(sizeof(struct sys_messagebus));
	(*p)->next = NULL;
	(*p)->fn = callback;
	(*p)->listens = listens;
}

//* ************************************************************************************************
/// @fn			sys_messagebus_unregister
/// @brief		Unregister an event callback.
/// @return		none
//* ************************************************************************************************
void sys_messagebus_unregister(void (*callback)(enum sys_message))
{
	struct sys_messagebus *p = messagebus, *pp = NULL;
	
	while (p)
	{
		if (p->fn == callback)
		{
			if (!pp)
				messagebus = p->next;
			else
				pp->next = p->next;
			
			free(p);
		}
		
		pp = p;
		p = p->next;
	}
}

//* ************************************************************************************************
/// @fn			check_events(void)
/// @brief		Check for events and notify the listener.
/// @return		none
//* ************************************************************************************************
void check_events(void)
{
	enum sys_message msg = 0;
	
	// Event from : "drivers/rtca"
	if (rtca_last_event)
	{
		msg |= rtca_last_event;
		rtca_last_event = 0;
	}
	
	// Event from : "drivers/timer"
	if (timer0_last_event)
	{
		msg |= timer0_last_event << 7;
		timer0_last_event = 0;
	}
	
	// Event from : "drivers/accelerometer"
	if(as_last_interrupt)
	{
		msg |= SYS_MSG_AS_INT;
		as_last_interrupt = 0;
	}
	
	// Event from : "driver/pressure"
	if(ps_last_interrupt)
	{
		msg |= SYS_MSG_PS_INT;
		ps_last_interrupt = 0;
	}
	
#ifdef CONFIG_BATTERY_MONITOR
	
	// Event from : "drivers/battery"
	if ((msg & SYS_MSG_RTC_MINUTE) == SYS_MSG_RTC_MINUTE)
	{
		msg |= SYS_MSG_BATT;
		battery_measurement();
	}
	
#endif
	
	{
		struct sys_messagebus *p = messagebus;
		
		while (p)
		{
			// Notify listener if he registered for any of these messages
			if (msg & p->listens)
			{
				p->fn(msg);
			}
			
			// Move to next
			p = p->next;
		}
	}
}


// *************************************************************************************************
// BEGIN - USER INPUT / MAIN MENU ******************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			editmode_handler(void)
/// @brief		Edit mode routine
/// @return		none
//* ************************************************************************************************
static void editmode_handler(void)
{
	// STAR button exits edit mode
	if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR))
	{
		// Deselect item
		menu_editmode.items[menu_editmode.pos].deselect();
		
		menu_editmode.complete_fn();
		menu_editmode.enabled = 0;
	}
	// NUM button switches to the next item
	else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_NUM))
	{
		// Deselect current item
		menu_editmode.items[menu_editmode.pos].deselect();
		
		// Select next item
		menu_editmode.pos++;
		if (! menu_editmode.items[menu_editmode.pos].set)
			menu_editmode.pos = 0;
		menu_editmode.items[menu_editmode.pos].select();
	}
	// UP button increments by 1 the item's value
	else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP))
	{
		/// @todo Replace here to allow user defined increments
		menu_editmode.items[menu_editmode.pos].set(1);
	}
	// UP button decrements by 1 the item's value
	else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN))
	{
		menu_editmode.items[menu_editmode.pos].set(-1);
	}
}

//* ************************************************************************************************
/// @fn			menumode_handler(void)
/// @brief		Menu mode routine
/// @return		none
//* ************************************************************************************************
static void menumode_handler(void)
{
	// STAR button activates the selected item
	if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR))
	{
		// Exit mode
		menumode.enabled = 0;
	
		// Clear both lines but keep symbols !
		display_clear(0, 1);
		display_clear(0, 2);
		
		// Turn off up/down symbols
		display_symbol(0, LCD_SYMB_ARROW_UP, SEG_OFF);
		display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_OFF);
		
		// Stop blinking name of current selected module
		display_chars(0, LCD_SEG_L2_4_0, NULL, BLINK_OFF);
		
		// Activate item
		if (menumode.item->activate_fn)
			menumode.item->activate_fn();
		
	}
	// UP button selects/shows the next item
	else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP))
	{
		menumode.item = menumode.item->next;
		display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);
	}
	// UP button selects/shows the previous item
	else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN))
	{
		menumode.item = menumode.item->prev;
		display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);
	}
}

//* ************************************************************************************************
/// @fn			menumode_enable(void)
/// @brief		Menu mode activation
/// @return		none
//* ************************************************************************************************
static void menumode_enable(void)
{
	// Deactivate current menu item
	if (menumode.item->deactivate_fn)
		menumode.item->deactivate_fn();
	
	// Enable edit mode
	menumode.enabled = 1;
	
	// Show MENU in the first line
	display_chars(0, LCD_SEG_L1_3_0, "MENU", SEG_SET);
	
	// Turn on up/down symbols
	display_symbol(0, LCD_SYMB_ARROW_UP, SEG_ON);
	display_symbol(0, LCD_SYMB_ARROW_DOWN, SEG_ON);
	
	// Show up blinking name of current selected item
	display_chars(0, LCD_SEG_L2_4_0, NULL, BLINK_ON);
	display_chars(0, LCD_SEG_L2_4_0, menumode.item->name, SEG_SET);
}

//* ************************************************************************************************
/// @fn			check_buttons(void)
/// @brief		Check for buttons and launch the proper routines
/// @return		none
//* ************************************************************************************************
static void check_buttons(void)
{
	// We are in edit mode, call it's handler routine
	if (menu_editmode.enabled)
	{
		editmode_handler();
	}
	// We are in menu mode, call it's handler routine
	else if (menumode.enabled)
	{
		menumode_handler();
	}
	// We are in a module, trigger button events
	else
	{
		// (long) STAR button
		if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_LSTAR))
		{
			if (menumode.item->lstar_btn_fn)
				menumode.item->lstar_btn_fn();
		}
		// STAR button
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_STAR))
		{
			// Enables the menu mode
			menumode_enable();
		}
		// (long) NUM button
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_LNUM))
		{
			if (menumode.item->lnum_btn_fn)
				menumode.item->lnum_btn_fn();
		}
		// NUM button
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_NUM))
		{
			if (menumode.item->num_btn_fn)
				menumode.item->num_btn_fn();
		}
		// UP & DOWN buttons
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP | PORTS_BTN_DOWN))
		{
			if (menumode.item->updown_btn_fn)
				menumode.item->updown_btn_fn();
		}
		// UP button
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_UP))
		{
			if (menumode.item->up_btn_fn)
				menumode.item->up_btn_fn();
		}
		// DOWN button
		else if (BIT_IS_SET(ports_pressed_btns, PORTS_BTN_DOWN))
		{
			if (menumode.item->down_btn_fn)
				menumode.item->down_btn_fn();
		}
	}
	
	ports_pressed_btns = 0;
}

//* ************************************************************************************************
/// @fn			menu_add_entry
/// @brief		Add an entry to the menu
/// @return		none
//* ************************************************************************************************
void menu_add_entry(char const * name,
	void (*up_btn_fn)(void),
	void (*down_btn_fn)(void),
	void (*num_btn_fn)(void),
	void (*lstar_btn_fn)(void),
	void (*lnum_btn_fn)(void),
	void (*updown_btn_fn)(void),
	void (*activate_fn)(void),
	void (*deactivate_fn)(void))
{
	struct menu **menu_hd = &menu_head;
	struct menu *menu_p;

	if (! *menu_hd)
	{
		// Head is empty, create new menu item linked to itself
		menu_p = (struct menu *) malloc(sizeof(struct menu));
		menu_p->next = menu_p;
		menu_p->prev = menu_p;
		*menu_hd = menu_p;
		
		// There wasnt any menu active, so we activate this one
		menumode.item = menu_p;
		activate_fn();
	}
	else
	{
		// Insert new item before head
		menu_p = (struct menu *) malloc(sizeof(struct menu));
		menu_p->next = (*menu_hd);
		menu_p->prev = (*menu_hd)->prev;
		(*menu_hd)->prev = menu_p;
		menu_p->prev->next = menu_p;
	}
	
	menu_p->name = name;
	menu_p->up_btn_fn = up_btn_fn;
	menu_p->down_btn_fn = down_btn_fn;
	menu_p->num_btn_fn = num_btn_fn;
	menu_p->lstar_btn_fn = lstar_btn_fn;
	menu_p->lnum_btn_fn = lnum_btn_fn;
	menu_p->updown_btn_fn = updown_btn_fn;
	menu_p->activate_fn = activate_fn;
	menu_p->deactivate_fn = deactivate_fn;
}

//* ************************************************************************************************
/// @fn			menu_editmode_start
/// @brief		Start the edit mode
/// @return		none
//* ************************************************************************************************
void menu_editmode_start(void (* complete_fn)(void), struct menu_editmode_item *items)
{
	menu_editmode.pos = 0;
	menu_editmode.items = items;
	menu_editmode.complete_fn = complete_fn;
	
	// Enable edit mode
	menu_editmode.enabled = 1;
	
	// Select the first item
	menu_editmode.items[0].select();
}


// *************************************************************************************************
// BEGIN - INITIALIZATION ROUTINE ******************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			init_application(void)
/// @brief		Init the watch's program
/// @return		none
//* ************************************************************************************************
void init_application(void)
{
	volatile unsigned char *ptr;
	
	// ---------------------------------------------------------------------
	// Enable watchdog
	
	// Watchdog triggers after 16 seconds when not cleared
#ifdef USE_WATCHDOG
	WDTCTL = WDTPW + WDTIS__512K + WDTSSEL__ACLK;
#else
	WDTCTL = WDTPW + WDTHOLD;
#endif
	
	// ---------------------------------------------------------------------
	// Configure PMM
	
	SetVCore(3);
	
	// Set global high power request enable
	PMMCTL0_H  = 0xA5;
	PMMCTL0_L |= PMMHPMRE;
	PMMCTL0_H  = 0x00;
	
	// ---------------------------------------------------------------------
	// Enable 32kHz ACLK
	
	P5SEL |= 0x03;				// Select XIN, XOUT on P5.0 and P5.1
	UCSCTL6 &= ~XT1OFF;			// XT1 On, Highest drive strength
	UCSCTL6 |= XCAP_3;			// Internal load cap
	
	UCSCTL3 = SELA__XT1CLK;		// Select XT1 as FLL reference
	UCSCTL4 = SELA__XT1CLK | SELS__DCOCLKDIV | SELM__DCOCLKDIV;
	
	// ---------------------------------------------------------------------
	// Configure CPU clock for 12MHz
	
	_BIS_SR(SCG0);				// Disable the FLL control loop
	UCSCTL0 = 0x0000;			// Set lowest possible DCOx, MODx
	UCSCTL1 = DCORSEL_5;		// Select suitable range
	UCSCTL2 = FLLD_1 + 0x16E;	// Set DCO Multiplier
	_BIC_SR(SCG0);				// Enable the FLL control loop
	
	// Worst-case settling time for the DCO when the DCO range bits have been
	// changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
	// UG for optimization.
	// 32 x 32 x 8 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
	
#if __GNUC_MINOR__ > 5 || __GNUC_PATCHLEVEL__ > 8
	
	__delay_cycles(250000);
	
#else
	
	__delay_cycles(62500);
	__delay_cycles(62500);
	__delay_cycles(62500);
	__delay_cycles(62500);
	
#endif
	
	// Loop until XT1 & DCO stabilizes, use do-while to insure that 
	// body is executed at least once
	do
	{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + XT1HFOFFG + DCOFFG);
		SFRIFG1 &= ~OFIFG;		// Clear fault flags
	}
	while ((SFRIFG1 & OFIFG));
	
	// ---------------------------------------------------------------------
	// Configure port mapping
	
	// Disable all interrupts
	__disable_interrupt();
	// Get write-access to port mapping registers:
	PMAPPWD = 0x02D52;
	// Allow reconfiguration during runtime:
	PMAPCTL = PMAPRECFG;
	
	// P2.7 = TA0CCR1A or TA1CCR0A output (buzzer output)
	ptr  = &P2MAP0;
	*(ptr + 7) = PM_TA1CCR0A;
	P2OUT &= ~BIT7;
	P2DIR |= BIT7;
	
	// P1.5 = SPI MISO input
	ptr  = &P1MAP0;
	*(ptr + 5) = PM_UCA0SOMI;
	// P1.6 = SPI MOSI output
	*(ptr + 6) = PM_UCA0SIMO;
	// P1.7 = SPI CLK output
	*(ptr + 7) = PM_UCA0CLK;
	
	// Disable write-access to port mapping registers:
	PMAPPWD = 0;
	// Re-enable all interrupts
	__enable_interrupt();
	
	// Init the hardwre real time clock (RTC_A)
	rtca_init();
	
	// ---------------------------------------------------------------------
	// Configure ports
	
	// ---------------------------------------------------------------------
	// Reset radio core
	
	radio_reset();
	radio_powerdown();
	
	// ---------------------------------------------------------------------
	// Init acceleration sensor
	
#ifdef CONFIG_MOD_ACCELEROMETER
	as_init();
#else
	as_disconnect();
#endif
	
	// ---------------------------------------------------------------------
	// Init LCD
	
	lcd_init();
	
	// ---------------------------------------------------------------------
	// Init buttons

	init_buttons();
	
	// ---------------------------------------------------------------------
	// Configure Timer0 for use by the clock and delay functions
	
	timer0_init();
	
	// Init buzzer
	buzzer_init();
	
	// ---------------------------------------------------------------------
	// Init pressure sensor
	
#ifdef CONFIG_PRESSURE_SENSOR
	ps_init();
#endif
	
	// ---------------------------------------------------------------------
	// Init other sensors
	
	// From: "driver/battery"
	battery_init();
	
	// From: "drivers/temperature"
	temperature_init();
	
	/// @todo What is this ?
#ifdef CONFIG_INFOMEM
	
	if (infomem_ready() == -2)
		infomem_init(INFOMEM_C, INFOMEM_C + 2 * INFOMEM_SEGMENT_SIZE);
	
#endif

}


// *************************************************************************************************
// BEGIN - ENTRYPOINT AND MAIN LOOP ****************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			main(void)
/// @brief		Application's entry point
/// @return		int		Exit status
//* ************************************************************************************************
int main(void)
{
	// Init MCU
	init_application();	
	
#ifdef CONFIG_TEST
	
	/// @todo Remove this ?
	
	// Branch to welcome screen
	test_mode();
	
#else
	
	// Clear whole scren
	display_clear(0, 0);
	
#endif
	
	// Init modules
	mod_init();
	
	// Main loop
	while (1)
	{
		// Go to LPM3, wait for interrupts
		_BIS_SR(LPM3_bits + GIE);
		__no_operation();
		
		// Service watchdog on wakeup
		#ifdef USE_WATCHDOG
			// Service watchdog (reset counter)
			WDTCTL = (WDTCTL & 0xff) | WDTPW | WDTCNTCL;
		#endif
		
		// Check if any driver has events pending
		check_events();
		
		// Check for button presses, drive the menu
		check_buttons();
	}
}


// *************************************************************************************************
// BEGIN - HERE BE HELPERS *************************************************************************
// *************************************************************************************************


//* ************************************************************************************************
/// @fn			helpers_loop
/// @return		none
//* ************************************************************************************************
void helpers_loop(uint8_t *value, uint8_t lower, uint8_t upper, int8_t step)
{
	// For now only increase/decrease on steps of 1 value
	if (step > 0)
	{
		// Prevent overflow
		if (*value == 255)
		{
			*value = lower;
			return;
		}
		
		(*value)++;
		
		if(*value -1 == upper)
			*value = lower;
	}
	else
	{
		// Prevent overflow
		if (*value == 0)
		{
			*value = upper;
			return;
		}
		
		(*value)--;
		
		if(*value +1 == lower)
			*value = upper;
	}
}
