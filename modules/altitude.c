/*
    TODO altitude.c: altitude display module

	Copyright (C) 2012 Stanislas Bach <stanislasbach@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

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

// System
#include <openchronos.h>

// Driver
#include "drivers/display.h"
#include "drivers/altitude.h"


// *************************************************************************************************
// Defines section


// *************************************************************************************************
// Global Variable section




// *************************************************************************************************
// Extern section


// *************************************************************************************************
// @fn          display_altitude
// @brief       Common display routine
// @return      none
// *************************************************************************************************
void display_altitude()
{
	display_chars(0, LCD_SEG_L1_3_0, _sprintf("%3s", altitude.pressure), SEG_SET);
	display_chars(0, LCD_SEG_L2_3_0, _sprintf("%3s", altitude.temperature), SEG_SET);
}


// *************************************************************************************************
// @fn          measure_altitude
// @brief       Temperature display routine. Mesure and parse the temperature.
// @return      none
// *************************************************************************************************
static void measure_altitude(enum sys_message msg)
{
	// Call the driver to measure the temperature
	start_altitude_measurement();
	altitude_measurement();
	stop_altitude_measurement();
	
	// Display new stuff on the screen
	display_altitude();
}

static void ps_event(enum sys_message msg)
{
	//start_altitude_measurement();
	altitude_measurement();
	//stop_altitude_measurement();
	
	// Display new stuff on the screen
	display_altitude();
}

// *************************************************************************************************
// @fn          altitude_activate
// @brief       Altitude screen activation. Display defaul stuff, register the mesuring loop.
// @return      none
// *************************************************************************************************
static void altitude_activate()
{	
	// Display the title of this module at the bottom
	display_chars(0, LCD_SEG_L2_3_0, "ALTI", SEG_SET);

	// Display something while a measure is not performed
	display_chars(0, LCD_SEG_L1_2_0, " - ", SEG_ON);
	
	// Register an event
	sys_messagebus_register(&ps_event, SYS_MSG_PS_INT);
	
	// Start the sensor
	start_altitude_measurement();

	// Init the sensor
	altitude_init();
		
	//sys_messagebus_register(&measure_altitude, SYS_MSG_TIMER_4S);
}


// *************************************************************************************************
// @fn          altitude_deactivate
// @brief       Altitude screen desactivation. Clear the screen, reset the events & values.
// @return      none
// *************************************************************************************************
static void altitude_deactivate()
{
	// Unregister the event
	//sys_messagebus_unregister(&measure_altitude);
	
	sys_messagebus_unregister(&ps_event);
	
	// Stop the sensor
	stop_altitude_measurement();
	
	// Cleanup screens
	display_clear(0, 1);
}


// *************************************************************************************************
// @fn          mod_altiude_init
// @brief       Init the module. Sets default values, register menu entry.
// @return      none
// *************************************************************************************************
void mod_altitude_init(void)
{			
	menu_add_entry(" ALTI",
		NULL, NULL, NULL,
		NULL,	// FIXME Altitude edit ?
		NULL, NULL,
		&altitude_activate,
		&altitude_deactivate);
}
